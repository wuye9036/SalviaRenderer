#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_SERVICE_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_SERVICE_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgllvm_intrins.h>
#include <sasl/enums/builtin_types.h>

#include <eflib/include/metaprog/enable_if.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/metaprog/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/type_traits.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace llvm{
	class Constant;
	class LLVMContext;
	class Module;
	class Type;
	class Value;
	class Argument;
	class Function;
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

class value_t;
class cgllvm_sctxt;
class llvm_module_impl;
class cg_service;

enum abis{
	abi_c,
	abi_llvm,
	abi_vectorize,
	abi_package,
	abi_unknown
};

enum value_kinds{
	vkind_unknown = 0,
	vkind_tyinfo_only = 1,
	vkind_swizzle = 2,

	vkind_value = 4,
	/// \brief Does treat type as reference if ABI is C compatible.
	///  
	/// An important fact is LLVM's ABI is not same as C API.
	/// If structure was passed into function by value,
	/// C compiler will copy a temporary instance and pass in its pointer on x64 calling convention.
	/// But LLVM will push the instance to stack.
	/// So this variable will qualify the type of arguments/parameters indicates the compiler.
	/// For e.g. we have a prototype:
	///		void foo( struct S );
	/// If is only called by LLVM code, the IR signature will be 
	///		def foo( %S %arg );
	/// But if it maybe called by external function as convention as "C" code,
	/// The IR signature will be generated as following:
	///		def foo( %S* %arg );
	/// And 'kind' the parameter/argument 'arg' is set to 'vkind_ref'.
	vkind_ref = 8
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
		llvm::Type* ty_c,
		llvm::Type* ty_llvm,
		llvm::Type* ty_vec,
		llvm::Type* ty_pkg
		);

	value_tyinfo( value_tyinfo const& );
	value_tyinfo& operator = ( value_tyinfo const& );

	sasl::syntax_tree::tynode* tyn_ptr() const;
	boost::shared_ptr<sasl::syntax_tree::tynode> tyn_shared() const;
	builtin_types hint() const;
	llvm::Type* ty( abis abi ) const;

protected:
	value_tyinfo();

	llvm::Type*					tys[4];
	sasl::syntax_tree::tynode*	tyn;
	classifications				cls;
};

class value_t{
public:
	friend class cg_service;

	value_t();
	value_t( value_t const& );
	value_t& operator = ( value_t const& );

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
	void emplace( value_t const& );
	void emplace( llvm::Value* v, value_kinds k, abis abi );

	bool storable() const;
	bool load_only() const;

	value_t as_ref() const;
	
	value_tyinfo*	tyinfo() const;				///< Get type information of value.

	builtin_types	hint() const;				///< Get type hint. if type is not built-in type it returns builtin_type::none.
	void			hint( builtin_types bt );	///< Set type hint.
	
	value_kinds		kind() const;				///< Get kind.
	void			kind( value_kinds vkind );	///< Set kind.
	
	value_t*		parent() const;				///< Get parent. If value is not a member of aggragation, it return NULL.
	void			parent( value_t const& v );
	void			parent( value_t const* v );
	
	abis			abi() const;				///< Get ABI.
	void			abi( abis abi );			///< Set ABI

	void			index( size_t v );			///< Set Index. It is only make sense if parent is available.
	uint32_t		masks() const;				///< Get masks
	void			masks( uint32_t v );		///< Set masks.
	/// @}

	/// @name Operators
	/// @{
	value_t swizzle( size_t swz_code ) const;
	value_t to_rvalue() const;
	/// @}

	static value_t slice( value_t const& vec, uint32_t masks );

protected:
	/// @name Constructor, Destructor, Copy constructor and assignment operator
	/// @{
	value_t(
		value_tyinfo* tyinfo,
		llvm::Value* val, value_kinds k, abis abi,
		cg_service* cg
		);
	value_t(
		builtin_types hint,
		llvm::Value* val, value_kinds k, abis abi,
		cg_service* cg
		);

	
	/// @}

	/// @name Members
	/// @{
	boost::scoped_ptr<value_t>	parent_; // For write mask and swizzle.
	uint32_t					masks_;

