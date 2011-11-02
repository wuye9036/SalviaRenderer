float test_dot_f3( float3 lhs, float3 rhs ){
	return dot( lhs, rhs );
}

float4 test_mul_m44v4( float4x4 lhs, float4 rhs ){
	return mul( lhs, rhs );
}

// float4 test_mul_v4v4( float4 lhs, float4 rhs ){
// 	return lhs*rhs;
// }

struct f4{ float x,y,z,w; };
struct f44{ f4 x,y,z,w; };
f4 test_fetch_m44v4( f44 lhs ){
	return lhs.x;
}

//int test_dot_i2( int2 lhs, int2 rhs ){
//	return dot( lhs, rhs );
//}
