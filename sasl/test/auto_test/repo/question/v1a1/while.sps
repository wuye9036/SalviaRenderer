struct PSIN{
	float in0: TEXCOORD(0);
};

struct PSOUT{
	float out: COLOR;
};

PSOUT fn( PSIN in ){
	PSOUT o;
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