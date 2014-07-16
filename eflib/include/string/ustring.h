#ifndef EFLIB_STRING_USTRING_H
#define EFLIB_STRING_USTRING_H

#include <eflib/include/platform/config.h>
#include <eflib/include/platform/typedefs.h>
#include <eflib/include/memory/atomic.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace eflib{

	template <typename CharT>
	class fixed_basic_string
	{
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
		size_t							hash_;
		boost::shared_ptr<string_type>	content_;

	public:
		fixed_basic_string()
		{
			content_ = boost::make_shared<string_type>();
			hash_ = boost::hash_value(*content_);
		}

		fixed_basic_string(string_type const& content)
		{
			content_ = boost::make_shared<string_type>(content);
			hash_ = boost::hash_value(*content_);
		}

		fixed_basic_string(const_pointer content)
		{
			content_ = boost::make_shared<string_type>(content);
			hash_ = boost::hash_value(*content_);
		}

		fixed_basic_string(string_type&& content)
		{
			content_ = boost::make_shared<string_type>( std::move(content) );
			hash_ = boost::hash_value(*content_);
		}

		fixed_basic_string(fixed_basic_string const& rhs)
			: hash_(rhs.hash_), content_(rhs.content_)
		{
		}

		operator string_type const& () const
		{
			return *content_;
		}

		template <typename IteratorT>
		fixed_basic_string(IteratorT const& begin, IteratorT const& end)
		{
			content_ = boost::make_shared<string_type>(begin, end);
			hash_ = boost::hash_value(*content_);
		}

		fixed_basic_string& operator = (fixed_basic_string const& rhs)
		{
			if(&rhs != this)
			{
				hash_ = rhs.hash_;
				content_ = rhs.content_;
			}
			return *this;
		}

		fixed_basic_string& operator = (string_type&& content)
		{
			if( content_.unique() )
			{
				content_->assign( std::move(content) );
			}
			else
			{
				content_ = boost::make_shared<string_type>( std::move(content) );
			}

			hash_ = boost::hash_value(*content_);
			return *this;
		}

		template <typename IndexT>
		value_type operator [] (IndexT index) const
		{
			return (*content_)[index];
		}

		const_iterator begin() const
		{
			return content_->begin();
		}

		const_iterator end() const
		{
			return content_->end();
		}

		const_reverse_iterator rbegin() const
		{
			return content_->rbegin();
		}

		const_reverse_iterator rend() const
		{
			return content_->rend();
		}

		bool empty() const
		{
			return content_->empty();
		}

		string_type const& raw_string() const
		{
			return *content_;
		}

		template <typename IteratorT>
		void assign(IteratorT const& begin, IteratorT const& end)
		{
			if( content_.unique() )
			{
				content_->assign(begin, end);
			}
			else
			{
				content_ = boost::make_shared<string_type>(begin, end);
			}
			
			hash_ = boost::hash_value(*content_);
		}

		void assign (string_type&& content)
		{
			if( content_.unique() )
			{
				content_->assign( std::move(content) );
			}
			else
			{
				content_ = boost::make_shared<string_type>( std::move(content) );
			}

			hash_ = boost::hash_value(*content_);
		}

		void append(const_pointer content)
		{
			if( !content_.unique() )
			{
				content_ = boost::make_shared<string_type>(content_);
			}

			content_->append(content);
			hash_ = boost::hash_value(*content_);
		}

		void append(string_type const& content)
		{
			if( !content_.unique() )
			{
				content_ = boost::make_shared<string_type>(content_);
			}

			content_->append(content);
			hash_ = boost::hash_value(*content_);
		}

		void append(this_type const& content)
		{
			if( !content_.unique() )
			{
				content_ = boost::make_shared<string_type>( content.raw_string() );
			}

			content_->append(content.raw_string());
			hash_ = boost::hash_value(*content_);
		}

		const_pointer c_str() const
		{
			return content_->c_str();
		}

		size_t hash() const
		{
			return hash_;
		}

		template <typename CharU> 
		friend bool operator == (fixed_basic_string<CharU> const& lhs, fixed_basic_string<CharU> const& rhs);
	};


	template <typename CharT>
	size_t hash_value(fixed_basic_string<CharT> const& v)
	{
		return v.hash();
	}

	template <typename CharT>
	bool operator == (
		fixed_basic_string<CharT> const& lhs,
		fixed_basic_string<CharT> const& rhs
		)
	{
		return
			&lhs == &rhs ||
			lhs.content_ == rhs.content_ ||
			lhs.hash() == rhs.hash() ||
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
