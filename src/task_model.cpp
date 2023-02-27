#include "task_model.hpp"
#include <QMetaEnum>

TaskModel::TaskModel()
{
    // Emit signals based on events from thread pool
    m_pool.set_async_callback([&](size_t task_idx, bool state)
                             {
        // Update progress bar
        if (state)
            emit numFinishedChanged();

        // Get row index by task index if possible
        std::unique_lock<std::mutex> id_map_lock(m_id_map_mtx);
        if (!m_id_map.count(task_idx))
            return;
        size_t row_idx = m_id_map[task_idx];
        id_map_lock.unlock();

        // Emit list update signal
        emit dataChanged(index(row_idx), index(row_idx), 
            state ? QVector<int>{StatusRole, ResultRole} : QVector<int>{StatusRole}); });
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    return m_tasks.size();
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    // Index sanity check
    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();

    switch (role)
    {
    case NameRole:
        return QString::fromStdString(m_tasks[index.row()]->name());
    case StatusRole:
        return QVariant::fromValue(m_tasks[index.row()]->status());
    case ResultRole:
        return QString::fromStdString(m_tasks[index.row()]->result_str());
    case SelectedRole:
        return (bool)m_selected.count(m_tasks[index.row()]->id());
    }

    return QVariant();
}

bool TaskModel::setData(const QModelIndex &index, const QVariant &v, int role)
{
    // Index sanity check
    if (!index.isValid() || index.row() >= rowCount())
        return false;

    if (role == SelectedRole)
    {
        // Update container of selected tasks
        if (v.value<bool>())
        {
            // Add task to selected if checkbox changed state to checked
            m_selected.insert(m_tasks[index.row()]->id());
        }
        else
        {
            // Add task to selected if checkbox changed state to unchecked
            m_selected.erase(m_tasks[index.row()]->id());
        }

        // Emit signals
        emit dataChanged(index, index, {SelectedRole});
        emit numSelectedChanged();
        return true;
    }

    return false;
}

QHash<int, QByteArray> TaskModel::roleNames() const
{
    auto roles = QAbstractListModel::roleNames();
    roles[NameRole] = "name";
    roles[StatusRole] = "status";
    roles[ResultRole] = "result";
    roles[SelectedRole] = "checked";
    return roles;
}

QList<QVariant> TaskModel::taskTypes()
{
    QList<QVariant> task_types;

    QMetaEnum e = QMetaEnum::fromType<TaskTypes>();
    for (int i = 0; i < e.keyCount(); i++)
    {
        task_types.push_back(
            QVariant::fromValue(static_cast<TaskTypes>(e.value(i))));
    }

    return task_types;
}

bool TaskModel::addTask(TaskTypes task_type, const QVariant &arg, bool enbl_emit)
{
    // Add task into thread pool
    std::unique_ptr<TP::ITaskInfo> task_info;
    switch (task_type)
    {
    case TaskTypes::Fibonacci:
        task_info = m_pool.add_task_uptr(tasks::fib, arg.value<int>());
        break;
    case TaskTypes::Factorial:
        task_info = m_pool.add_task_uptr(tasks::factorial, arg.value<int>());
        break;
    case TaskTypes::DoubleFactorial:
        task_info = m_pool.add_task_uptr(tasks::double_factorial, arg.value<int>());
        break;
    }

    // Add task_info to list if it was created
    if (task_info)
    {
        if (enbl_emit)
        {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
        }

        // Put task_info into list
        QString task_name = QVariant::fromValue(task_type).toString() + "(" + arg.toString() + ")";
        task_info->name() = task_name.toStdString();
        m_tasks.emplace_back(std::move(task_info));

        if (enbl_emit)
        {
            emit numTotalChanged();
            endInsertRows();
        }

        return true;
    }

    return false;
}

void TaskModel::addTasksRandom(int n, int min_value, int max_value)
{
    QMetaEnum e = QMetaEnum::fromType<TaskTypes>();
    std::uniform_int_distribution<> task_distrib(0, e.keyCount() - 1);
    std::uniform_int_distribution<> arg_distrib(min_value, max_value);

    beginInsertRows(QModelIndex(), rowCount(), rowCount() - 1 + n);
    for (int i = 0; i < n; i++)
    {
        addTask(static_cast<TaskTypes>(task_distrib(m_rand_gen)), arg_distrib(m_rand_gen), false);
    }
    endInsertRows();

    // Emit signal num total
    emit numTotalChanged();
}

void TaskModel::removeTasks()
{
    // Copy m_selected (because the next step will partially clear it)
    auto selected = m_selected;

    // Remove tasks from pool (and clears m_selected from all indexes that were in queue)
    m_pool.remove_tasks(m_selected);

    // Emit signal
    beginResetModel();

    // Remove using remove&erase idiom
    auto &remaining_idxs = m_selected; // Symlink m_selected for convinience
    auto &counter = m_num_finished_removed;
    m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
                                [&selected, &remaining_idxs, &counter](const std::unique_ptr<TP::ITaskInfo> &task)
                                {
                                    // Check if the task has been deleted from pool
                                    bool deleted_from_pool = !remaining_idxs.count(task->id());
                                    // Task should be deleted if it's selected and either was deleted from pool or finished
                                    bool remove = (selected.count(task->id()) &&
                                                   (deleted_from_pool || (task->status() == TP::TaskStatus::Completed)));
                                    // Delete already finished task
                                    if (remove && !deleted_from_pool)
                                    {
                                        remaining_idxs.erase(task->id());
                                        counter++;
                                    }
                                    return remove;
                                }),
                 m_tasks.end());

    // Invalidate idMap
    std::unique_lock<std::mutex> id_map_lock(m_id_map_mtx);
    m_id_map.clear();
    id_map_lock.unlock();

    // Emit signals
    endResetModel();
    emit numTotalChanged();
    emit numSelectedChanged();
    emit numFinishedChanged();
}

void TaskModel::selectTasksAll(bool select)
{
    if (select)
    {
        for (const auto &task : m_tasks)
        {
            m_selected.insert(task->id());
        }
    }
    else
    {
        m_selected.clear();
    }
    emit dataChanged(index(0), index(rowCount() - 1), {SelectedRole});
    emit numSelectedChanged();
}

void TaskModel::updateIdMap(int row_idx)
{
    if (row_idx < 0 || row_idx >= rowCount())
        return;

    // Lock and update idMap
    std::lock_guard<std::mutex> id_map_lock(m_id_map_mtx);
    m_id_map[m_tasks[row_idx]->id()] = row_idx;
}

bool TaskModel::startPool(int num_threads)
{
    return m_pool.start(num_threads);
}

bool TaskModel::stopPool()
{
    return m_pool.stop();
}

int TaskModel::numFinished() const
{
    return m_pool.num_finished() - m_num_finished_removed;
}

int TaskModel::numSelected() const
{
    return m_selected.size();
}