	value_tyinfo*				tyinfo_;
	builtin_types				hint_;
	value_kinds					kind_;
	abis						abi_;

	llvm::Value*				val_;
	llvm::Value*				bet_;	///<Branch execution tag, for SIMD.

	cg_service*					cg_;
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

struct insert_point_t{
	insert_point_t();
	EFLIB_OPERATOR_BOOL( insert_point_t ) { return block != NULL; }
	llvm::BasicBlock* block;
};

struct function_t{
	function_t();

	EFLIB_OPERATOR_BOOL( function_t ){ return NULL != fn; }

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
	/// Get Execution Mask.
	value_t packed_execution_mask() const;
	/// Return name
	void return_name( std::string const& s );
	/// Set Inline hint
	void inline_hint();
	void allocation_block( insert_point_t const& ip);
	insert_point_t allocation_block() const;

	boost::shared_ptr<value_tyinfo> get_return_ty() const;

	insert_point_t						alloc_block;
	std::vector<llvm::Argument*>		argCache;
	sasl::syntax_tree::function_type*	fnty;
	llvm::Function*						fn;
	bool								c_compatible;
	bool								external;
	bool								partial_execution;
	bool								ret_void;
	cg_service*							cg;
};

class cg_service
{
public:
	typedef boost::function< cgllvm_sctxt* (sasl::syntax_tree::node*, bool) > node_ctxt_fn;
	virtual bool initialize( llvm_module_impl* mod, node_ctxt_fn const& fn );

	/// @name Service States
	/// @{
	virtual abis intrinsic_abi() const = 0;
	/// @}

	/// @name Value Operators
	/// @{
	virtual llvm::Value* load( value_t const& );
	virtual llvm::Value* load( value_t const&, abis abi );
	virtual llvm::Value* load_ref( value_t const& );
	virtual llvm::Value* load_ref( value_t const& v, abis abi );
	virtual void store( value_t& lhs, value_t const& rhs ) = 0;
	/// @}

