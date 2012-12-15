#ifndef EFLIB_STRING_USTRING_H
#define EFLIB_STRING_USTRING_H

#include <eflib/include/platform/config.h>
#include <eflib/include/memory/atomic.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace eflib{

	template <typename CharT>
	class fixed_basic_string{
	private:
		typedef std::basic_string<
			CharT, std::char_traits<CharT>, std::allocator<CharT>
		> string_type;

	public:
		typedef typename string_type::iterator			iterator;
		typedef typename string_type::const_iterator	const_iterator;
		typedef typename string_type::size_type			size_type;
		typedef typename string_type::difference_type	difference_type;
		typedef typename string_type::pointer			pointer;
		typedef typename string_type::const_pointer		const_pointer;
		typedef typename string_type::reference			reference;
		typedef typename string_type::const_reference	const_reference;
		typedef typename string_type::value_type		value_type;

	private:
		class content_data
		{
			content_data(std::string const& content)
				: is_hashed(0), hash_code(0), content(content)
				// , begin( content.begin() ), end( content.end() )
			{
			}

			atomic<int32_t>				is_hashed;
			size_t						hash_code;
			string_type					content;
			/*
			const_iterator				begin;
			const_iterator				end;
			*/
		};

		boost::shared_ptr<content_data> data_;

	public:
		fixed_basic_string(std::string const& content)
			: data_(content)
		{
		}
		
		fixed_basic_string(const fixed_basic_string& rhs)
			: data_(rhs.data_)
		{
		}

		fixed_basic_string& operator = (const fixed_basic_string& rhs)
		{
			data_ = rhs.data_;
		}

		const_iterator begin() const
		{
			return data_->content.begin();
		}

		const_iterator end() const
		{
			return data_->content.end();
		}

		std::string const& raw_string() const
		{
			return data_->content;
		}

		size_t compute_hash() const
		{
			if(!data_->is_hashed)
			{	
				data_->hash_code = boost::hash_value(data_->content);
				data_->is_hashed = 1;
			}
			return data_->hash_code;
		}
	};
}

#endif