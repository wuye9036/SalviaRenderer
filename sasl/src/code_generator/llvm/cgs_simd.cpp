#include <sasl/include/code_generator/llvm/cgllvm_simd.h>

using std::vector;

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

END_NS_SASL_CODE_GENERATOR();
