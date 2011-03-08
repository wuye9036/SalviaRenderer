#ifndef EFLIB_MEMORY_ATOMIC_H
#define EFLIB_MEMORY_ATOMIC_H

#include <eflib/include/platform/config.h>

#ifdef EFLIB_WINDOWS
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif

namespace eflib{

	template <typename T>
	class atomic
	{
	public:
		explicit atomic(T const & rhs);
		atomic(atomic const & rhs);

		T value() const;
		void value(T const & rhs);

		atomic& operator=(T const & rhs);
		atomic& operator=(atomic const & rhs);

		bool operator<(T const & rhs) const;
		bool operator<(atomic const & rhs) const;
		bool operator<=(int32_t const & rhs) const;
		bool operator<=(T const & rhs) const;
		bool operator>(T const & rhs) const;
		bool operator>(atomic const & rhs) const;
		bool operator>=(T const & rhs) const;
		bool operator>=(atomic const & rhs) const;

		bool operator==(T const & rhs) const;
		bool operator==(atomic const & rhs) const;

		bool operator!=(T const & rhs) const;
		bool operator!=(atomic const & rhs) const;

		T const & operator++();
		T const & operator--();
		T operator++(int);
		T operator--(int);

		atomic& operator+=(T const & rhs);
		atomic& operator+=(atomic const & rhs);
		atomic& operator-=(T const & rhs);
		atomic& operator-=(atomic const & rhs);
		atomic& operator&=(T const & rhs);
		atomic& operator&=(atomic const & rhs);
		atomic& operator|=(T const & rhs);
		atomic& operator|=(atomic const & rhs);
		atomic& operator^=(T const & rhs);
		atomic& operator^=(atomic const & rhs);

		bool cas(T const & old_val, T const & new_val);
	};

	template <>
	class atomic<int32_t>
	{
	public:
		atomic()
		{
		}

		explicit atomic(int32_t rhs)
		{
			this->value(rhs);
		}

		int32_t value() const
		{
#ifdef BOOST_WINDOWS
			return value_;
#elif (__GNUC__ >= 4)
			return __sync_fetch_and_add(&value_, 0);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			return __gnu_cxx::__exchange_and_add(&value_, 0);
#endif
		}

		void value(int32_t const & rhs)
		{
#ifdef BOOST_WINDOWS
			InterlockedExchange(reinterpret_cast<long*>(&value_), rhs);
#elif (__GNUC__ >= 4)
			value_ = rhs;
			__sync_synchronize();
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			value_ = rhs;
#endif
		}

		atomic& operator=(int32_t const & rhs)
		{
			this->value(rhs);
			return *this;
		}
		atomic& operator=(atomic const & rhs)
		{
			this->value(rhs.value_);
			return *this;
		}

		bool cas(int32_t const & old_val, int32_t const & new_val)
		{
#ifdef BOOST_WINDOWS
			return old_val == InterlockedCompareExchange(reinterpret_cast<long*>(&value_), new_val, old_val);
#elif (__GNUC__ >= 4)
			return __sync_bool_compare_and_swap(&value_, old_val, new_val);
#else
			return old_val == __cmpxchg(&value_, old_val, new_val, sizeof(old_val));
#endif
		}

		bool operator<(int32_t const & rhs) const
		{
			return this->value() < rhs;
		}
		bool operator<(atomic const & rhs) const
		{
			return this->value() < rhs.value();
		}
		bool operator<=(int32_t const & rhs) const
		{
			return this->value() <= rhs;
		}
		bool operator<=(atomic const & rhs) const
		{
			return this->value() <= rhs.value();
		}
		bool operator>(int32_t const & rhs) const
		{
			return this->value() > rhs;
		}
		bool operator>(atomic const & rhs) const
		{
			return this->value() > rhs.value();
		}
		bool operator>=(int32_t const & rhs) const
		{
			return this->value() >= rhs;
		}
		bool operator>=(atomic const & rhs) const
		{
			return this->value() >= rhs.value();
		}

		bool operator==(int32_t const & rhs) const
		{
			return this->value() == rhs;
		}
		bool operator==(atomic const & rhs) const
		{
			return this->value() == rhs.value();
		}

