cbuffer ObjectBuffer : register(b0)
{
    float4x4 WorldViewProj;
    float4 Color;
    int UseTexture;
    float3 padding;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL0;
    float2 Tex : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Pos = mul(float4(input.Pos, 1.0f), WorldViewProj);
    output.Normal = input.Normal;
    output.Tex = input.Tex;
    return output;
}
