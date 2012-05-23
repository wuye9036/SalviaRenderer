#ifndef SASL_CODE_GENERATOR_LLVM_CGS_OBJECTS_H
#define SASL_CODE_GENERATOR_LLVM_CGS_OBJECTS_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/enums/builtin_types.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/metaprog/util.h>

#include <vector>

namespace llvm
{
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
	class AllocaInst;
	class Function;
	class APInt;

	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
	> class IRBuilder;
	class ConstantFolder;
	typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
		DefaultIRBuilder;
}

namespace sasl
{
	namespace syntax_tree
	{
		struct node;
		struct tynode;
		struct function_type;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

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

	/// @name State accessors 
	/// @{
	/// Get service.
	cg_service* service() const;
	/// Return internal llvm value.
	llvm::Value* raw() const;

	/// Load llvm value from value_t.
	llvm::Value* load() const;
	llvm::Value* load( abis abi ) const;
	llvm::Value* load_i1() const;
	llvm::Value* load_ref() const;

	void store( value_t const& ) const;

	/// Store llvm value to value_t
	void emplace( value_t const& );
	void emplace( llvm::Value* v, value_kinds k, abis abi );

	bool storable() const;
	bool load_only() const;

	value_t as_ref() const;

	value_tyinfo*	tyinfo() const;				///< Get type information of value.
	void			tyinfo(value_tyinfo*);		///< Set type information of value.

	builtin_types	hint() const;				///< Get type hint. if type is not built-in type it returns builtin_type::none.
	void			hint( builtin_types bt );	///< Set type hint.

	value_kinds		kind() const;				///< Get kind.
	void			kind( value_kinds vkind );	///< Set kind.

	value_t*		parent() const;				///< Get parent. If value is not a member of aggragation, it return NULL.
	void			parent( value_t const& v );
	void			parent( value_t const* v );

	abis			abi() const;				///< Get ABI.
	void			abi( abis abi );			///< Set ABI

	value_t*		index() const;
	void			index( value_t const& );
	void			index( value_t const* );
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
	static value_t slice( value_t const& vec, value_t const& index );
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
	boost::scoped_ptr<value_t>	index_;
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

END_NS_SASL_CODE_GENERATOR();

#endif