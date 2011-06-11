Texture2D sa_tex : register(t0);
SamplerState point_sampler : register(s0);

float4 PSMain(float2 tex0 : TEXCOORD0) : SV_Target
{
	return sa_tex.Sample(point_sampler, tex0);
}
