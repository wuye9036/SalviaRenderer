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

bool3 test_vbool( int3 i, int3 j, int3 k )
{
	return i > j || i > k && i <= j+k;  
}

bool3x4 test_mbool( float3x4 i, float3x4 j, float3x4 k )
{
	return i > j || i > k && i <= j+k;  
}