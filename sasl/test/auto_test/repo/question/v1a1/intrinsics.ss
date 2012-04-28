float test_dot_f3( float3 lhs, float3 rhs ){
	return dot( lhs, rhs );
}

float4 test_mul_m44v4( float4x4 lhs, float4 rhs ){
	return mul( lhs, rhs );
}

struct f4{ float x,y,z,w; };
struct f44{ f4 x,y,z,w; };
f4 test_fetch_m44v4( f44 lhs ){
	return lhs.x;
}

float test_abs_f(float v)
{
	return abs(v);
}

int test_abs_i(int v)
{
	return abs(v);
}

float test_exp(float v)
{
	return exp(v);
}

float3x4 test_exp_m34( float3x4 v )
{
	return exp(v);
}

float test_sqrt_f( float v ){
	return sqrt(v);
}

float2 test_sqrt_f2(float2 v){
	return sqrt(v);
}

float4 test_sqrt_f4(float4 v){
	return sqrt(v);
}

float3 test_cross_prod( float3 v0, float3 v1 ){
	return cross(v0, v1);
}

float test_distance( float2 f, float3 f2, float3 f3 )
{
	return distance(f, f3.xy) + distance( dst(f3.xyzx, f2.yxzy), f.xyxy );
}

float4 test_fmod( float3 f, float f1, float3 f2 )
{
	return float4( fmod(f, f2), fmod(f1, f2.y) );
}

float3 test_lerp( float3 s, float3 d, float3 f )
{
	return lerp(s, d, f);
}

float3 test_rad_deg( float3 s )
{
	return radians(s.xy).xxy + degrees(s);
}

bool4 test_any_all(float3 f, int3 i)
{
	return bool4( any(f), all(f), any(i), all(i) );
}