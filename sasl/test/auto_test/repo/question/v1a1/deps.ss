int g;

int param_deps( int a ){
	return a;
}

int rv_deps( int a ){
	return a+a;
}

int global_deps( int a )
{
	return a + g;
}

int local_deps( int a )
{
	int b = a;
	return b;
}

int constant_deps()
{
	int b = 0;
	return b;
}