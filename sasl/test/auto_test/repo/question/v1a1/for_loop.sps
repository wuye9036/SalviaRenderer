struct VSIN{
	float in0: TEXCOORD(0);
};

struct VSOUT{
	float out: COLOR;
};

VSOUT fn( VSIN in ){
	VSOUT o;
	float x = in.in0;
	for( int i = 0; i < 10; i = i + 1 )
	{
		x = x * 2.0f;
		if ( x > 5000.0f ){ break; }
	}
	o.out = x;
	return o;
}