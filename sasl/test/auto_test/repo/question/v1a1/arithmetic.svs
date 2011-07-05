struct VSIN{
	float pos: SV_Position;
};

struct VSOUT{
	float pos: SV_Position;
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