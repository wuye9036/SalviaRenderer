#ifndef EFLIB_MEMORY_ATOMIC_H
#define EFLIB_MEMORY_ATOMIC_H

#include <eflib/include/platform/config.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/atomic.hpp>
#include <eflib/include/platform/boost_end.h>

namespace eflib{
	class spinlock {
	private:
		typedef enum {Locked, Unlocked} LockState;
		boost::atomic<LockState> state_;

	public:
		spinlock() : state_(Unlocked) {}

		void lock()
		{
			while (state_.exchange(Locked, boost::memory_order_acquire) == Locked)
			{
				/* busy-wait */
			}
		}

		void unlock()
		{
			state_.store(Unlocked, boost::memory_order_release);
		}
	};

	class scoped_spin_locker
	{
	public:
		scoped_spin_locker(spinlock& lock): lock_(lock) { lock_.lock(); }
		~scoped_spin_locker() { lock_.unlock(); }
	private:
		scoped_spin_locker(scoped_spin_locker const&);
		scoped_spin_locker& operator = (scoped_spin_locker const&);
		spinlock& lock_;
	};
}


#endif		// SALVIAR_ATOMIC_H
