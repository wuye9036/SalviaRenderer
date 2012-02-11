struct VSIN{
	float in0: TEXCOORD(0);
	float3 in1: TEXCOORD(1);
};

struct VSOUT{
	float2 out: COLOR;
};

VSOUT fn( VSIN in ){
	VSOUT o;

	o.out.x = 88.3f;
	o.out.y = 75.4f;

	// Test if
	if( in.in0 > 0.0f ){
		o.out.x = in.in0;
	}
	
	// Test if-else
	if( in.in0 > 1.0f ){
		o.out.y = in.in1.x;
	}  else {
		o.out.y = in.in1.y;
	}

	// Test nested if-else
	if( in.in0 > 2.0f ){
		o.out.y = in.in1.z;
		if ( in.in0 > 3.0f ){
			o.out.y = o.out.y + 1.0f;
		} else if( in.in0 > 2.5f ){
			o.out.y = o.out.y + 2.0f;
		}
	}
	return o;
}