struct PSIN{
	float in0: TEXCOORD(0);
};

struct PSOUT{
	float out: COLOR;
};

PSOUT fn( PSIN in ){
	PSOUT o;
	float x = in.in0;
	
	do {
		if( x < 1.0f ) {
			break;
		}
		x = x * x;
	} while( x < 3000.0f );

	o.out = x;
	return o;
}