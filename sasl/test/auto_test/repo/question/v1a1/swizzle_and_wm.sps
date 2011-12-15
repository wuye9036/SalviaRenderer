struct PSIN{
	float4 f4: Texcoord(0);
};

struct PSOUT{
	float3 f3: SV_Target;
};

PSOUT fn( PSIN in ){
	PSOUT o;
	
	float3 x, y;

	// Test swizzle and mask.
	x = (in.f4).xyz;
	y = (in.f4).wxy;
	
	o.f3 = x + y;
	
	return o;
}