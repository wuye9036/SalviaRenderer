#ifndef EFLIB_STRING_USTRING_H
#define EFLIB_STRING_USTRING_H

#include <eflib/include/platform/config.h>
#include <eflib/include/platform/typedefs.h>
#include <eflib/include/memory/atomic.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace eflib{

	template <typename CharT>
	class fixed_basic_string{
	public:
		typedef std::basic_string<
			CharT, std::char_traits<CharT>, std::allocator<CharT>
		> string_type;
		typedef fixed_basic_string<CharT>				this_type;
		typedef typename string_type::iterator			iterator;
		typedef typename string_type::const_iterator	const_iterator;
		typedef typename string_type::const_reverse_iterator
														const_reverse_iterator;
		typedef typename string_type::size_type			size_type;
		typedef typename string_type::difference_type	difference_type;
		typedef typename string_type::pointer			pointer;
		typedef typename string_type::const_pointer		const_pointer;
		typedef typename string_type::reference			reference;
		typedef typename string_type::const_reference	const_reference;
		typedef typename string_type::value_type		value_type;

	private:
		struct content_data
		{
			content_data()
				: hash_code(0)
			{
			}

			template <typename IteratorT>
			content_data(IteratorT const& begin, IteratorT const& end)
				: hash_code(0)
				, content(begin, end)
				// , begin( content.begin() ), end( content.end() )
			{
			}

			content_data(CharT const* str)
				: hash_code(0)
				, content(str)
				// , begin( content.begin() ), end( content.end() )
			{
			}

			content_data(std::string const& content)
				: hash_code(0)
				, content(content)
				// , begin( content.begin() ), end( content.end() )
			{
			}

			void lock() const
			{
				write_mutex.lock();
			}

			void unlock() const
			{
				write_mutex.unlock();
			}

			mutable spinlock	write_mutex;
			size_t				hash_code;
			string_type			content;
			/*
			const_iterator				begin;
			const_iterator				end;
			*/
		};

		mutable boost::shared_ptr<content_data> data_;

		static boost::shared_ptr<content_data> null_data;


	public:
		fixed_basic_string()
		{
		}

		fixed_basic_string(int /*for initialize static.*/)
		{
		}
		
		fixed_basic_string(std::string const& content)
		{
			 data_.reset( new content_data(content) );
		}
		
		fixed_basic_string(CharT const* str)
		{
			if( str[0] != CharT(0) )
			{
				data_.reset( new content_data(str) );
			}		
		}
		
		fixed_basic_string(const fixed_basic_string& rhs)
			: data_(rhs.data_)
		{
		}

		template <typename IteratorT>
		fixed_basic_string(IteratorT const& begin, IteratorT const& end)
		{
			data_.reset( new content_data(begin, end) );
		}

		fixed_basic_string& operator = (const fixed_basic_string& rhs)
		{
			data_ = rhs.data_;
			return *this;
		}

		fixed_basic_string& operator = (const CharT* str)
		{
			data_.reset( new content_data(str) );
			return *this;
		}

		template <typename IndexT>
		CharT operator [] (IndexT index) const
		{
			return data_->content[index];
		}
		
		fixed_basic_string& operator = (string_type const& str)
		{
			data_.reset( new content_data(str) );
			return *this;
		}

		operator std::string const&() const
		{
			return raw_string();
		}

		const_iterator begin() const
		{
			if(!data_)
			{
				data_.reset( new content_data() );
			}
			return data_->content.begin();
		}

		const_iterator end() const
		{
			if(!data_)
			{
				data_.reset( new content_data() );
			}
			return data_->content.end();
		}

		const_reverse_iterator rbegin() const
		{
			if(!data_)
			{
				data_.reset( new content_data() );
			}
			return data_->content.rbegin();
		}

		const_reverse_iterator rend() const
		{
			if(!data_)
			{
				data_.reset( new content_data() );
			}
			return data_->content.rend();
		}

		bool empty() const
		{
			return !data_ || data_->content.empty();
		}

		std::string const& raw_string() const
		{
			return data_->content;
		}

		std::string& mutable_raw_string()
		{
			if( !data_ )
			{
				data_.reset( new content_data() );
				return data_->content;
			}
			
			if( data_.use_count() == 1 )
			{
				data_->hash_code = 0;
				return data_->content;
			}

			data_->lock();
			content_data* new_content = new content_data(data_->content);
			data_->unlock();
			data_.reset(new_content);
			return data_->content;
		}

		size_t compute_hash() const
		{
			size_t hc = data_->hash_code;
			if(hc == 0)
			{	
				hc = boost::hash_value(data_->content);
				if( !data_->content.empty() )
				{
					assert(hc != 0);
				}
				data_->hash_code = hc; 
			}
			return hc;
		}

		template <typename IteratorT>
		void assign(IteratorT const& begin, IteratorT const& end)
		{
			data_.reset( new content_data(begin, end) );
		}

		void assign(CharT const* v)
		{
			data_.reset( new content_data(v) );
		}

		CharT const* c_str() const
		{
			data_->lock();
			CharT const* ret = data_->content.c_str();
			data_->unlock();
			return ret;
		}

		void append(CharT const* str)
		{
			// Copy on write
			if( data_.use_count() == 1 )
			{
				data_->content.append(str);
			}
			else
			{
				data_->lock();
				content_data* new_content = new content_data(data_->content);
				data_->unlock();
				data_.reset(new_content);
				data_->content.append(str);
			}
			data_->hash_code = 0;
		}

		template <typename CharT> friend bool operator == (
			fixed_basic_string<CharT> const& lhs, fixed_basic_string<CharT> const& rhs
			);
	};


	template <typename CharT>
	size_t hash_value(fixed_basic_string<CharT> const& v)
	{
		return v.compute_hash();
	}

	template <typename CharT>
	bool operator == (
		fixed_basic_string<CharT> const& lhs,
		fixed_basic_string<CharT> const& rhs
		)
	{
		return
			&lhs == &rhs ||
			lhs.data_ == rhs.data_ ||
			lhs.compute_hash() == rhs.compute_hash() ||
			lhs.raw_string() == rhs.raw_string()
			;
	}

	template <typename CharT>
	bool operator == (
		typename fixed_basic_string<CharT>::string_type const& lhs,
		fixed_basic_string<CharT> const& rhs
		)
	{
		return lhs == rhs.raw_string();
	}

	template <typename CharT>
	bool operator == (
		fixed_basic_string<CharT> const& lhs,
		typename fixed_basic_string<CharT>::string_type const& rhs
		)
	{
		return rhs == lhs;
	}

	template <typename CharT>
	bool operator == (fixed_basic_string<CharT> const& lhs, CharT const* rhs)
	{
		return lhs.raw_string() == rhs;
	}

	template <typename StreamT, typename CharT>
	StreamT& operator << (StreamT& ostr, fixed_basic_string<CharT> const& v)
	{
		ostr << v.raw_string();
		return ostr;
	}

	typedef fixed_basic_string<char>	fixed_string;
	typedef fixed_basic_string<wchar_t> fixed_wstring;
}

namespace std
{
	template <typename CharT>
	struct hash< eflib::fixed_basic_string<CharT> >
	{
		size_t operator()(const eflib::fixed_basic_string<CharT>& v) const
		{
			return eflib::hash_value(v);
		}
	};
}
#endif