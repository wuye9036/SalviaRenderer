#ifndef SASL_CODEGEN_CGS_OBJECTS_H
#define SASL_CODEGEN_CGS_OBJECTS_H

#include <sasl/include/codegen/forward.h>

#include <sasl/enums/builtin_types.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/utility/operator_bool.h>

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
	template <typename T> class ArrayRef;
}

namespace sasl
{
	namespace syntax_tree
	{
		struct node;
		struct tynode;
		struct function_type;
	}
	namespace semantic
	{
		class module_semantic;
	}
}

BEGIN_NS_SASL_CODEGEN();

class module_context;

namespace abis
{
	enum id{
		unknown,
		c,
		llvm,
		vectorize,
		package,
		count
	};
}

namespace value_kinds
{
	enum id
	{
		unknown = 0,
		ty_only = 1,
		elements = 2,

		value = 4,
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
		/// And 'kind' the parameter/argument 'arg' is set to 'value_kinds::id::reference'.
		reference = 8
	};
};

class cg_type{
public:
	friend class cg_service;

	cg_type(
		sasl::syntax_tree::tynode* sty,
		llvm::Type* ty_c,
		llvm::Type* ty_llvm,
		llvm::Type* ty_vec,
		llvm::Type* ty_pkg
		);
	
	cg_type();

	cg_type( cg_type const& );
	cg_type& operator = ( cg_type const& );

	sasl::syntax_tree::tynode*
					tyn_ptr() const;
	builtin_types	hint() const;
	llvm::Type*		ty( abis::id abi ) const;

protected:
	llvm::Type*					tys[abis::count];
	sasl::syntax_tree::tynode*	tyn;
};

class cg_value{
public:
	friend class cg_service;

	cg_value();
	cg_value( cg_value const& );
	cg_value& operator = ( cg_value const& );

	/// @name State accessors 
	/// @{
	/// Get service.
	cg_service* service() const;
	/// Return internal llvm value.
	llvm::Value* raw() const;

	/// Load llvm value from cg_value.
	llvm::Value* load() const;
	llvm::Value* load( abis::id abi ) const;
	llvm::Value* load_i1() const;
	llvm::Value* load_ref() const;

	void store( cg_value const& ) const;

	/// Store llvm value to cg_value
	void emplace( cg_value const& );
	void emplace( llvm::Value* v, value_kinds::id k, abis::id abi );

	bool storable() const;
	bool load_only() const;

	cg_value as_ref() const;

	cg_type*		ty() const;				///< Get type information of value.
	void			ty(cg_type*);		///< Set type information of value.

	builtin_types	hint() const;				///< Get type hint. if type is not built-in type it returns builtin_type::none.
	void			hint( builtin_types bt );	///< Set type hint.

	value_kinds::id		kind() const;				///< Get kind.
	void			kind( value_kinds::id vkind );	///< Set kind.

	cg_value*		parent() const;				///< Get parent. If value is not a member of aggragation, it return NULL.
	void			parent( cg_value const& v );
	void			parent( cg_value const* v );

	abis::id			abi() const;				///< Get ABI.
	void			abi( abis::id abi );			///< Set ABI

	cg_value*		index() const;
	void			index( cg_value const& );
	void			index( cg_value const* );
	void			index( size_t v );			///< Set Index. It is only make sense if parent is available.
	uint32_t		masks() const;				///< Get masks
	void			masks( uint32_t v );		///< Set masks.
	/// @}

	/// @name Operators
	/// @{
	cg_value swizzle( size_t swz_code ) const;
	cg_value to_rvalue() const;
	/// @}

	static cg_value slice( cg_value const& vec, uint32_t masks );
	static cg_value slice( cg_value const& vec, cg_value const& index );

protected:
	/// @name Constructor, Destructor, Copy constructor and assignment operator
	/// @{
	cg_value(cg_type* ty, llvm::Value* val, value_kinds::id k, abis::id abi, cg_service* cg);
	cg_value(builtin_types hint, llvm::Value* val, value_kinds::id k, abis::id abi, cg_service* cg);
	/// @}

	/// @name Members
	/// @{
	boost::scoped_ptr<cg_value>	parent_; // For write mask and swizzle.
	boost::scoped_ptr<cg_value>	index_;
	uint32_t					masks_;

	cg_type*					ty_;
	builtin_types				builtin_ty_;
	value_kinds::id				kind_;
	abis::id					abi_;

	llvm::Value*				val_;

	cg_service*					cg_;
	/// @}
};

template <typename RVT>
struct cg_scope_guard{
	typedef boost::function<RVT ()> on_exit_fn;
	cg_scope_guard( on_exit_fn do_exit ): do_exit(do_exit){}
	~cg_scope_guard(){ do_exit(); }
private:
	on_exit_fn do_exit;
};

struct insert_point_t{
	insert_point_t();
	EFLIB_OPERATOR_BOOL( insert_point_t ) { return block != NULL; }
	llvm::BasicBlock* block;
};

struct cg_function{
	cg_function();

	EFLIB_OPERATOR_BOOL( cg_function ){ return NULL != fn; }

	/// Get argument's value by index.
	cg_value arg( size_t index ) const;
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
	abis::id abi() const;
	/// Get return address value.
	llvm::Value* return_address() const;
	/// Get Execution Mask.
	cg_value packed_execution_mask() const;
	/// Return name
	void return_name( std::string const& s );
	/// Set Inline hint
	void inline_hint();
	void allocation_block( insert_point_t const& ip);
	insert_point_t allocation_block() const;

	cg_type* get_return_ty() const;

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

END_NS_SASL_CODEGEN();

#endif