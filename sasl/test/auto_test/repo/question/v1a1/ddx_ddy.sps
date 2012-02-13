struct VSIN{
	float  in0: TEXCOORD(0);
	float2 in1: TEXCOORD(1);
	float3 in2: TEXCOORD(2);
	float4 in3: TEXCOORD(3);
};

struct VSOUT{
	float  out0: COLOR(0);
	float2 out1: COLOR(1);
	float3 out2: COLOR(2);
	float4 out3: COLOR(3);
};

VSOUT fn( VSIN in ){
	VSOUT o;

	o.out0 = ddx(in.in0) + ddy(in.in0);
	o.out1 = ddx(in.in1).xy + ddy(in.in1).yx;
	o.out2 = ddx(in.in2).xyz + ddy(in.in2).yzx;
	o.out3 = ddx(in.in3).xwzy + ddy(in.in3).yzxw;

	return o;
}