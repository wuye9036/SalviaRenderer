float4 test_float_arith( float4 v ){
	return float4( v.x+v.y, v.y-v.z, v.z*v.w, v.w/v.x );
}

int3 test_int_arith( int3 v )
{
	return int3( v.x/v.y, v.y%v.z, v.z*v.x );
}

float3x4 test_mat_arith(float3x4 v0, float3x4 v1)
{
	return (v0+v1)*v0-(v1/v0)%v1;
}