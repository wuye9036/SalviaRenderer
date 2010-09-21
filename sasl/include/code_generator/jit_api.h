#ifndef SASL_CODE_GENERATOR_JIT_API_H
#define SASL_CODE_GENERATOR_JIT_API_H

#include <sasl/include/code_generator/forward.h>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <vector>

BEGIN_NS_SASL_CODE_GENERATOR();

class codegen_context;

class jit_engine{
public:
	virtual void* get_function( const std::string& func_name ) = 0;
protected:
	jit_engine();
	virtual ~jit_engine(){}
private:
	jit_engine( const jit_engine& );
	jit_engine& operator = (const jit_engine& );
};


//class jit_engine_manager{
//public:
//	typedef boost::function< bool ( const codegen_context& ) > filter_t;
//	typedef boost::function< boost::shared_ptr<jit_engine_manager> ( const codegen_context& ) > creator_t;
//
//	void register_creator( filter_t filter, creator_t creator );
//	boost::shared_ptr<jit_engine_manager> create( const codegen_context& );
//
//	static boost::shared_ptr<jit_engine_manager> instance();
//
//private:
//	std::vector< std::pair< filter_t, creator_t > > creators;
//
//	static boost::shared_ptr<jit_engine_manager> ins;
//	static boost::shared_ptr<boost::mutex> mtx;
//};

END_NS_SASL_CODE_GENERATOR();

#endif