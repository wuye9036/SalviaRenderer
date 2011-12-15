/*
	Here we test condition expression, >, <, >=, <=, ||, && and short evaluation.
*/

int test_max( int i, int j ){
	return i > j ? i : j;
}

int test_min( int i, int j ){
	return i < j ? i : j;
}

bool test_le( int i, int j ){
	return i <= j;
}

bool test_ge( float i, float j ){
	return i >= j;
}

bool test_short( int i, int j, int k ){
	return ( i == 0 || j == 0 ) && k != 0;
}