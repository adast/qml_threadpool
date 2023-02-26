#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <atomic>

namespace TP
{
    /**
     * @brief Utility class, useful for calling callbacks asynchronous
    */
    template <typename... Args>
    class AsyncEvent
    {
    public:
        AsyncEvent(size_t max_len = 128) : m_max_len(max_len) {}

        /**
         * @brief Starts the thread
         * @param func Universal reference to callback function
         * @return Success (true) or failure (false)
         */
        template <typename Func>
        bool start(Func &&func)
        {
            if (m_active)
                return false;

            m_active = true;
            m_callback = std::forward<Func>(func);
            m_thread_ptr = std::unique_ptr<std::thread>(new std::thread(&AsyncEvent::run, this));
            return true;
        }

        /**
         * @brief Stops the thread and cleans the the queue
         * @return Success (true) or failure (false)
         */
        bool stop()
        {
            if (!m_active)
                return false;
                
            m_active = false;
            m_queue_cv.notify_all();
            m_thread_ptr->join();
            m_thread_ptr.reset();
            m_events = {};
            return true;
        }

        /**
         * @brief Adds callback to the queue
         * @param ...args - arguments to the callback
         */
        template<typename... Ts>
        bool call(Ts &&...args)
        {
            if (!m_callback)
                return false;

            // Populate containers
            std::lock_guard<std::mutex> q_lock(m_queue_mtx);
            if (m_events.size() == m_max_len) {
                m_events.pop_front();
            }
            m_events.emplace_back(std::bind(m_callback, std::forward<Ts>(args)...));
            m_queue_cv.notify_one();
            return true;
        }

        /**
         * @brief Working thread
         */
        void run()
        {
            while (m_active)
            {
                std::unique_lock<std::mutex> lock(m_queue_mtx);
                m_queue_cv.wait(lock, [this]
                                { return !m_events.empty() || !m_active; });

                if (!m_events.empty())
                {
                    // Get task and start_promise from the queues
                    auto bind_cb = std::move(m_events.front());
                    m_events.pop_front();
                    lock.unlock();

                    // Call actual callback
                    bind_cb();
                }
            }
        }

        /**
         * @brief Destructor, just calls the stop method
        */
        ~AsyncEvent()
        {
            stop();
        }

    private:
        // Callback function
        std::function<void(Args...)> m_callback;

        // Thread to run callbacks on
        std::unique_ptr<std::thread> m_thread_ptr;

        // Thread pool state flag (active or not)
        std::atomic<bool> m_active = {false};

        // Mutex for reading&writing to queue by different threads
        std::mutex m_queue_mtx;

        // Conditional variable for notifying thread that a queue is not empty
        std::condition_variable m_queue_cv;

        // Event queue
        std::deque<std::function<void()>> m_events;

        // Max queue length
        size_t m_max_len;
    };
}