
int test_pre_inc( int x ){
	// Now argument is a right-value. So create variable to store i.
	int i = x;
	int z = ++i;
	return z+(++i);
}

int test_pre_dec( int x ){
	// Now argument is a right-value. So create variable to store i.
	int i = x;
	int z = --i;
	return z+(--i);
}

int test_post_inc( int x ){
	// Now argument is a right-value. So create variable to store i.
	int i = x;
	int z = i++;
	return z+(i++);
}

int test_post_dec( int x ){
	// Now argument is a right-value. So create variable to store i.
	int i = x;
	int z = i--;
	return z+(i--);
}

int4 test_neg_i( int3 x, int y )
{
	return int4(-x, -y);
}

float3x4 test_neg_f ( float3x4 x )
{
	return -x;
}

bool2x3 test_not( bool2x3 x )
{
	return !x;
}

uint2x3 test_bit_not( uint2x3 x )
{
	return ~x;
}
