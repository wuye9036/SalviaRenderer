struct PSIN{
	float in0: TEXCOORD(0);
};

struct PSOUT{
	float out: COLOR;
};

PSOUT fn( PSIN in ){
	PSOUT o;
	float x = in.in0;
	for( int i = 0; i < 10; i = i + 1 )
	{
		x = x * 2.0f;
		if ( x > 5000.0f ){ break; }
	}
	o.out = x;
	return o;
}