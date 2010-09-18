#ifndef SASL_TEST_TEST_CASES_UTILITY_H
#define SASL_TEST_TEST_CASES_UTILITY_H

#include <boost/preprocessor/stringize.hpp>

#define SASL_TEST_CHECK_RETURN( shared_pointer ) if( !( shared_pointer ) ) return;

#define TEST_CASE_SP_VARIABLE( type_name, var_name ) \
public:		\
	boost::shared_ptr< type_name > var_name () { return LOCVAR_(var_name); }	\
	std::string var_name##_name(){ return std::string( BOOST_PP_STRINGIZE(var_name) ); } \
private:	\
	boost::shared_ptr< type_name > LOCVAR_(var_name);

#define TEST_CASE_CREF_VARIABLE( type_name, var_name ) \
public:		\
	const type_name & var_name () { return LOCVAR_(var_name); }	\
	std::string var_name##_name(){ return std::string( BOOST_PP_STRINGIZE(var_name) ); } \
private:	\
	type_name LOCVAR_(var_name);
	
#define SYNTAX_( type_name ) ::sasl::syntax_tree::##type_name
#define SEMANTIC_( type_name ) ::sasl::semantic::##type_name

#define LOCVAR_( var_name ) var_name##_
#define NAME_( var_name ) var_name##_name()

#endif