		bool operator!=(int32_t const & rhs) const
		{
			return this->value() != rhs;
		}
		bool operator!=(atomic const & rhs) const
		{
			return this->value() != rhs.value();
		}

		atomic& operator+=(int32_t const & rhs)
		{
#ifdef BOOST_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), rhs);
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs);
#else
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator+=(atomic const & rhs)
		{
#ifdef BOOST_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), rhs.value_);
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs.value_);
#else
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}

		atomic& operator-=(int32_t const & rhs)
		{
#ifdef BOOST_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), -rhs);
#elif (__GNUC__ >= 4)
			__sync_sub_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs);
#else
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator-=(atomic const & rhs)
		{
#ifdef EFLIB_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), -rhs.value_);
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, -rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs.value_);
#else
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}

		atomic& operator*=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand * rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator/=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand / rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator%=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand % rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator&=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand & rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator|=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand | rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator^=(int32_t const & rhs)
		{
			int32_t comperand;
			int32_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand ^ rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		int32_t const & operator++()
		{
#ifdef EFLIB_WINDOWS
			InterlockedIncrement(reinterpret_cast<long*>(&value_));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, 1);
#else
			this->operator+=(1);
#endif
			return value_;
		}

		int32_t const & operator--()
		{
#ifdef EFLIB_WINDOWS
			InterlockedDecrement(reinterpret_cast<long*>(&value_));
#elif (__GNUC__ >= 4)
			__sync_sub_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -1);
#else
			this->operator-=(1);
#endif
			return value_;
		}

		int32_t operator++(int)
		{
			long old_val;
			long new_val;
			do
			{
				old_val = value_;
				new_val = old_val + 1;		
			} while (!this->cas(old_val, new_val));
			return old_val;
		}

		int32_t operator--(int)
		{
			long old_val;
			long new_val;
			do
			{
				old_val = value_;
				new_val = old_val - 1;		
			} while (!this->cas(old_val, new_val));
			return old_val;
		}

	private:
#ifdef EFLIB_WINDOWS
		mutable int32_t value_;
#elif (__GNUC__ >= 4)
		mutable int32_t value_;
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
		mutable _Atomic_word value_;
#else
		mutable int32_t value_;
#endif
	};

#ifdef _WIN64

	template <>
	class atomic<int64_t>
	{
	public:
		atomic()
		{
		}

		explicit atomic(int64_t rhs)
		{
			this->value(rhs);
		}

		int64_t value() const
		{
#ifdef EFLIB_WINDOWS
			return value_;
#elif (__GNUC__ >= 4)
			return __sync_fetch_and_add(&value_, 0);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			return __gnu_cxx::__exchange_and_add(&value_, 0);
#endif
		}

		void value(int64_t const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchange64(reinterpret_cast<__int64*>(&value_), rhs);
#elif (__GNUC__ >= 4)
			value_ = rhs;
			__sync_synchronize();
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			value_ = rhs;
#endif
		}

		atomic& operator=(int64_t const & rhs)
		{
			this->value(rhs);
			return *this;
		}
		atomic& operator=(atomic const & rhs)
		{
			this->value(rhs.value_);
			return *this;
		}

		bool cas(int64_t const & old_val, int64_t const & new_val)
		{
#ifdef EFLIB_WINDOWS
			return old_val == _InterlockedCompareExchange64(reinterpret_cast<__int64*>(&value_), new_val, old_val);
#elif (__GNUC__ >= 4)
			return __sync_bool_compare_and_swap(&value_, old_val, new_val);
#else
			return old_val == __cmpxchg(&value_, old_val, new_val, sizeof(old_val));
#endif
		}

		bool operator<(int64_t const & rhs) const
		{
			return this->value() < rhs;
		}
		bool operator<(atomic const & rhs) const
		{
			return this->value() < rhs.value();
		}
		bool operator<=(int64_t const & rhs) const
		{
			return this->value() <= rhs;
		}
		bool operator<=(atomic const & rhs) const
		{
			return this->value() <= rhs.value();
		}
		bool operator>(int64_t const & rhs) const
		{
			return this->value() > rhs;
		}
		bool operator>(atomic const & rhs) const
		{
			return this->value() > rhs.value();
		}
		bool operator>=(int64_t const & rhs) const
		{
			return this->value() >= rhs;
		}
		bool operator>=(atomic const & rhs) const
		{
			return this->value() >= rhs.value();
		}

		bool operator==(int64_t const & rhs) const
		{
			return this->value() == rhs;
		}
		bool operator==(atomic const & rhs) const
		{
			return this->value() == rhs.value();
		}

		bool operator!=(int64_t const & rhs) const
		{
			return this->value() != rhs;
		}
		bool operator!=(atomic const & rhs) const
		{
			return this->value() != rhs.value();
		}

		atomic& operator+=(int64_t const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchangeAdd64(reinterpret_cast<__int64*>(&value_), rhs);
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs);
#else
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator+=(atomic const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchangeAdd64(reinterpret_cast<__int64*>(&value_), rhs.value_);
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs.value_);
#else
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}

		atomic& operator-=(int64_t const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchangeAdd64(reinterpret_cast<__int64*>(&value_), -rhs);
#elif (__GNUC__ >= 4)
			__sync_sub_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs);
