float4 test_mat_index( float3x4 v )
{
	return v[0] + v[1];
}

float test_vec_index( float3x4 v )
{
	return v[2][1] + v[1][2];
}