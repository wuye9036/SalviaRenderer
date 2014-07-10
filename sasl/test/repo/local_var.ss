int get_sum( int c, int s )
{
	int total = 0;
	for( int i = 0; i < c; ++i )
	{
		int tmp = total + s;
		total = tmp;
	}
	return total;
}