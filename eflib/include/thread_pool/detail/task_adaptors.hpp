#pragma once

#include <memory>
#include <thread>
#include <functional>
#include <chrono>

namespace eflib {
    namespace threadpool
    {

        /*! \brief Standard task function object.
        *
        * This function object wraps a nullary function which returns void.
        * The wrapped function is invoked by calling the operator ().
        *
        * \see boost function library
        *
        */
        typedef std::function<void()> task_func;


        /*! \brief Prioritized task function object.
        *
        * This function object wraps a task_func object and binds a priority to it.
        * prio_task_funcs can be compared using the operator < which realises a partial ordering.
        * The wrapped task function is invoked by calling the operator ().
        *
        * \see prio_scheduler
        *
        */
        class prio_task_func
        {
        private:
            unsigned int m_priority;  //!< The priority of the task's function.
            task_func m_function;     //!< The task's function.

        public:
            typedef void result_type; //!< Indicates the functor's result type.

        public:
            /*! Constructor.
            * \param priority The priority of the task.
            * \param function The task's function object.
            */
            prio_task_func(unsigned int const priority, task_func const& function)
                : m_priority(priority)
                , m_function(function)
            {
            }

            /*! Executes the task function.
            */
            void operator() (void) const
            {
                if (m_function)
                {
                    m_function();
                }
            }

            /*! Comparison operator which realises a partial ordering based on priorities.
            * \param rhs The object to compare with.
            * \return true if the priority of *this is less than right hand side's priority, false otherwise.
            */
            bool operator< (const prio_task_func& rhs) const
            {
                return m_priority < rhs.m_priority;
            }

        };  // prio_task_func

        /*! \brief Looped task function object.
        *
        * This function object wraps a boolean thread function object.
        * The wrapped task function is invoked by calling the operator () and it is executed in regular
        * time intervals until false is returned. The interval length may be zero.
        * Please note that a pool's thread is engaged as long as the task is looped.
        *
        */
        class looped_task_func
        {
        private:
            std::function<bool()> m_function;   //!< The task's function.
            std::chrono::microseconds m_break_ms;            //!< Duration of breaks in milliseconds

            inline bool do_interval() const
            {
                if (m_break_ms.count() > 0)
                {
                    std::this_thread::sleep_for(m_break_ms);
                    return true;
                }
                return false;
            }
        public:
            typedef void result_type; //!< Indicates the functor's result type.

        public:
            /*! Constructor.
            * \param function The task's function object which is looped until false is returned.
            * \param interval The minimum break time in milli seconds before the first execution of the task function and between the following ones.
            */
            looped_task_func(std::function<bool()> const& function, unsigned int const interval_ms = 0)
                : m_function(function)
                , m_break_ms{static_cast<decltype(m_break_ms)::rep>(interval_ms)}
            {
            }

            /*! Executes the task function.
            */
            void operator() (void) const
            {
                if (m_function)
                {
                    do_interval();
                    while (m_function())
                    {
                        if (!do_interval())
                        {
                            std::this_thread::yield(); // Be fair to other threads
                        }
                    }
                }
            }

        }; // looped_task_func


    }
}

