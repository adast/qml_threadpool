#include "thread_pool.hpp"

namespace TP
{

    bool ThreadPool::start(size_t num_threads)
    {
        if (m_active)
            return false;

        // Create threads
        m_active = true;
        for (size_t i = 0; i < num_threads; i++)
        {
            m_threads.emplace_back(&ThreadPool::run, this);
        }

        return true;
    }

    bool ThreadPool::stop()
    {
        if (!m_active)
            return false;

        m_active = false;
        m_queue_cv.notify_all();
        for (size_t i = 0; i < m_threads.size(); i++)
        {
            m_threads[i].join();
        }
        m_threads.clear();

        return true;
    }

    void ThreadPool::remove_tasks(std::unordered_set<size_t> &idxs)
    {
        std::lock_guard<std::mutex> q_lock(m_queue_mtx);

        if (idxs.size() < 100 && idxs.size() < m_tasks.size() / 10)
        {
            // Remove using binary search for small number of tasks to be deleted
            for (auto idxs_it = idxs.begin(); idxs_it != idxs.end();)
            {
                // Get id of task to be removed from iterator
                auto idx = *idxs_it;

                // Find task by id
                auto tasks_it = std::lower_bound(m_tasks.begin(), m_tasks.end(), idx,
                                                 [](const QueueElement &task, size_t target_idx)
                                                 { return task.idx < target_idx; });
                // Check if task was found
                if (tasks_it == m_tasks.end() || tasks_it->idx != idx)
                {
                    idxs_it++;
                    continue;
                }

                // Remove task
                idxs_it = idxs.erase(idxs_it);
                m_tasks.erase(tasks_it);
            }
        }
        else
        {
            // Remove using remove&erase idiom
            m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
                                         [&idxs](const QueueElement &task)
                                         {
                                             bool to_delete = idxs.count(task.idx);
                                             if (to_delete)
                                             {
                                                 idxs.erase(task.idx);
                                             }
                                             return to_delete;
                                         }),
                          m_tasks.end());
        }
    }

    void ThreadPool::run()
    {
        while (m_active)
        {
            std::unique_lock<std::mutex> lock(m_queue_mtx);
            m_queue_cv.wait(lock, [this]
                            { return !m_tasks.empty() || !m_active; });

            if (!m_tasks.empty())
            {
                // Get task
                auto task = std::move(m_tasks.front());
                m_tasks.pop_front();

                // Unlock the queue
                lock.unlock();

                // Send event (task in progress)
                mEvent.call(task.idx, false);

                // Indicate that computations stated
                task.start_promise.set_value();

                // Start actual computations
                task.task();

                // Update number of finished tasks
                m_finished++;

                // Send event (task finished)
                mEvent.call(task.idx, true);
            }
        }
    }

}