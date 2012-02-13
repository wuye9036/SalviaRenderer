#ifndef SASL_CODE_GENERATOR_CGLLVM_CGS_SISD_H
#define SASL_CODE_GENERATOR_CGLLVM_CGS_SISD_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgllvm_intrins.h>
#include <sasl/include/code_generator/llvm/cgllvm_service.h>
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

class cgs_sisd: public cg_service{
public:
	abis intrinsic_abi() const;

	/** @name Emit expressions
	Some simple overloadable operators such as '+' '-' '*' '/'
	will be implemented in 'cgv_*' classes in operator overload form.
	@{ */
	value_t emit_cond_expr( value_t cond, value_t const& yes, value_t const& no );
	
	value_t emit_cmp_lt( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_le( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_eq( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_ne( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_ge( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_gt( value_t const& lhs, value_t const& rhs );

	/// Didn't support swizzle yet.
	value_t emit_swizzle( value_t const& vec, uint32_t mask );
	value_t emit_write_mask( value_t const& vec, uint32_t mask );
	/** @} */

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
	/// Cast integer to bool
	value_t cast_i2b( value_t const& v );
	/// Cast float to bool
	value_t cast_f2b( value_t const& v );
	/// @}

	/// @name Emit Declarations
	/// @{
	function_t begin_fndecl();
	function_t end_fndecl();
	/// @}

	/// @name Intrinsics
	/// @{
	virtual value_t emit_ddx( value_t const& v );
	virtual value_t emit_ddy( value_t const& v );
	/// @}

	/// @name Emit statement
	/// @{
	void emit_return();
	void emit_return( value_t const&, abis abi );
	/// @}

	/// @name Emit assignment
	/// @{
	virtual void store( value_t& lhs, value_t const& rhs );
	/// @}

	/// @name Emit values
	/// @{
	value_t undef_value( builtin_types bt, abis abi );

	template <typename T>
	value_t create_constant_vector( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	
	value_t create_scalar( llvm::Value* val, value_tyinfo* tyinfo );
	value_t create_vector( std::vector<value_t> const& scalars, abis abi );

	template <typename T>
	value_t create_constant_matrix( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	/// @}

	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( size_t tyid ) = 0;
	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( builtin_types bt ) = 0;

	/// @name Utilities
	/// @{
	/// Jump to the specified block by condition.
	void jump_cond( value_t const& cond_v, insert_point_t const & true_ip, insert_point_t const& false_ip );
	/// Switch to blocks
	void switch_to( value_t const& cond, std::vector< std::pair<value_t, insert_point_t> > const& cases, insert_point_t const& default_branch );
	/// Clean empty blocks of current function.
	cgllvm_sctxt* node_ctxt( boost::shared_ptr<sasl::syntax_tree::node> const& node, bool create_if_need );
	/// @}

	/// @name Bridges
	/// @{
	llvm::Value* select_( llvm::Value* cond, llvm::Value* yes, llvm::Value* no );
	template <typename T>
	llvm::Value* c_vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	/// @}

	/// @name State
	/// @{
	abis param_abi( bool c_compatible ) const;
	/// Prefer to use external functions as intrinsic.
	bool prefer_externals() const;
	/// Prefer to use scalar code to intrinsic.
	bool prefer_scalar_code() const;
	/// @}

};

END_NS_SASL_CODE_GENERATOR();

#endif