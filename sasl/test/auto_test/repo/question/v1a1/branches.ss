float test_if( int i ){
	if ( i == 0 ){
		return 1.0f;
	}
	return 0.0f;
}

int test_for( int base, int n ){
	int ret = base;
	for( int i = 0; i < n-1; ++i )
	{
		ret = ret * base;
	}
	return ret;
}