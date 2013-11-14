#pragma once

#include <eflib/include/utility/operator_bool.h>
#include <eflib/include/math/vector.h>
#include <eflib/include/math/matrix.h>
#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/function_pointer.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/sizeof.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/test_tools.hpp>
#include <eflib/include/platform/boost_end.h>

#if defined(EFLIB_WINDOWS)
#include <excpt.h>
#endif

namespace sasl
{
	namespace drivers
	{
		class compiler;
	}
	namespace semantic
	{
		class symbol;
	}
	namespace codegen
	{
		EFLIB_DECLARE_CLASS_SHARED_PTR(module_vmcode);
	}
	namespace common
	{
		class diag_chat;
		class diag_item;
	}
}

using sasl::drivers::compiler;
using sasl::common::diag_chat;
using sasl::common::diag_item;
using sasl::semantic::symbol;
EFLIB_USING_SHARED_PTR(sasl::codegen, module_vmcode);

using eflib::vector_;
using eflib::matrix_;

using boost::function_types::result_type;
using boost::function_types::function_pointer;
using boost::function_types::parameter_types;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;
using boost::mpl::_;
using boost::mpl::if_;
using boost::mpl::or_;
using boost::mpl::push_front;
using boost::mpl::sizeof_;
using boost::mpl::transform;
using boost::is_arithmetic;
using boost::is_pointer;
using boost::is_same;
using boost::add_reference;
using boost::enable_if_c;
using boost::enable_if;
using boost::disable_if;

using std::string;

#if defined(EFLIB_WINDOWS)
#pragma warning(push)
#pragma warning(disable: 4701) // C4701: potentially uninitialized local variable 'X' used
#pragma warning(disable: 4244) // C4244: conversion from 'X' to 'Y', possible loss of data

template <typename Fn>
class jit_function_forward_base{
protected:
	typedef typename result_type<Fn>::type result_t;
	typedef result_t* result_type_pointer;
	typedef typename parameter_types<Fn>::type param_types;
	typedef typename boost::mpl::transform< param_types, if_< or_< is_arithmetic<_>, is_pointer<_> >, _, add_reference<_> > >::type param_refs;
	typedef typename if_<
	is_same<result_t, void>,
	param_refs,
		typename push_front<param_refs, result_type_pointer>::type
	>::type	callee_parameters;
	typedef typename push_front<callee_parameters, void>::type
		callee_return_parameters;
public:
	EFLIB_OPERATOR_BOOL( jit_function_forward_base<Fn> ){ return callee != NULL; }
	typedef typename function_pointer<callee_return_parameters>::type
		callee_ptr_t;
	callee_ptr_t callee;
	std::string  name;
	jit_function_forward_base():callee(NULL){}
	void on_error(char const* desc) { BOOST_ERROR( (std::string(desc) + " @ " + name).c_str() ); }
};

void invoke( void* callee, void* psi, void* pbi, void* pso, void* pbo );

template <typename RT, typename Fn>
class jit_function_forward: public jit_function_forward_base<Fn>{
public:
	using typename jit_function_forward_base<Fn>::result_t;
	result_t operator ()(){
		result_t tmp;
#if defined(EFLIB_MSVC)
		__try
		{
			callee(&tmp);
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			on_error("SEH exception was raised");
		}
#else
		this->callee(&tmp);
#endif
		return tmp;
	}

	template <typename T0>
	result_t operator() (T0 p0 ){
		result_t tmp;
#if defined(EFLIB_MSVC)
		__try
		{
			callee(&tmp, p0);
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			on_error("SEH exception was raised");
		}
#else
		this->callee(&tmp, p0);
#endif
		return tmp;
	}

	template <typename T0, typename T1>
	result_t operator() (T0 p0, T1 p1){
		result_t tmp;
#if defined(EFLIB_MSVC)
		__try
		{
			callee(&tmp, p0, p1);
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			on_error("SEH exception was raised");
		}
#else
		this->callee(&tmp, p0, p1);
#endif
		return tmp;
	}

