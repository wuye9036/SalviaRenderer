#ifndef SASL_CODE_GENERATOR_CGLLVM_CGS_SISD_H
#define SASL_CODE_GENERATOR_CGLLVM_CGS_SISD_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgllvm_intrins.h>
#include <sasl/include/code_generator/llvm/cgs.h>
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
		struct function_type;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_sctxt;
class llvm_intrin_cache;

class cgs_sisd: public cg_service{
public:
	abis intrinsic_abi() const;

	/** @name Emit expressions
	Some simple overload-able operators such as '+' '-' '*' '/'
	will be implemented in 'cgv_*' classes in operator overload form.
	@{ */
	cg_value emit_cond_expr( cg_value cond, cg_value const& yes, cg_value const& no );

	/// Didn't support swizzle yet.
	cg_value emit_swizzle( cg_value const& vec, uint32_t mask );
	cg_value emit_write_mask( cg_value const& vec, uint32_t mask );
	/** @} */

	/// @name Emit type casts
	/// @{
	/// Cast between integer types.
	cg_value cast_ints( cg_value const& v, cg_type* dest_tyi );
	/// Cast integer to float.
	cg_value cast_i2f( cg_value const& v, cg_type* dest_tyi );
	/// Cast float to integer.
	cg_value cast_f2i( cg_value const& v, cg_type* dest_tyi );
	/// Cast between float types.
	cg_value cast_f2f( cg_value const& v, cg_type* dest_tyi );
	/// Cast integer to bool
	cg_value cast_i2b( cg_value const& v );
	/// Cast float to bool
	cg_value cast_f2b( cg_value const& v );
	/// @}

	/// @name Emit Declarations
	/// @{
	function_t begin_fndecl();
	function_t end_fndecl();
	/// @}

	/// @name Intrinsics
	/// @{
	virtual cg_value emit_ddx( cg_value const& v );
	virtual cg_value emit_ddy( cg_value const& v );
	/// @}

	/// @name Emit statement
	/// @{
	void emit_return();
	void emit_return( cg_value const&, abis abi );
	/// @}

	/// @name Emit assignment
	/// @{
	virtual void store( cg_value& lhs, cg_value const& rhs );
	/// @}

	/// @name Emit values
	/// @{
	template <typename T>
	cg_value create_constant_vector( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	
	cg_value create_scalar( llvm::Value* val, cg_type* tyinfo, builtin_types hint );
	cg_value create_vector( std::vector<cg_value> const& scalars, abis abi );

	template <typename T>
	cg_value create_constant_matrix( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	/// @}

	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( size_t tyid ) = 0;
	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( builtin_types bt ) = 0;

	/// @name Utilities
	/// @{
	/// Switch to blocks
	void switch_to( cg_value const& cond, std::vector< std::pair<cg_value, insert_point_t> > const& cases, insert_point_t const& default_branch );
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
	cg_value packed_mask();

	abis param_abi( bool c_compatible ) const;
	/// Prefer to use external functions as intrinsic.
	bool prefer_externals() const;
	/// Prefer to use scalar code to intrinsic.
	bool prefer_scalar_code() const;
	/// @}

};

END_NS_SASL_CODE_GENERATOR();

#endif