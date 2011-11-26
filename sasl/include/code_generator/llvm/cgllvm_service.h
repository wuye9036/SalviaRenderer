#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_SERVICE_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_SERVICE_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgllvm_intrins.h>
#include <sasl/enums/builtin_types.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/for.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/metaprog/util.h>
#include <eflib/include/metaprog/enable_if.h>
#include <eflib/include/diagnostics/assert.h>

#include <boost/any.hpp>
#include <boost/type_traits.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>

namespace llvm{
	class Argument;
	class Function;
	class Type;
	class Value;
	class LLVMContext;
	class Module;
	class BasicBlock;
	class ConstantInt;
	class ConstantVector;

	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
	> class IRBuilder;
	class ConstantFolder;

	typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
		DefaultIRBuilder;
}

namespace sasl{
	namespace syntax_tree{
		struct node;
		struct tynode;
		struct function_type;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_sctxt;
class llvm_intrin_cache;

enum abis{
	abi_c,
	abi_llvm,
	abi_unknown
};

class value_tyinfo{
public:
	friend class cg_service;

	enum classifications{
		unknown_type,
		builtin,
		aggregated
	};

	value_tyinfo(
		sasl::syntax_tree::tynode* sty,
		llvm::Type const* cty,
		llvm::Type const* llty
		);

	value_tyinfo( value_tyinfo const& );
	value_tyinfo& operator = ( value_tyinfo const& );

	sasl::syntax_tree::tynode* typtr() const;
	boost::shared_ptr<sasl::syntax_tree::tynode> tysp() const;
	builtin_types hint() const;

	llvm::Type const* llvm_ty( abis abi ) const;

protected:
	value_tyinfo();

	llvm::Type const*			llvm_tys[2];
	sasl::syntax_tree::tynode*	sty;
	classifications				cls;
};

class value_t{
public:
	friend class cg_service;

	value_t();
	value_t( value_t const& );
	value_t& operator = ( value_t const& );

	enum kinds{
		kind_unknown = 0,
		kind_tyinfo_only = 1,
		kind_swizzle = 2,

		kind_value = 4,
		/// \brief Does treat type as reference if ABI is C compatible.
		///  
		/// An important fact is LLVM's ABI is not same as C API.
		/// If structure was passed into function by value,
		/// C compiler will copy a temporary instance and pass in its pointer on x64 calling convention.
		/// But LLVM will push the instance to stack.
		/// So this varaible will qualify the type of arguments/parameters indicates the compiler.
		/// For e.g. we have a prototype:
		///		void foo( struct S );
		/// If is only called by LLVM code, the IR signature will be 
		///		def foo( %S %arg );
		/// But if it maybe called by external function as convention as "C" code,
		/// The IR signature will be generated as following:
		///		def foo( %S* %arg );
		/// And 'kind' the parameter/argument 'arg' is set to 'kind_ref'.
		kind_ref = 8
	};

	/// @name State queriers 
	/// @{

	/// Get service.
	cg_service* service() const;

	/// Return internal llvm value.
	llvm::Value* raw() const;

	/// Load llvm value from value_t.
	llvm::Value* load() const;
	llvm::Value* load( abis abi ) const;
	llvm::Value* load_ref() const;

	void store( value_t const& ) const;

	/// Store llvm value to value_t
	// llvm::Value* store( value_t const& );
	void emplace( value_t const& );
	void emplace( llvm::Value* v, kinds k, abis abi );
	void set_parent( value_t const& v );
	void set_parent( value_t const* v );

	bool storable() const;
	bool load_only() const;

	value_t as_ref() const;
	/// Get type information of value.
	value_tyinfo* get_tyinfo() const;
	/// Get type hint. if type is not built-in type it returns builtin_type::none.
	builtin_types get_hint() const;
	/// Set type hint.
	void set_hint( builtin_types bt );
	/// Get kind.
	kinds get_kind() const;
	/// Get parent. If value is not a member of aggragation, it return NULL.
	value_t* get_parent() const;
	/// Get ABI.
	abis get_abi() const;
	/// Set ABI
	void set_abi( abis abi );
	/// Set Index. It is only make sense if parent is available.
	void set_index( size_t index );
	/// @}

	/// @name Operators
	/// @{
	value_t swizzle( size_t swz_code ) const;
	value_t to_rvalue() const;
	/// @}
protected:
	/// @name Constructor, Destructor, Copy constructor and assignment operator
	/// @{
	value_t(
		value_tyinfo* tyinfo,
		llvm::Value* val, value_t::kinds k, abis abi,
		cg_service* cg
		);
	value_t(
		builtin_types hint,
		llvm::Value* val, value_t::kinds k, abis abi,
		cg_service* cg
		);

	static value_t slice( value_t const& vec, uint32_t masks );
	/// @}

	/// @name Members
	/// @{
	boost::scoped_ptr<value_t>	parent; // For write mask and swizzle.
	uint32_t					masks;
	
	kinds			kind;
	llvm::Value*	val;

	/// Type information
	value_tyinfo*	tyinfo;
	builtin_types	hint;

