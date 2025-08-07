cbuffer ObjectBuffer : register(b0)
{
    float4x4 WorldViewProj;
    float4 Color;
    int UseTexture;
    float3 padding;
};
Texture2D tex : register(t0);
SamplerState smp : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 baseColor = (UseTexture != 0) ? tex.Sample(smp, input.Tex) * Color : Color;
    return baseColor;
}
