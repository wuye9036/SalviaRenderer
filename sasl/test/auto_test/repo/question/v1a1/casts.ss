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