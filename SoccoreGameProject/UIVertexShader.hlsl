// 頂点シェーダ（3D変換とUVだけ出せばOK）
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
    float2 Tex : TEXCOORD0;
};
VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;
    o.Pos = mul(float4(input.Pos, 1), WorldViewProj);
    o.Tex = input.Tex;
    return o;
}
