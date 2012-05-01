int2x3 arith_assign(int2x3 v0, int2x3 v1)
{
	return (v0+=v1)-=(v0*=v1)/=v0%=v1;
}