#else
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator-=(atomic const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchangeAdd64(reinterpret_cast<__int64*>(&value_), -rhs.value_);
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, -rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs.value_);
#else
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}

		atomic& operator*=(int64_t const & rhs)
		{
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand * rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator/=(int64_t const & rhs)
		{
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand / rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator%=(int64_t const & rhs)
		{
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand % rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator&=(int64_t const & rhs)
		{
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand & rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator|=(int64_t const & rhs)
		{
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand | rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator^=(int64_t const & rhs)
		{
			int64_t comperand;
			int64_t exchange;
			do
			{
				comperand = value_;
				exchange = comperand ^ rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic const & operator++()
		{
#ifdef EFLIB_WINDOWS
			_InterlockedIncrement64(reinterpret_cast<__int64*>(&value_));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, 1);
#else
			this->operator+=(1);
#endif
			return *this;
		}

		atomic const & operator--()
		{
#ifdef EFLIB_WINDOWS
			_InterlockedDecrement64(reinterpret_cast<__int64*>(&value_));
#elif (__GNUC__ >= 4)
			__sync_sub_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -1);
#else
			this->operator-=(1);
#endif
			return *this;
		}

		atomic operator++(int)
		{
			atomic tmp = *this;
			++ *this;
			return tmp;
		}

		atomic operator--(int)
		{
			atomic tmp = *this;
			-- *this;
			return tmp;
		}

	private:
#ifdef EFLIB_WINDOWS
		mutable int64_t value_;
#elif (__GNUC__ >= 4)
		mutable int64_t value_;
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
		mutable _Atomic_word value_;
#else
		mutable int64_t value_;
#endif
	};

	template <typename T>
	class atomic<T*>
	{
	public:
		atomic()
		{
		}

		explicit atomic(T* rhs)
		{
			this->value(rhs);
		}

		T* value() const
		{
#ifdef EFLIB_WINDOWS
			return value_;
#elif (__GNUC__ >= 4)
			return __sync_fetch_and_add(&value_, 0);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			return __gnu_cxx::__exchange_and_add(&value_, 0);
#endif
		}

		void value(T* const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchange64(reinterpret_cast<__int64*>(&value_), reinterpret_cast<__int64>(rhs));
#elif (__GNUC__ >= 4)
			value_ = rhs;
			__sync_synchronize();
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			value_ = rhs;
#endif
		}

		atomic& operator=(T* const & rhs)
		{
			this->value(rhs);
			return *this;
		}
		atomic& operator=(atomic const & rhs)
		{
			this->value(rhs.value_);
			return *this;
		}

		bool cas(T* const & old_val, T* const & new_val)
		{
#ifdef EFLIB_WINDOWS
			return reinterpret_cast<__int64>(old_val) == _InterlockedCompareExchange64(reinterpret_cast<__int64*>(&value_), reinterpret_cast<__int64>(new_val), reinterpret_cast<__int64>(old_val));
#elif (__GNUC__ >= 4)
			return __sync_bool_compare_and_swap(&value_, old_val, new_val);
#else
			return old_val == __cmpxchg(&value_, old_val, new_val, sizeof(old_val));
#endif
		}

		bool operator<(T* const & rhs) const
		{
			return this->value() < rhs;
		}
		bool operator<(atomic const & rhs) const
		{
			return this->value() < rhs.value();
		}
		bool operator<=(T* const & rhs) const
		{
			return this->value() <= rhs;
		}
		bool operator<=(atomic const & rhs) const
		{
			return this->value() <= rhs.value();
		}
		bool operator>(T* const & rhs) const
		{
			return this->value() > rhs;
		}
		bool operator>(atomic const & rhs) const
		{
			return this->value() > rhs.value();
		}
		bool operator>=(T* const & rhs) const
		{
			return this->value() >= rhs;
		}
		bool operator>=(atomic const & rhs) const
		{
			return this->value() >= rhs.value();
		}

		bool operator==(T* const & rhs) const
		{
			return this->value() == rhs;
		}
		bool operator==(atomic const & rhs) const
		{
			return this->value() == rhs.value();
		}

		bool operator!=(T* const & rhs) const
		{
			return this->value() != rhs;
		}
		bool operator!=(atomic const & rhs) const
		{
			return this->value() != rhs.value();
		}

		atomic& operator+=(T* const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchangeAdd64(reinterpret_cast<__int64*>(&value_), reinterpret_cast<__int64>(rhs));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs);
#else
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator+=(atomic const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchangeAdd64(reinterpret_cast<__int64*>(&value_), reinterpret_cast<__int64>(rhs.value_));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs.value_);
#else
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}

		atomic& operator-=(T* const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchangeAdd64(reinterpret_cast<__int64*>(&value_), reinterpret_cast<__int64>(-rhs));
#elif (__GNUC__ >= 4)
			__sync_sub_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs);
#else
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator-=(atomic const & rhs)
		{
#ifdef EFLIB_WINDOWS
			_InterlockedExchangeAdd64(reinterpret_cast<__int64*>(&value_), reinterpret_cast<__int64>(-rhs.value_));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, -rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs.value_);
#else
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}

		atomic& operator*=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand * rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator/=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand / rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator%=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand % rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator&=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand & rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator|=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand | rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator^=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand ^ rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic const & operator++()
		{
#ifdef EFLIB_WINDOWS
			_InterlockedIncrement64(reinterpret_cast<__int64*>(&value_));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, 1);
#else
			this->operator+=(1);
#endif
			return *this;
		}

		atomic const & operator--()
		{
#ifdef EFLIB_WINDOWS
			_InterlockedDecrement64(reinterpret_cast<__int64*>(&value_));
#elif (__GNUC__ >= 4)
			__sync_sub_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -1);
#else
			this->operator-=(1);
#endif
			return *this;
		}

		atomic operator++(int)
		{
			atomic tmp = *this;
			++ *this;
			return tmp;
		}

		atomic operator--(int)
		{
			atomic tmp = *this;
			-- *this;
			return tmp;
		}

	private:
#ifdef EFLIB_WINDOWS
		mutable T* value_;
#elif (__GNUC__ >= 4)
		mutable T* value_;
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
		mutable _Atomic_word value_;
#else
		mutable T* value_;
#endif
	};

