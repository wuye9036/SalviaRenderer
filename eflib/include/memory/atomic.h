#ifndef EFLIB_MEMORY_ATOMIC_H
#define EFLIB_MEMORY_ATOMIC_H

#include <eflib/include/platform/config.h>
#include <atomic>

namespace eflib{
	class spinlock {
	private:
		enum class LockState: intptr_t
        {
            Locked,
            Unlocked
        };
		std::atomic<LockState> state_;

	public:
		spinlock() : state_(LockState::Unlocked) {}
		
		bool try_lock()
		{
			return state_.exchange(LockState::Locked, std::memory_order_acquire) == LockState::Unlocked;
		}

		void lock()
		{
			while ( !try_lock() )
			{
				/* busy-wait */
			}
		}

		void unlock()
		{
			state_.store(LockState::Unlocked, std::memory_order_release);
		}
	};

	class scoped_spin_locker
	{
	public:
		scoped_spin_locker(spinlock& lock): lock_(lock) { lock_.lock(); }
		~scoped_spin_locker() { lock_.unlock(); }
	private:
		scoped_spin_locker(scoped_spin_locker const&) = delete;
		scoped_spin_locker& operator = (scoped_spin_locker const&) = delete;
		spinlock& lock_;
	};
}


#endif		// SALVIAR_ATOMIC_H
