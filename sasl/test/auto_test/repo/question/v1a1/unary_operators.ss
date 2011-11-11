
int test_pre_inc( int i ){
	int z = ++i;
	return z+(++i);
}

int test_pre_dec( int i ){
	int z = --i;
	return z+(--i);
}

int test_post_inc( int i ){
	int z = i++;
	return z+(i++);
}

int test_post_dec( int i ){
	int z = i--;
	return z+(i--);
}