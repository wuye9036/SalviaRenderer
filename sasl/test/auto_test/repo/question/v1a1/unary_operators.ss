
int test_pre_inc( int x ){
	// Now argument is a right-value. So create variable to store i.
	int i;
	i = x;
	int z;
	z = ++i;
	return z+(++i);
}

int test_pre_dec( int x ){
	// Now argument is a right-value. So create variable to store i.
	int i;
	i = x;
	int z;
	z = --i;
	return z+(--i);
}

int test_post_inc( int x ){
	// Now argument is a right-value. So create variable to store i.
	int i;
	i = x;
	int z;
	z = i++;
	return z+(i++);
}

int test_post_dec( int x ){
	// Now argument is a right-value. So create variable to store i.
	int i;
	i = x;
	int z;
	z = i--;
	return z+(i--);
}