#else

	template <typename T>
	class atomic<T*>
	{
	public:
		atomic()
		{
		}

		explicit atomic(T* rhs)
		{
			this->value(rhs);
		}

		T* value() const
		{
#ifdef EFLIB_WINDOWS
			return value_;
#elif (__GNUC__ >= 4)
			return __sync_fetch_and_add(&value_, 0);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			return __gnu_cxx::__exchange_and_add(&value_, 0);
#endif
		}

		void value(T* const & rhs)
		{
#ifdef EFLIB_WINDOWS
			InterlockedExchange(reinterpret_cast<long*>(&value_), reinterpret_cast<long>(rhs));
#elif (__GNUC__ >= 4)
			value_ = rhs;
			__sync_synchronize();
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			value_ = rhs;
#endif
		}

		atomic& operator=(T* const & rhs)
		{
			this->value(rhs);
			return *this;
		}
		atomic& operator=(atomic const & rhs)
		{
			this->value(rhs.value_);
			return *this;
		}

		bool cas(T* const & old_val, T* const & new_val)
		{
#ifdef EFLIB_WINDOWS
			return reinterpret_cast<long>(old_val) == InterlockedCompareExchange(reinterpret_cast<long*>(&value_), reinterpret_cast<long>(new_val), reinterpret_cast<long>(old_val));
#elif (__GNUC__ >= 4)
			return __sync_bool_compare_and_swap(&value_, old_val, new_val);
#else
			return old_val == __cmpxchg(&value_, old_val, new_val, sizeof(old_val));
#endif
		}

		bool operator<(T* const & rhs) const
		{
			return this->value() < rhs;
		}
		bool operator<(atomic const & rhs) const
		{
			return this->value() < rhs.value();
		}
		bool operator<=(T* const & rhs) const
		{
			return this->value() <= rhs;
		}
		bool operator<=(atomic const & rhs) const
		{
			return this->value() <= rhs.value();
		}
		bool operator>(T* const & rhs) const
		{
			return this->value() > rhs;
		}
		bool operator>(atomic const & rhs) const
		{
			return this->value() > rhs.value();
		}
		bool operator>=(T* const & rhs) const
		{
			return this->value() >= rhs;
		}
		bool operator>=(atomic const & rhs) const
		{
			return this->value() >= rhs.value();
		}

		bool operator==(T* const & rhs) const
		{
			return this->value() == rhs;
		}
		bool operator==(atomic const & rhs) const
		{
			return this->value() == rhs.value();
		}

		bool operator!=(T* const & rhs) const
		{
			return this->value() != rhs;
		}
		bool operator!=(atomic const & rhs) const
		{
			return this->value() != rhs.value();
		}

		atomic& operator+=(T* const & rhs)
		{
#ifdef EFLIB_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), reinterpret_cast<long>(rhs));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs);
#else
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator+=(atomic const & rhs)
		{
#ifdef EFLIB_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), reinterpret_cast<long>(rhs.value_));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, rhs.value_);
#else
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand + rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}

		atomic& operator-=(T* const & rhs)
		{
#ifdef EFLIB_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), reinterpret_cast<long>(-rhs));
#elif (__GNUC__ >= 4)
			__sync_sub_and_fetch(&value_, rhs);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs);
#else
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}
		atomic& operator-=(atomic const & rhs)
		{
#ifdef EFLIB_WINDOWS
			InterlockedExchangeAdd(reinterpret_cast<long*>(&value_), reinterpret_cast<long>(-rhs.value_));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, -rhs.value_);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -rhs.value_);
