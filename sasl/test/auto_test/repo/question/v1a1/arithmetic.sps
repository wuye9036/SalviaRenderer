struct VSIN{
	float pos: TEXCOORD(0);
};

struct VSOUT{
	float pos: COLOR;
};

VSOUT fn( VSIN in ){
	VSOUT o;
	
	float x, y;
	x = in.pos;
	y = 5.0f;
	
	o.pos = x + y;
	o.pos = o.pos + (x - y);
	o.pos = o.pos + (x * y);
	o.pos = o.pos + (x / y);
	
	return o;
}