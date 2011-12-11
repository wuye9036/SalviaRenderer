struct In{
	float		tex0: TEXCOORD(0);
	float2		tex1: TEXCOORD(1);
	float3		tex2: TEXCOORD(2);
	float4		tex3: TEXCOORD(3);
	float2x3	tex4: TEXCOORD(4);
	int3		tex5: TEXCOORD(5);
};

float ps_main( In in ): COLOR
{
	return in.tex0;
}