#pragma once

#include "task_info.hpp"
#include "async_event.hpp"

#include <deque>
#include <unordered_set>
#include <memory>
#include <future>
#include <type_traits>
#include <atomic>

namespace TP
{
    /**
     * @brief Thread pool class
    */
    class ThreadPool
    {
        /**
         * @brief Utility struct, used for storing tasks in the queue
        */
        struct QueueElement
        {
            size_t idx;
            std::packaged_task<void()> task;
            std::promise<void> start_promise;

            template <typename Task, typename StartPromise>
            QueueElement(size_t idx,
                         Task &&task,
                         StartPromise &&start_promise) : idx(idx),
                                                         task(std::forward<Task>(task)),
                                                         start_promise(std::forward<StartPromise>(start_promise)) {}
        };

    public:
        /**
         * @brief Starts the thread pool with given number of threads
         * @param num_threads Threads in thread pool
         * @return Success (true) or failure (false)
         */
        bool start(size_t num_threads);

        /**
         * @brief Stops the thread pool
         * Not started tasks remain in the queue
         * Started tasks are completed (blocking, wait till finish)
         * Threads are released
         * @return Success (true) or failure (false)
         */
        bool stop();

        /**
         * @brief Working thread
         */
        void run();

        /**
         * @brief Removes tasks by given task indices
         * @param idxs Set of indices of tasks to be removed
         */
        void remove_tasks(std::unordered_set<size_t> &idxs);

        /**
         * @brief Adds task to the queue
         * @param func Task function
         * @param args Arguments of the task (variadic)
         * @return TaskInfo<RET>, where RET - return type of func
         */
        template <typename Func, typename... Args, typename RET = typename std::result_of<Func(Args...)>::type>
        auto add_task(Func &&func, Args &&...args) -> TaskInfo<RET>
        {
            // Get task unique index
            size_t task_idx = m_last_idx++;

            // Create packaged task
            auto task = std::packaged_task<RET()>(
                std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

            // Create promise that will be fulfilled when the task starts executing
            std::promise<void> start_promise;

            // Create TaskInfo
            TaskInfo<RET> info(task_idx, start_promise.get_future(), task.get_future());

            // Populate containers
            std::lock_guard<std::mutex> q_lock(m_queue_mtx);
            m_tasks.emplace_back(task_idx, std::move(task), std::move(start_promise));
            m_queue_cv.notify_one();

            return info;
        }

        /**
         * @brief Wrapper for add_task, return unique_ptr to TaskInfo
         * @param func Task function
         * @param Arguments of the task (variadic)
         * @return std::unique_ptr<TaskInfo<RET>>, where RET - return type of func
         */
        template <typename Func, typename... Args, typename RET = typename std::result_of<Func(Args...)>::type>
        auto add_task_uptr(Func &&func, Args &&...args) -> std::unique_ptr<TaskInfo<RET>>
        {
            return std::unique_ptr<TaskInfo<RET>>(
                new TaskInfo<RET>(add_task(std::forward<Func>(func), std::forward<Args>(args)...)));
        }

        /**
         * @brief Sets callback for receiving thread pool events
         * @param func Callback function
        */
        template <typename Func>
        void set_async_callback(Func &&func)
        {
            mEvent.start(std::forward<Func>(func));
        }

        /**
         * @brief Returns number of finished tasks
         * @returns Number of finished tasks
        */
        inline double num_finished() const
        {
            return m_finished;
        }

        /**
         * @brief Destructor, just calls the stop method
        */
        inline ~ThreadPool()
        {
            stop();
        }

    private:
        // Threads container
        std::vector<std::thread> m_threads;

        // Thread pool state flag (active or not)
        std::atomic<bool> m_active = {false};

        // Atomic variable for keeping track of new tasks indices
        std::atomic<size_t> m_last_idx = {0};

        // Atomic variable for keeping track of finished tasks
        std::atomic<size_t> m_finished = {0};

        // Mutex for reading&writing to queue by different threads
        std::mutex m_queue_mtx;

        // Conditional variable for notifying thread that a queue is not empty
        std::condition_variable m_queue_cv;

        // Tasks queue
        std::deque<QueueElement> m_tasks;

        // Event for callbacks
        AsyncEvent<size_t, bool> mEvent;
    };
}