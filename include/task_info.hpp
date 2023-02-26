#pragma once

#include "make_string.hpp"
#include <string>
#include <future>
#include <type_traits>
#include <QObject>

namespace TP
{
    Q_NAMESPACE

    /**
     * @brief All possible task states
     */
    enum class TaskStatus
    {
        InQueue,
        InProcess,
        Completed
    };
    Q_ENUM_NS(TaskStatus)

    /**
     * @brief Interface class for TaskInfo
    */
    class ITaskInfo
    {
    public:
        /**
         * @brief Returns the task status
         * @return ITaskInfo::TaskStatus
         */
        virtual TaskStatus status() const = 0;

        /**
         * @brief Instantly returns result of the task
         * If the task is not completed or void - returns an empty string
         * @return 
         */
        virtual std::string result_str() = 0;

        /**
         * @brief Getter for task id
         * @return Const reference to inner field
         */
        virtual const size_t &id() const = 0;

        /**
         * @brief Setter for task name
         * @return Reference to inner field
         */
        virtual std::string &name() = 0;

        /**
         * @brief Getter for task name
         * @return Const reference to inner field
         */
        virtual const std::string &name() const = 0;
    };

    /**
     * @brief Interface class for TaskInfo
    */
    template <typename T>
    class TaskInfo : public ITaskInfo
    {
    public:
        /**
         * @brief Construct TaskInfo
         * @param idx Index of task
         * @param start_future Future that will be fulfilled when the task starts executing
         * @param ret_future Future that will be fulfilled when the task finishes executing (Will contain result of task)
         */
        TaskInfo(size_t idx,
                 std::future<void> &&start_future,
                 std::future<T> &&ret_future) : m_idx(idx),
                                                m_start_future(std::move(start_future)),
                                                m_ret_future(std::move(ret_future))
        {
        }

        /**
         * @brief Class that stores futures for the task and main information
         */
        TaskStatus status() const
        {
            if (m_start_future.valid() && m_ret_future.valid())
            {
                if (m_ret_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    return TaskStatus::Completed;
                if (m_start_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    return TaskStatus::InProcess;
            }
            return TaskStatus::InQueue;
        }

        /**
         * @brief The result function waits until the future has a valid result and retrieves it. 
         * It effectively calls wait() in order to wait for the result. 
         * @return Future result
         */
        T result()
        {
            return m_ret_future.get();
        }

        /**
         * @brief Instantly returns result of the task
         * If the task is not completed or void - returns an empty string
         * @return 
         */
        std::string result_str()
        {
            return make_string(m_ret_future);
        }

        /**
         * @brief Getter for task id
         * @return Const reference to m_idx
         */
        const size_t &id() const { return m_idx; }

        /**
         * @brief Setter for task name
         * @return Reference to m_task_name
         */
        std::string &name() { return m_task_name; }

        /**
         * @brief Getter for task name
         * @return Const reference to m_task_name
         */
        const std::string &name() const { return m_task_name; }

    private:
        // Information about the task in the thread pool
        size_t m_idx;
        std::shared_future<void> m_start_future;
        std::shared_future<T> m_ret_future;

        // String representation of the task (name and arguments)
        std::string m_task_name;
    };

}