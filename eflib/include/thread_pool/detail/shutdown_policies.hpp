#pragma once

/// The namespace threadpool contains a thread pool and related utility classes.
namespace eflib {
    namespace threadpool
    {
        /*! \brief ShutdownPolicy which waits for the completion of all tasks
          *          and the worker termination afterwards.
            *
          * \param Pool The pool's core type.
          */
        template<typename Pool>
        class wait_for_all_tasks
        {
        public:
            static void shutdown(Pool& pool)
            {
                pool.wait();
                pool.terminate_all_workers(true);
            }
        };


        /*! \brief ShutdownPolicy which waits for the completion of all active tasks
        *          and the worker termination afterwards.
        *
        * \param Pool The pool's core type.
        */
        template<typename Pool>
        class wait_for_active_tasks
        {
        public:
            static void shutdown(Pool& pool)
            {
                pool.clear();
                pool.wait();
                pool.terminate_all_workers(true);
            }
        };


        /*! \brief ShutdownPolicy which does not wait for any tasks or worker termination.
        *
        * This policy does not wait for any tasks. Nevertheless all active tasks will be processed completely.
        *
        * \param Pool The pool's core type.
        */
        template<typename Pool>
        class immediately
        {
        public:
            static void shutdown(Pool& pool)
            {
                pool.clear();
                pool.terminate_all_workers(false);
            }
        };

    }
}
