struct VSIN{
	float  in0: TEXCOORD(0);
};

struct VSOUT{
	float  out0: COLOR(0);
};

sampler s;

VSOUT fn( VSIN in ){
	VSOUT o;
	
	o.out0 = in.in0;

	return o;
}