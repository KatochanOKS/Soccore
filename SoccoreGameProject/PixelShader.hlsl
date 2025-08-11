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
    float3 Normal : NORMAL0; // 未使用だけど構造体は合わせておく
    float2 Tex : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // ベースカラー（テクスチャ × 色 or 色のみ）
    float4 baseColor = (UseTexture != 0) ? tex.Sample(smp, input.Tex) * Color : Color;

    // ライティングなしでそのまま返す
    return baseColor;
}
