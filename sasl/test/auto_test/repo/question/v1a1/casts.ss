
int test_implicit_cast_i32_b( int i ){
	if( i ){
		return 33;
	} else {
		return 85;
	}
}

int test_implicit_cast_f32_b( float f ){
	if( f ){
		return 33;
	} else {
		return 85;
	}
}

float test_implicit_cast_i32_f32( int i ){
	float tmpf = i;
	return tmpf;
}

float test_op_add_cast( int i, float j )
{
	return i+j;
}

int test_op_sub_cast( uint8_t i, int j )
{
	return i-j;
}

float test_sqrt_cast( int j )
{
	return sqrt(j);
}

float test_imp_v1_s_cast(int2 x)
{
	return x.x;
}

int2 test_bitcast_to_i( float2 f, uint2 u )
{
	return asint(f) + asint(u);
}

uint3 test_bitcast_to_u( float3 f, int3 i )
{
	return asuint(f) + asuint(i);
}

float test_bitcast_to_f( uint u, int i )
{
	return asfloat(u) + asfloat(i);
}

int2x3 test_bitcast_to_mi( float2x3 f, uint2x3 u )
{
	return asint(f) + asint(u);
}
