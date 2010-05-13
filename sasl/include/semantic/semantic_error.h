#ifndef SASL_SEMANTIC_SEMANTIC_ERROR_H
#define SASL_SEMANTIC_SEMANTIC_ERROR_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/common/compiler_information.h>
#include <boost/shared_ptr.hpp>

#define BEGIN_NS_SASL_SEMANTIC_ERRORS() namespace sasl{ namespace semantic{ namespace errors{
#define END_NS_SASL_SEMANTIC_ERRORS() } } }

BEGIN_NS_SASL_SEMANTIC_ERRORS();

using ::sasl::common::compiler_information_impl;

class semantic_error: public compiler_information_impl{
public:
	static boost::shared_ptr<semantic_error> create( compiler_informations info );
	virtual std::string desc();
protected:
	semantic_error( compiler_informations info );
	semantic_error( const semantic_error& );
	semantic_error& operator = ( const semantic_error& );
};

END_NS_SASL_SEMANTIC_ERRORS();

#endif