	/// ABI
	abis			abi;

	cg_service*		cg;
	/// @}
};

template <typename RVT>
struct scope_guard{
	typedef boost::function<RVT ()> on_exit_fn;
	scope_guard( on_exit_fn do_exit ): do_exit(do_exit){}
	~scope_guard(){ do_exit(); }
private:
	on_exit_fn do_exit;
};

struct function_t{
	EFLIB_OPERATOR_BOOL( function_t )
	{
		return NULL != fn;
	}

	function_t();

	/// Get argument's value by index.
	value_t arg( size_t index ) const;
	/// Get argument size.
	size_t arg_size() const;
	/// Set argument name.
	void arg_name( size_t index, std::string const& );
	/// Set arguments name. Size of names must be less than argument size.
	void args_name( std::vector<std::string> const& names );
	/// Return true if argument is a reference.
	bool arg_is_ref( size_t index ) const;
	/// Return true if first argument is pointer to return value.
	bool first_arg_is_return_address() const;
	/// Get ABI
	abis abi() const;
	/// Get return address value.
	llvm::Value* return_address() const;
	/// Return name
	void return_name( std::string const& s );
	/// Set Inline hint
	void inline_hint();

	boost::shared_ptr<value_tyinfo> get_return_ty() const;

	std::vector<llvm::Argument*>		argCache;
	sasl::syntax_tree::function_type*	fnty;
	llvm::Function*						fn;
	bool								c_compatible;
	bool								ret_void;
	cg_service*							cg;
};

struct insert_point_t{
	EFLIB_OPERATOR_BOOL( insert_point_t )
	{
		return block != NULL;
	}

	insert_point_t();

	llvm::BasicBlock* block;
};

class cg_service{
public:
	/** @name Emit expressions
	Some simple overloadable operators such as '+' '-' '*' '/'
	will be implemented in 'cgv_*' classes in operator overload form.
	@{ */
	value_t emit_cond_expr( value_t cond, value_t const& yes, value_t const& no );
	value_t emit_add( value_t const& lhs, value_t const& rhs );
	value_t emit_sub( value_t const& lhs, value_t const& rhs );
	value_t emit_mul( value_t const& lhs, value_t const& rhs );
	value_t emit_dot( value_t const& lhs, value_t const& rhs );

