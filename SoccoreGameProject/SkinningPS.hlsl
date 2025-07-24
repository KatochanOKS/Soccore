// SkinningPS.hlsl
Texture2D gTex : register(t0);
SamplerState gSmp : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PS_IN pin) : SV_TARGET
{
    return gTex.Sample(gSmp, pin.uv);
}