#else
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand - rhs.value_;
			} while (!this->cas(comperand, exchange));
#endif
			return *this;
		}

		atomic& operator*=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand * rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator/=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand / rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator%=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand % rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator&=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand & rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator|=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand | rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic& operator^=(T* const & rhs)
		{
			T* comperand;
			T* exchange;
			do
			{
				comperand = value_;
				exchange = comperand ^ rhs;
			} while (!this->cas(comperand, exchange));
			return *this;
		}

		atomic const & operator++()
		{
#ifdef EFLIB_WINDOWS
			InterlockedIncrement(reinterpret_cast<long*>(&value_));
#elif (__GNUC__ >= 4)
			__sync_add_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, 1);
#else
			this->operator+=(1);
#endif
			return *this;
		}

		atomic const & operator--()
		{
#ifdef EFLIB_WINDOWS
			InterlockedDecrement(reinterpret_cast<long*>(&value_));
#elif (__GNUC__ >= 4)
			__sync_sub_and_fetch(&value_, 1);
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
			__gnu_cxx::__exchange_and_add(&value_, -1);
#else
			this->operator-=(1);
#endif
			return *this;
		}

		atomic operator++(int)
		{
			atomic tmp = *this;
			++ *this;
			return tmp;
		}

		atomic operator--(int)
		{
			atomic tmp = *this;
			-- *this;
			return tmp;
		}

	private:
#ifdef EFLIB_WINDOWS
		mutable T* value_;
#elif (__GNUC__ >= 4)
		mutable T* value_;
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
		mutable _Atomic_word value_;
#else
		mutable T* value_;
#endif
	};
#endif
}
#endif		// SOFTART_ATOMIC_H
