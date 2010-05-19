void VSMain(float4 pos : POSITION,
		float2 uv : TEXCOORD0,
		out float2 oTex0 : TEXCOORD0,
		out float4 oPos : SV_Position)
{
	oPos = pos;
	oTex0 = uv;
}
