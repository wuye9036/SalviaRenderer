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

template <typename CharT> struct ustring_data{
	ustring_data( std::basic_string<CharT> const & data )
		: data(data)
	{
		hash_code = boost::hash_value(data);
	}
	ustring_data(CharT const * data )
		: data(data)
	{
		hash_code = boost::hash_value(data);
	}
	std::basic_string<CharT> data;
	size_t hash_code;
};

template <typename CharT>
class basic_ustring{

public:
	basic_string(){}

	basic_ustring( CharT const * val ){
		pdata = new ustring_data(val);
	}

	basic_ustring( std::basic_string<CharT> const& val ){
		pdata.reset( new ustring_data(val) );
	}

	basic_ustring( basic_ustring<CharT> const& val ){
		pdata = val.pdata;
	}

	~basic_ustring(){}

	CharT const * c_str() const{
		if( pdata ){
			return pdata->data.c_str();
		}
		return NULL;
	}

	basic_ustring& operator = ( basic_ustring<CharT> const& val ){
		pdata = val.pdata;
		return *this;
	}

	bool operator == ( basic_ustring<CharT> const& data ){
		return
			pdata == data.pdata
			|| pdata->hash_code == data.pdata->hash_code
			|| pdata->data == data.pdata->data
			;
	}

	bool operator != ( basic_ustring<CharT> const& data ){
		return !( *this == data );
	}

private:
	shared_ptr<ustring_data> pdata;
};

}

#endif