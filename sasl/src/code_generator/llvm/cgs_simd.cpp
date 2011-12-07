#include <sasl/include/code_generator/llvm/cgllvm_simd.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Type.h>
#include <eflib/include/platform/enable_warnings.h>

using llvm::Type;
using std::vector;

int const SIMD_WIDTH_IN_BYTES = 16;
int const PACKAGE_SIZE = 16;

BEGIN_NS_SASL_CODE_GENERATOR();

llvm::Value* cgs_simd::load( value_t const& )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

llvm::Value* cgs_simd::load( value_t const& , abis abi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

llvm::Value* cgs_simd::load_ref( value_t const& )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

void cgs_simd::store( value_t& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

value_t cgs_simd::cast_ints( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_i2f( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_f2i( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_f2f( value_t const& v, value_tyinfo* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_i2b( value_t const& v )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::cast_f2b( value_t const& v )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::create_vector( vector<value_t> const& scalars, abis abi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

abis cgs_simd::intrinsic_abi() const
{
	return abi_vectorize;
}

void cgs_simd::push_fn( function_t const& fn )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void cgs_simd::pop_fn()
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void cgs_simd::set_insert_point( insert_point_t const& ip )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

insert_point_t cgs_simd::insert_point() const
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return insert_point_t();
}

value_t cgs_simd::emit_add( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_sub( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_mul( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

void cgs_simd::emit_return()
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void cgs_simd::emit_return( value_t const&, abis abi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

value_t cgs_simd::emit_dot( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_sqrt( value_t const& lhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

value_t cgs_simd::emit_cross( value_t const& lhs, value_t const& rhs )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

abis cgs_simd::param_abi( bool /*c_compatible*/ ) const
{
	return abi_package;
}

END_NS_SASL_CODE_GENERATOR();
