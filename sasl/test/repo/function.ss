int foo(int a){
	return a;
}

int foo2(int a, int b){
	return a;
}

int cross_callee(int i)
{
	return i;
}

int cross_caller(int i)
{
	return cross_callee(i);
}

int fib( int i )
{
	if( i < 2 ){ return i; }
	return fib(i-1) + fib(i-2) ;
}