	/** @name Emit expressions
	Some simple overload-able operators such as '+' '-' '*' '/'
	will be implemented in 'cgv_*' classes in operator overload form.
	@{ */
	virtual value_t emit_add( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_sub( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_mul( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_div( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_mod( value_t const& lhs, value_t const& rhs );

	virtual value_t emit_cmp_lt( value_t const& lhs, value_t const& rhs ) = 0;
	virtual value_t emit_cmp_le( value_t const& lhs, value_t const& rhs ) = 0;
	virtual value_t emit_cmp_eq( value_t const& lhs, value_t const& rhs ) = 0;
	virtual value_t emit_cmp_ne( value_t const& lhs, value_t const& rhs ) = 0;
	virtual value_t emit_cmp_ge( value_t const& lhs, value_t const& rhs ) = 0;
	virtual value_t emit_cmp_gt( value_t const& lhs, value_t const& rhs ) = 0;

	virtual value_t emit_call( function_t const& fn, std::vector<value_t> const& args );
	virtual value_t emit_call( function_t const& fn, std::vector<value_t> const& args, value_t const& exec_mask );
	/// @}

	/// @name Emit element extraction
	/// @{
	virtual value_t emit_insert_val( value_t const& lhs, value_t const& idx, value_t const& elem_value );
	virtual value_t emit_insert_val( value_t const& lhs, int index, value_t const& elem_value );

	virtual value_t emit_extract_val( value_t const& lhs, int idx );
	virtual value_t emit_extract_val( value_t const& lhs, value_t const& idx );
	virtual value_t emit_extract_ref( value_t const& lhs, int idx );
	virtual value_t emit_extract_ref( value_t const& lhs, value_t const& idx );
	virtual value_t emit_extract_elem_mask( value_t const& vec, uint32_t mask );
	value_t emit_extract_col( value_t const& lhs, size_t index );

	template <typename IndexT>
	value_t emit_extract_elem( value_t const& vec, IndexT const& idx ){
		if( vec.storable() ){
			return emit_extract_ref( vec, idx );
		} else {
			return emit_extract_val( vec, idx );
		}
	}
	/// @}

	/// @name Intrinsics
	/// @{
	virtual value_t emit_dot( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_abs( value_t const& lhs );
	virtual value_t emit_exp( value_t const& lhs, function_t const& workaround_expf );
	virtual value_t emit_sqrt( value_t const& lhs );
	virtual value_t emit_cross( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_ddx( value_t const& v ) = 0;
	virtual value_t emit_ddy( value_t const& v ) = 0;
	/// @}

	/// @name Emit type casts
	/// @{
	/// Cast between integer types.
	virtual value_t cast_ints( value_t const& v, value_tyinfo* dest_tyi ) = 0;
	/// Cast integer to float.
	virtual value_t cast_i2f( value_t const& v, value_tyinfo* dest_tyi ) = 0;
	/// Cast float to integer.
	virtual value_t cast_f2i( value_t const& v, value_tyinfo* dest_tyi ) = 0;
	/// Cast between float types.
	virtual value_t cast_f2f( value_t const& v, value_tyinfo* dest_tyi ) = 0;
	/// Cast integer to bool
	virtual value_t cast_i2b( value_t const& v ) = 0;
	/// Cast float to bool
	virtual value_t cast_f2b( value_t const& v ) = 0;
	/// Cast scalar to vector
	virtual value_t cast_s2v( value_t const& v );
	/// Cast vector to scalar
	virtual value_t cast_v2s( value_t const& v );
	/// @}

	/// @name Emit statement
	/// @{
	virtual void emit_return() = 0;
	virtual void emit_return( value_t const&, abis abi ) = 0;
	/// @}

	/// @name Context switch
	/// @{
	virtual void function_beg(){}
	virtual void function_end(){}

	virtual void for_init_beg(){}
	virtual void for_init_end(){}
	virtual void for_cond_beg(){}
	virtual void for_cond_end( value_t const& ){}
	virtual void for_body_beg(){}
	virtual void for_body_end(){}
	virtual void for_iter_beg(){}
	virtual void for_iter_end(){}

	virtual value_t joinable(){ return value_t(); }

	virtual void if_beg(){}
	virtual void if_end(){}
	virtual void if_cond_beg(){}
	virtual void if_cond_end( value_t const& ){}
	virtual void then_beg(){}
	virtual void then_end(){}
	virtual void else_beg(){}
	virtual void else_end(){}

	virtual void switch_cond_beg(){}
	virtual void switch_cond_end(){}
	virtual void switch_expr_beg(){}
	virtual void switch_expr_end(){}

	virtual void while_beg(){}
	virtual void while_end(){}
	virtual void while_cond_beg(){}
	virtual void while_cond_end( value_t const& ){}
	virtual void while_body_beg(){}
	virtual void while_body_end(){}

	virtual void do_beg(){}
	virtual void do_end(){}
	virtual void do_body_beg(){}
	virtual void do_body_end(){}
	virtual void do_cond_beg(){}
	virtual void do_cond_end( value_t const& ){}

	virtual void break_(){}
	virtual void continue_(){}

	virtual void push_fn( function_t const& fn );
	virtual void pop_fn();
	virtual void set_insert_point( insert_point_t const& ip );
	virtual insert_point_t insert_point() const;
	/// @}

	/// @name Context queries
	/// @{
	bool in_function() const;
	function_t& fn();
	/// Get Packed Mask Which is a uint16_t.
	virtual value_t packed_mask() = 0;
	/// @}

	/// @name Emit value and variables
	/// @{
	function_t fetch_function( boost::shared_ptr<sasl::syntax_tree::function_type> const& fn_node );
	
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
	virtual value_t create_scalar( llvm::Value* val, value_tyinfo* tyinfo ) = 0;

	value_t null_value( value_tyinfo* tyinfo, abis abi );
	value_t null_value( builtin_types bt, abis abi );
	value_t undef_value( builtin_types bt, abis abi );

	value_t create_value( value_tyinfo* tyinfo, llvm::Value* val, value_kinds k, abis abi );
	value_t create_value( builtin_types hint, llvm::Value* val, value_kinds k, abis abi );
	value_t create_value( value_tyinfo* tyinfo, builtin_types hint, llvm::Value* val, value_kinds k, abis abi );

	value_t create_variable( value_tyinfo const*, abis abi, std::string const& name );
	value_t create_variable( builtin_types bt, abis abi, std::string const& name );

	virtual value_t create_vector( std::vector<value_t> const& scalars, abis abi ) = 0;
	/// @}

	/// @name Utilities
	/// @{
	/// Create a new block at the last of function
	insert_point_t new_block( std::string const& hint, bool set_insert_point );
	/// Jump to the specified block by condition.
	void jump_cond( value_t const& cond_v, insert_point_t const & true_ip, insert_point_t const& false_ip );
	/// Jump to
	void jump_to( insert_point_t const& ip );
	void clean_empty_blocks();
	/// @}

	/// @name Type emitters
	/// @{
	/// Create tyinfo.
	boost::shared_ptr<value_tyinfo> create_tyinfo( boost::shared_ptr<sasl::syntax_tree::tynode> const& tyn );
	/// Get member type information is type is aggregated.
	value_tyinfo* member_tyinfo( value_tyinfo const* agg, size_t index ) const;
	/// @}

	/// @name Bridges
	/// @{
	llvm::Type* type_( builtin_types bt, abis abi );
	llvm::Type* type_( value_tyinfo const*, abis abi );
	template <typename T>
	llvm::ConstantInt* int_( T v ){
		return llvm::ConstantInt::get( context(), apint(v) );
	}
	template <typename T>
	llvm::Constant* vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) )
	{
		assert( vals && length > 0 );

		std::vector<llvm::Constant*> elems(length);
		for( size_t i = 0; i < length; ++i ){
			elems[i] = int_(vals[i]);
		}

		return llvm::cast<llvm::Constant>( llvm::ConstantVector::get( elems ) );
	}

	template <typename U, typename T>
	llvm::Constant* vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) )
	{
		assert( vals && length > 0 );

		std::vector<llvm::Constant*> elems(length);
		for( size_t i = 0; i < length; ++i ){
			elems[i] = int_( U(vals[i]) );
		}

		return llvm::cast<llvm::Constant>( llvm::ConstantVector::get( elems ) );
	}

	llvm::Value* load_as( value_t const& v, abis abi );
	/// @}

	llvm::Module*			module () const;
	llvm::LLVMContext&		context() const;
	llvm::DefaultIRBuilder& builder() const;

	virtual bool			prefer_externals() const	= 0;
	virtual bool			prefer_scalar_code() const	= 0;

	virtual abis			param_abi( bool is_c_compatible ) const = 0;
			abis			promote_abi( abis abi0, abis abi1 );
			abis			promote_abi( abis abi0, abis abi1, abis abi2 );
	node_ctxt_fn			node_ctxt;

protected:
	std::vector<function_t> fn_ctxts;
	llvm_intrin_cache		intrins;
	llvm_module_impl*		mod_impl;
	value_t					exec_mask;
	
	value_t emit_add_ss_vv( value_t const& lhs, value_t const& rhs );
	value_t emit_sub_ss_vv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_ss_vv( value_t const& lhs, value_t const& rhs );
	value_t emit_div_ss_vv( value_t const& lhs, value_t const& rhs );
	value_t emit_mod_ss_vv( value_t const& lhs, value_t const& rhs );

	value_t emit_dot_vv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_sv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_sm( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_vm( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_mv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_mm( value_t const& lhs, value_t const& rhs );

	llvm::Value* sqrt_vf_( llvm::Value* v );
	llvm::Function* intrin_( int );
	template <typename FunctionT>
	llvm::Function* intrin_( int );
	llvm::Value* shrink_( llvm::Value* vec, size_t vsize );
	llvm::Value* extract_elements_( llvm::Value* src, size_t start_pos, size_t length );
	llvm::Value* insert_elements_ ( llvm::Value* dst, llvm::Value* src, size_t start_pos );

private:
	llvm::Value* load_as_llvm_c			( value_t const& v, abis abi );
	llvm::Value* load_c_as_package		( value_t const& v );
	llvm::Value* load_llvm_as_vec		( value_t const& v );
	llvm::Value* load_vec_as_llvm		( value_t const& v );
	llvm::Value* load_vec_as_package	( value_t const& v );
};

END_NS_SASL_CODE_GENERATOR();

#endif