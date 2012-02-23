struct VSIN{
	float in0: TEXCOORD(0);
};

struct VSOUT{
	float out: COLOR;
};

VSOUT fn( VSIN in ){
	VSOUT o;
	float x = in.in0;

	while( x < 3000.0f )
	{
		if( x < 1.0f ) {
			break;
		}
		x = x * x;
	}

	o.out = x;

	return o;
}