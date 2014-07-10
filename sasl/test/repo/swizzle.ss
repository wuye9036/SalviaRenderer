int3 fn( int4 x, int2 y )
{
	int3 ret;
	ret.xy = x.zw + y.yx;
	ret.zy.x = x.zxw.z + y.yxy.x;
	return ret.yzxx.yxw;
}