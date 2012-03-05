int2 get_int2()
{
	return int2(95, 86);
}

int3 add_int3(int3 v)
{
	return v + int3(22, int2(95, 86) );
}

float3 get_float3()
{
	return float3(0.87f, 7.89f, 98.76f);
}

float4 add_float4( float4 v )
{
	return float4(1.22f, float2(0.93f, 187.22f), 5.56f) + v;
}
