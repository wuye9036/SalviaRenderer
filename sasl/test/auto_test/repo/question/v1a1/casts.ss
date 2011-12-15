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