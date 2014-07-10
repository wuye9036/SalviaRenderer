struct PSIN{
	float4 pos: Texcoord(0);
};

struct PSOUT{
	float3 pos: SV_Target;
};

PSOUT fn( PSIN in ){
	PSOUT o;
	
	float3 x, y;

	// Test swizzle and mask.
	x = (in.pos).xyz;
	y = (in.pos).wxy;
	
	o.pos = x + y;
	o.pos = o.pos + (x - y);
	o.pos = o.pos + (x * y);
	// o.pos = o.pos + (x / y);
	
	return o;
}