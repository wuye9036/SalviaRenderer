uint test_bitwise_ops( uint4 a, uint b )
{
	return  ( (a.x<<a.y) + (a.y<<3u) - (a.y>>2u) ) & (a.z>>a.w) | b;
}