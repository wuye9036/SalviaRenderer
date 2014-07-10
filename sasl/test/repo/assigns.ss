int2x3 test_arith_assign(int2x3 v0, int2x3 v1)
{
	int2x3 v = (v0%=v1);
	int2x3 vv= (v1*=v0);
	return (v0+=v1)-=(vv/=v);
}

int test_scalar_arith_assign(int arg0, int arg1)
{
	int v0 = arg0;
	int v1 = arg1;
	int v = (v0%=v1);
	int vv = (v1*=v0);
	return (v0+=v1)-=(vv/=v);
}

int2x3 test_bit_assign(int2x3 v0, int2x3 v1)
{
	int2x3 v = (v0&=v1);
	int2x3 vv= (v1|=v0);
	return v0^=(vv-v);
}