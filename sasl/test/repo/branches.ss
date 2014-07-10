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

int test_switch( int base, int n ){
	int ret = 0;
	switch ( n ){
	case 1:
		return base;			// Return
	case 2:
		return base*base;
	case 3:						// Walk through
	case 4:
		return base*base*base;
	case 0:						// Unordered
		return 1;
	case 5:						// Break
		ret = 8876;
		break;
	default:					// Default
		return 0;
	}
	return ret;
}