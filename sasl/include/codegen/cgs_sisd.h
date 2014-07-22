#ifndef SASL_CODEGEN_CGS_SISD_H
#define SASL_CODEGEN_CGS_SISD_H

#include <sasl/include/codegen/forward.h>

#include <sasl/include/codegen/cg_intrins.h>
#include <sasl/include/codegen/cgs.h>
#include <sasl/enums/builtin_types.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/for.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

//#include <eflib/include/utility/util.h>
#include <eflib/include/utility/enable_if.h>
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
		struct function_full_def;
	}
}

BEGIN_NS_SASL_CODEGEN();

class llvm_intrin_cache;

class cgs_sisd: public cg_service{
public:
	cgs_sisd();

	/** @name Emit expressions
	Some simple overload-able operators such as '+' '-' '*' '/'
	will be implemented in 'cgv_*' classes in operator overload form.
	@{ */
	multi_value emit_cond_expr( multi_value cond, multi_value const& yes, multi_value const& no );

	/// Didn't support swizzle yet.
	multi_value emit_swizzle( multi_value const& vec, uint32_t mask );
	multi_value emit_write_mask( multi_value const& vec, uint32_t mask );
	/** @} */

	/// @name Emit type casts
	/// @{
	/// Cast between integer types.
	multi_value cast_ints( multi_value const& v, cg_type* dest_tyi );
	/// Cast integer to float.
	multi_value cast_i2f( multi_value const& v, cg_type* dest_tyi );
	/// Cast float to integer.
	multi_value cast_f2i( multi_value const& v, cg_type* dest_tyi );
	/// Cast between float types.
	multi_value cast_f2f( multi_value const& v, cg_type* dest_tyi );
	/// Cast integer to bool
	multi_value cast_i2b( multi_value const& v );
	/// Cast float to bool
	multi_value cast_f2b( multi_value const& v );
	/// @}

	/// @name Emit Declarations
	/// @{
	cg_function begin_fndecl();
	cg_function end_fndecl();
	/// @}

	/// @name Intrinsics
	/// @{
	virtual multi_value emit_ddx( multi_value const& v );
	virtual multi_value emit_ddy( multi_value const& v );
	/// @}

	/// @name Emit statement
	/// @{
	void emit_return();
	void emit_return( multi_value const&, abis abi );
	/// @}

	/// @name Emit assignment
	/// @{
	virtual void store( multi_value& lhs, multi_value const& rhs );
	/// @}

	/// @name Emit values
	/// @{
	template <typename T>
	multi_value create_constant_vector( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	multi_value create_vector( std::vector<multi_value> const& scalars, abis abi );

	template <typename T>
	multi_value create_constant_matrix( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	/// @}

	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( size_t tyid ) = 0;
	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( builtin_types bt ) = 0;

	/// @name Utilities
	/// @{
	/// Switch to blocks
	void switch_to( multi_value const& cond, std::vector< std::pair<multi_value, insert_point_t> > const& cases, insert_point_t const& default_branch );
	/// @}

	/// @name Bridges
	/// @{
	llvm::Value* select_( llvm::Value* cond, llvm::Value* yes, llvm::Value* no );
	llvm::Value* phi_( llvm::BasicBlock* b0, llvm::Value* v0, llvm::BasicBlock* b1, llvm::Value* v1 );
	template <typename T>
	llvm::Value* c_vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	/// @}

	/// @name State
	/// @{
	/// Prefer to use external functions as intrinsic.
	bool prefer_externals() const;
	/// Prefer to use scalar code to intrinsic.
	bool prefer_scalar_code() const;
	/// @}

	llvm::Value* current_execution_mask() const { return NULL; }
};

END_NS_SASL_CODEGEN();

#endif