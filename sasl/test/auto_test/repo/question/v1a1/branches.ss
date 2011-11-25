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

int test_while( int base, int n ){
	int ret = base;
	int i = 1;
	while ( i++ < n ){
		ret = ret*base;
	}
	return ret;
}

int test_dowhile( int base, int n ){
	int ret = 1;
	int i = 0;

	do{
		ret = ret * base;
	} while( ++i < n );

	return ret;
}