	template <typename T0, typename T1, typename T2>
	result_t operator() (T0 p0, T1 p1, T2 p2){
		result_t tmp;
#if defined(EFLIB_MSVC)
		__try
		{
			callee(&tmp, p0, p1, p2);
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			on_error("SEH exception was raised");
		}
#else
		this->callee(&tmp, p0, p1, p2);
#endif
		return tmp;
	}
};

template <typename Fn>
class jit_function_forward<void, Fn>: public jit_function_forward_base<Fn>{
public:
	using typename jit_function_forward_base<Fn>::result_t;
	result_t operator ()(){
		this->callee();
	}

	template <typename T0>
	result_t operator() (T0 p0 ){
		this->callee(p0);
	}

	template <typename T0, typename T1>
	result_t operator() (T0 p0, T1 p1){
		this->callee(p0, p1);
	}

	template <typename T0, typename T1, typename T2>
	result_t operator() (T0 p0, T1 p1, T2 p2){
		this->callee(p0, p1, p2);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	result_t operator() (T0 p0, T1 p1, T2 p2, T3 p3){
		this->callee(p0, p1, p2, p3);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	result_t operator() (T0* psi, T1* pbi, T2* pso, T3* pbo){
#if defined(EFLIB_MSVC)
		__try
		{
			invoke( (void*)callee, psi, pbi, pso, pbo );
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			on_error("SEH exception was raised");
		}
#else
		invoke( (void*)(this->callee), psi, pbi, pso, pbo );
#endif
	}
};

#pragma warning(pop)
#endif

template <typename Fn>
class jit_function: public jit_function_forward< typename result_type<Fn>::type, Fn >
{};

class jit_fixture {
public:
	jit_fixture() {}

	void init_g ( string const& file_name );
	void init_vs( string const& file_name );
	void init_ps( string const& file_name );
	void init( string const& file_name, string const& options );

	void* function ( string const& unmangled_name );

	template <typename FunctionT>
	void function( FunctionT& fn, string const& unmangled_name )
	{
		fn.callee = reinterpret_cast<typename FunctionT::callee_ptr_t>( function(unmangled_name) );
		fn.name = unmangled_name;
	}

	void set_function( void* fn, string const& unmangled_name );

	void set_raw_function( void* fn, string const& mangled_name );

	~jit_fixture(){}

	shared_ptr<compiler>	drv;
	symbol*					root_sym;
	module_vmcode_ptr		vmc;
	shared_ptr<diag_chat>	diags;
private:
	jit_fixture(jit_fixture const&);
	jit_fixture& operator = (jit_fixture const&);
};

#define INIT_JIT_FUNCTION(fn_name) function( fn_name, #fn_name ); BOOST_REQUIRE(fn_name);
#define JIT_FUNCTION( signature, name ) jit_function<signature> name; function(name, #name); BOOST_REQUIRE(name);

using eflib::int3;

typedef vector_<char,2>		char2;
typedef vector_<char,3>		char3;
typedef vector_<char,3>		bool3;
typedef vector_<char,4>		bool4;
typedef vector_<uint32_t,2>	uint2;
typedef vector_<uint32_t,3>	uint3;

typedef matrix_<char,3,2>		bool2x3;
typedef matrix_<char,3,3>		bool3x3;
typedef matrix_<char,4,3>		bool3x4;
typedef matrix_<int32_t,3,2>	int2x3;
typedef matrix_<int32_t,3,3>	int3x3;
typedef matrix_<int32_t,4,3>	int3x4;
typedef matrix_<uint32_t,3,2>	uint2x3;
typedef matrix_<float,3,2>		float2x3;
typedef matrix_<float,3,3>		float3x3;
typedef matrix_<float,4,3>		float3x4;


string	make_command( string const& file_name, string const& options);
bool	print_diagnostic( diag_chat*, diag_item* item );
