struct VSIN{
	float4	in0: TEXCOORD(0);
};

struct VSOUT{
	float4	out0: COLOR(0);
};

sampler s;

VSOUT fn( VSIN in ){
	VSOUT o;
	
	o.out0 = tex2Dlod(s, in.in0);

	return o;
}