	value_t emit_cmp_lt( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_le( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_eq( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_ne( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_ge( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_gt( value_t const& lhs, value_t const& rhs );

	value_t emit_add_ss_vv( value_t const& lhs, value_t const& rhs );
	value_t emit_sub_ss_vv( value_t const& lhs, value_t const& rhs );

	value_t emit_dot_vv( value_t const& lhs, value_t const& rhs );

	value_t emit_mul_ss_vv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_sv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_sm( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_vm( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_mv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_mm( value_t const& lhs, value_t const& rhs );

	value_t emit_cross( value_t const& lhs, value_t const& rhs );

	value_t emit_extract_col( value_t const& lhs, size_t index );

	template <typename IndexT>
	value_t emit_extract_elem( value_t const& vec, IndexT const& idx ){
		if( vec.storable() ){
			return emit_extract_ref( vec, idx );
		} else {
			return emit_extract_val( vec, idx );
		}
	}

	/// Didn't support swizzle yet.
	value_t emit_extract_elem_mask( value_t const& vec, uint32_t mask );
	value_t emit_swizzle( value_t const& vec, uint32_t mask );
	value_t emit_write_mask( value_t const& vec, uint32_t mask );

	value_t emit_extract_ref( value_t const& lhs, int idx );
	value_t emit_extract_ref( value_t const& lhs, value_t const& idx );
	value_t emit_extract_val( value_t const& lhs, int idx );
	value_t emit_extract_val( value_t const& lhs, value_t const& idx );

	value_t emit_insert_val( value_t const& lhs, value_t const& idx, value_t const& elem_value );
	value_t emit_insert_val( value_t const& lhs, int index, value_t const& elem_value );

	value_t emit_call( function_t const& fn, std::vector<value_t> const& args );
	/** @} */

	/// @name Intrinsics
	/// @{
	value_t emit_sqrt( value_t const& lhs );
	/// @}

	/// @name Emit type casts
	/// @{
	/// Cast between integer types.
	value_t cast_ints( value_t const& v, value_tyinfo* dest_tyi );
	/// Cast integer to float.
	value_t cast_i2f( value_t const& v, value_tyinfo* dest_tyi );
	/// Cast float to integer.
	value_t cast_f2i( value_t const& v, value_tyinfo* dest_tyi );
	/// Cast between float types.
	value_t cast_f2f( value_t const& v, value_tyinfo* dest_tyi );
	/// @}

	/// @name Emit Declarations
	/// @{
	function_t begin_fndecl();
	function_t end_fndecl();
	/// @}

	/// @name Context switchs
	/// @{
	void push_fn( function_t const& fn );
	void pop_fn();

	void set_insert_point( insert_point_t const& ip );
	insert_point_t insert_point() const;
	/// @}

	/// @name Context queries
	/// @{
	bool in_function() const;
	function_t& fn();
	/// @}

	/// @name Emit statement
	/// @{
	void emit_return();
	void emit_return( value_t const&, abis abi );
	/// @}

	/// @name Emit assignment
	/// @{
	void store( value_t& lhs, value_t const& rhs );
	/// @}

	/// @name Emit values
	/// @{
	value_t null_value( value_tyinfo* tyinfo, abis abi );
	value_t null_value( builtin_types bt, abis abi );
	value_t undef_value( builtin_types bt, abis abi );

	value_t create_value( value_tyinfo* tyinfo, llvm::Value* val, value_t::kinds k, abis abi );
	value_t create_value( builtin_types hint, llvm::Value* val, value_t::kinds k, abis abi );
	value_t create_value( value_tyinfo* tyinfo, builtin_types hint, llvm::Value* val, value_t::kinds k, abis abi );

	template <typename T>
	value_t create_constant_scalar( T const& v, value_tyinfo* tyinfo, EFLIB_ENABLE_IF_COND( boost::is_integral<T> ) ){
		Value* ll_val = ConstantInt::get( IntegerType::get( context(), sizeof(T) * 8 ), uint64_t(v), boost::is_signed<T>::value );
		if( tyinfo ){
			return create_scalar( ll_val, tyinfo );
		} else {
			// Guess tyinfo.
			EFLIB_ASSERT_UNIMPLEMENTED();
			return value_t();
		}
	}

	template <typename T>
	value_t create_constant_scalar( T const& v, value_tyinfo* tyinfo, EFLIB_ENABLE_IF_COND( boost::is_floating_point<T> ) ){
		Value* ll_val = ConstantFP::get( Type::getFloatTy( context() ), v );

		if( tyinfo ){
			return create_scalar( ll_val, tyinfo );
		} else {
			// Guess tyinfo.
			EFLIB_ASSERT_UNIMPLEMENTED();
			return value_t();
		} 
	}
	value_t create_scalar( llvm::Value* val, value_tyinfo* tyinfo );

	template <typename T>
	value_t create_constant_vector( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );

	value_t create_vector( std::vector<value_t> const& scalars, abis abi );

	template <typename T>
	value_t create_constant_matrix( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	/// @}

	/// @name Emit variables
	/// @{
	value_t create_variable( value_tyinfo const*, abis abi, std::string const& name );
	value_t create_variable( builtin_types bt, abis abi, std::string const& name );

	function_t fetch_function( boost::shared_ptr<sasl::syntax_tree::function_type> const& fn_node );
	/// @}

	/// @name Type emitters
	/// @{
	boost::shared_ptr<value_tyinfo> create_tyinfo( boost::shared_ptr<sasl::syntax_tree::tynode> const& tyn );
	/// @}

	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( size_t tyid ) = 0;
	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( builtin_types bt ) = 0;

	/// @name Utilities
	/// @{
	
	/// Create a new block at the last of function
	insert_point_t new_block( std::string const& hint, bool set_insert_point );
	/// Jump to the specified block.
	void jump_to( insert_point_t const& );
	/// Jump to the specified block by condition.
	void jump_cond( value_t const& cond_v, insert_point_t const & true_ip, insert_point_t const& false_ip );
	/// Switch to blocks
	void switch_to( value_t const& cond, std::vector< std::pair<value_t, insert_point_t> > const& cases, insert_point_t const& default_branch );
	/// Clean empty blocks of current function.
	virtual void clean_empty_blocks(); 
	virtual cgllvm_sctxt* node_ctxt( boost::shared_ptr<sasl::syntax_tree::node> const& node, bool create_if_need ) = 0;
	/// Get member type information is type is aggrated.
	value_tyinfo* member_tyinfo( value_tyinfo const* agg, size_t index ) const;

	/// @}

	/// @name Fundamentals
	/// @{
	virtual llvm::DefaultIRBuilder* builder() const = 0;
	virtual llvm::LLVMContext&		context() const = 0;
	virtual llvm::Module*			module() const = 0;
	/// @}

	/// @name Bridges
	/// @{
	llvm::Value* select_( llvm::Value* cond, llvm::Value* yes, llvm::Value* no );
	llvm::Type const* type_( builtin_types bt, abis abi );
	llvm::Type const* type_( value_tyinfo const*, abis abi );
	template <typename T>
	llvm::ConstantInt* int_(T v);
	template <typename T>
	llvm::ConstantVector* vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	template <typename T>
	llvm::Value* c_vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	llvm::Function* intrin_( int );
	template <typename FunctionT>
	llvm::Function* intrin_( int );
	/// @}

	/// @name State
	/// @{
	/// Prefer to use external functions as intrinsic.
	bool prefer_externals() const;
	/// Prefer to use scalar code to intrinsic.
	bool prefer_scalar_code() const;
	/// @}
private:
	std::vector<function_t> fn_ctxts;

protected:
	llvm_intrin_cache intrins;
};

END_NS_SASL_CODE_GENERATOR();

#endif