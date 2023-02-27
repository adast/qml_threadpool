#pragma once

#include "tasks.hpp"
#include "thread_pool.hpp"

#include <memory>
#include <random>
#include <unordered_set>
#include <unordered_map>

#include <QAbstractListModel>

/**
 * @brief Model for TaskList.qml
 */
class TaskModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QList<QVariant> taskTypes READ taskTypes CONSTANT)
    Q_PROPERTY(int numTotal READ rowCount NOTIFY numTotalChanged)
    Q_PROPERTY(int numSelected READ numSelected NOTIFY numSelectedChanged)
    Q_PROPERTY(double numFinished READ numFinished NOTIFY numFinishedChanged)

public:
    /**
     * @brief Enum of all available roles
     */
    enum Roles
    {
        NameRole = Qt::UserRole + 1,
        StatusRole,
        ResultRole,
        SelectedRole
    };

    /**
     * @brief Enum of all available tasks
     */
    enum class TaskTypes
    {
        Fibonacci,
        Factorial,
        DoubleFactorial
    };
    Q_ENUM(TaskTypes);

    /**
     * @brief Constructor
    */
    TaskModel();

    // Model basic functionality
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &v, int role) override;
    QHash<int, QByteArray> roleNames() const override;
    
    /**
     * @brief Returns all available task types
     * @return QList of QVariant
     */
    QList<QVariant> taskTypes();

    /**
     * @brief Returns number of already finished tasks (useful for progress bars)
     * @return Number of already finished tasks
     */
    int numFinished() const;
    
    /**
     * @brief Returns number of selected tasks (useful for GUI)
     * @return Number of selected tasks
     */
    int numSelected() const;

public slots:
    
    /**
     * @brief Creates task with given type and arguments.
     * Puts it into thread pool and this model (m_tasks).
     * @param task_type Task type (from TaskTypes enum)
     * @param arg Argument of the task
     * @param enbl_emit Emit (true) or not (false) signal during insertion. 
     * This argument could be useful to prevent signal spamming 
     * (for example calling this function in a loop, see addTasksRandom)
     * 
     * Emits numTotalChanged and calls insertRows
     * 
     * @return Success (true) or failure (false)
     */
    bool addTask(TaskTypes task_type, const QVariant &arg, bool enbl_emit = true);

    /**
     * @brief Creates n random tasks of all available types (see TaskTypes)
     * @param n Number of tasks to create
     * @param min_value Minimum argument value
     * @param max_value Maximum argument value
     */
    void addTasksRandom(int n, int min_value = 0, int max_value = 99999);

    /**
     * @brief Removes tasks selected previously
     * Resets the model (basically forces GUI to redraw)
     * Emits whole bunch of signals(numTotalChanged, numSelectedChanged, numFinishedChanged)
     * TODO: Should be optimized, currently - amortized O(N) on average, where N - total number of tasks
     */
    void removeTasks();

    /**
     * @brief Selects or deselects all tasks
     * @param select Select (true) or deselect(false)
     */
    void selectTasksAll(bool select);

    /**
     * @brief Inserts mapping task_idx->row_idx into idMap
     * !IMPORTANT This slot should be called with delegate (Component.onCompleted)
     * Otherwise thread pool events will not affect GUI 
     * @param row_idx Index of row where task located
     */
    void updateIdMap(int row_idx);

    /**
     * @brief Starts the thread pool with given number of threads
     * @param num_threads Threads in thread pool
     * @return Success (true) or failure (false)
     */
    bool startPool(int num_threads);

    /**
     * @brief Stops the thread pool
     * Not started tasks remain in the queue
     * Started tasks are completed (blocking, wait till finish)
     * Threads are released
     * TODO: This could be slow for long tasks (unresponsive UI), should be async
     * @return Success (true) or failure (false)
     */
    bool stopPool();

signals:
    /**
     * @brief This signal is emitted after number of rows (m_tasks) have been changed
     * This signal is emitted when tasks are added
     * This signal is emitted when tasks are removed
    */
    void numTotalChanged();

    /**
     * @brief This signal is emitted after number of selected tasks have been changed
     * This signal is emitted when the user selects tasks
     * This signal is emitted when tasks are deleted 
    */
    void numSelectedChanged();

    /**
     * @brief This signal is emitted after number of selected tasks have been changed
     * This signal is emitted when the user selects tasks
     * This signal is emitted when tasks are deleted 
    */
    void numFinishedChanged();
    
private:
    // Instance of thread pool
    TP::ThreadPool m_pool;

    // Containter that stores information about tasks
    std::deque<std::unique_ptr<TP::ITaskInfo>> m_tasks;

    // Task selection
    std::unordered_set<size_t> m_selected; // ids of selected tasks

    // Map that connects task ids to row indexes
    std::mutex m_id_map_mtx;
    std::unordered_map<size_t, size_t> m_id_map;

    // Compensation for already finished removed tasks 
    // Useful to keep progress bar (numFinished) in valid state
    size_t m_num_finished_removed = 0;

    // Random engine
    std::mt19937 m_rand_gen;
};