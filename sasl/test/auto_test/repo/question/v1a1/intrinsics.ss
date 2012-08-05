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

float3x4 test_exp2_m34(float3x4 v)	{ return exp2(v); }
float3x4 test_sin_m34(float3x4 v)	{ return sin(v); }
float3x4 test_cos_m34(float3x4 v)	{ return cos(v); }
float3x4 test_tan_m34(float3x4 v)	{ return tan(v); }
float3x4 test_sinh_m34(float3x4 v)	{ return sinh(v); }
float3x4 test_cosh_m34(float3x4 v)	{ return cosh(v); }
float3x4 test_tanh_m34(float3x4 v)	{ return tanh(v); }
float3x4 test_asin_m34(float3x4 v)	{ return asin(v); }
float3x4 test_acos_m34(float3x4 v)	{ return acos(v); }
float3x4 test_atan_m34(float3x4 v)	{ return atan(v); }
float3x4 test_ceil_m34(float3x4 v)	{ return ceil(v); }
float3x4 test_floor_m34( float3x4 v){ return floor(v); }
float3x4 test_round_m34( float3x4 v){ return round(v); }
float3x4 test_log_m34(float3x4 v)	{ return log(v); }
float3x4 test_log2_m34(float3x4 v)	{ return log2(v); }
float3x4 test_log10_m34(float3x4 v)	{ return log10(v); }
float3x4 test_rsqrt_m34(float3x4 v)	{ return rsqrt(v); }
float3x4 test_ldexp_m34(float3x4 v0, float3x4 v1){ return ldexp(v0, v1); }
float2 test_length(float2 v0, float4 v1) { return float2( length(v0), length(v1) ); }
int3 test_clamp_i3(int3 v0, int3 v1, int3 v2)  { return clamp(v0, v1, v2); }
int3 test_min_i3(int3 v0, int3 v1)  { return min(v0, v1); }
int3 test_max_i3(int3 v0, int3 v1)  { return max(v0, v1); }
float2x3 test_clamp_m23(float2x3 m0, float2x3 m1, float2x3 m2) { return clamp(m0, m1, m2); }
float2x3 test_min_m23(float2x3 m0, float2x3 m1) { return min(m0, m1); }
float2x3 test_max_m23(float2x3 m0, float2x3 m1) { return max(m0, m1); }
uint3 test_countbits_u3(uint3 v) { return countbits(v); }
uint3 test_count_bits_u3(uint3 v) { return count_bits(v); }
bool3x3 test_isinf_m33(float3x3 v) { return isinf(v); }
bool3x3 test_isfinite_m33(float3x3 v) { return isfinite(v); }
bool3x3 test_isnan_m33(float3x3 v) { return isnan(v); }
int3 test_firstbithigh_i3(int3 v) { return firstbithigh(v); }
uint2 test_firstbithigh_u2(uint2 v) { return firstbithigh(v); }
int3 test_firstbitlow_i3(int3 v) { return firstbitlow(v); }
uint2 test_firstbitlow_u2(uint2 v) { return firstbitlow(v); }
float3 test_frac_f3(float3 v){ return frac(v); }
float2x3 test_saturate_m23( float2x3 m ) { return saturate(m); }