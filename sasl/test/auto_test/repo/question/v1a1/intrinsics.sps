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
	float3 out0: COLOR(0);
	float2 out1: COLOR(1);
};

PSOUT fn( PSIN in ){
	PSOUT o;
	float3 f3 = cross(in.in0, in.in1);
	float x = dot(f3, in.in1);
	o.out0 = sqrt(f3);
	o.out1.x = x;
	o.out1.y = sqrt(f3.x);
	return o;
}