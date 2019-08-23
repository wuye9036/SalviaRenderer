#include "scope_guard.hpp"

#include <memory>
#include <thread>
#include <mutex>
#include <functional>


namespace eflib { namespace threadpool { namespace detail 
{

  using namespace std;
  /*! \brief Thread pool worker. 
  *
  * A worker_thread represents a thread of execution. The worker is attached to a 
  * thread pool and processes tasks of that pool. The lifetime of the worker and its 
  * internal boost::thread is managed automatically.
  *
  * This class is a helper class and cannot be constructed or accessed directly.
  * 
  * \see pool_core
  */ 
  template <typename Pool>
  class worker_thread
  : public enable_shared_from_this< worker_thread<Pool> >
  {
  public:
    typedef Pool pool_type;         	   //!< Indicates the pool's type.

  private:
    shared_ptr<pool_type>      m_pool;     //!< Pointer to the pool which created the worker.
    shared_ptr<boost::thread>  m_thread;   //!< Pointer to the thread which executes the run loop.

    
    /*! Constructs a new worker. 
    * \param pool Pointer to it's parent pool.
    * \see function create_and_attach
    */
    worker_thread(shared_ptr<pool_type> const & pool)
    : m_pool(pool)
    {
      assert(pool);
    }

	
	/*! Notifies that an exception occurred in the run loop.
	*/
	void died_unexpectedly()
	{
		m_pool->worker_died_unexpectedly(this->shared_from_this());
	}


  public:
	  /*! Executes pool's tasks sequentially.
	  */
	  void run()
	  { 
		  scope_guard notify_exception(bind(&worker_thread::died_unexpectedly, this));

		  while(m_pool->execute_task()) {}

		  notify_exception.disable();
		  m_pool->worker_destructed(this->shared_from_this());
	  }


	  /*! Joins the worker's thread.
	  */
	  void join()
	  {
		  m_thread->join();
	  }


	  /*! Constructs a new worker thread and attaches it to the pool.
	  * \param pool Pointer to the pool.
	  */
	  static void create_and_attach(shared_ptr<pool_type> const & pool)
	  {
		  shared_ptr<worker_thread> worker(new worker_thread(pool));
		  if(worker)
		  {
			  worker->m_thread.reset(new boost::thread(bind(&worker_thread::run, worker)));
		  }
	  }

  };


} } }

