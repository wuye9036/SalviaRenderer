void VSMain(float2 pos : POSITION,
		out float2 oTex0 : TEXCOORD0,
		out float4 oPos : SV_Position)
{
	oPos = float4(pos, 0, 1);
	oTex0 = float2(float2(pos.x, -pos.y) * 0.5f + 0.5f);
}
