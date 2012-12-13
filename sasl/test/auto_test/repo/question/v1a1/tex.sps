struct PSIN{
	float4	in0: TEXCOORD(0);
};

struct PSOUT{
	float4	out0: COLOR(0);
};

sampler s;

PSOUT fn( PSIN in ){
	PSOUT o;
	
	o.out0 = tex2Dlod(s, in.in0);

	return o;
}