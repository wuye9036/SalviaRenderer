// Test Items
//	mul( float3x3, float3 );
//	mul( float3, float3x3 );
//	dot( float3, float3 );
//	sqrt( float3 );

struct PSIN{
	float3 in0: Texcoord(0);
	float3 in1: Texcoord(1);
};

struct PSOUT{
	float4 out0: COLOR(0);
	float2 out1: COLOR(1);
};

VSOUT fn( VSIN in ){
	VSOUT o;
	float3x3 f33 = cross(in0, in1);
	float3 f3 = mul( in0.yzy, f33 );
	float3 f3_2 = mul( f33, in0.yzy );
	float x = dot(f3, in1);
	o.out0 = sqrt(f3);
	o.out1.x = x;
	o.out1.y = sqrt(f3_2.x);
	return o;
}