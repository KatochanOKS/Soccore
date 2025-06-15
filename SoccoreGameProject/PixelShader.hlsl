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
    float3 Normal : NORMAL0;
    float2 Tex : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // 正規化法線
    float3 normal = normalize(input.Normal);

    // 光源ベクトル（好きな方向・ここでは左上奥から）
    float3 lightDir = normalize(float3(1, 5, -6)); // 右上手前から

    // 視線ベクトル（zマイナス方向＝カメラ前方想定）
    float3 viewDir = normalize(float3(0, 0, -1));

    // Lambert拡散
    float diff = max(dot(normal, lightDir), 0.0);

    // 環境光（暗い部分も多少明るくする）
    float ambient = 0.7; // より暗い部分が強調される


    // Blinn-Phongスペキュラ
    float3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 8.0);

    // ベースカラー（テクスチャ×色 or 色のみ）
    float4 baseColor = (UseTexture != 0) ? tex.Sample(smp, input.Tex) * Color : Color;

    // 合成（RGB各要素で）
    float3 finalColor = (ambient + diff) * baseColor.rgb + spec * float3(1, 1, 1);
    return float4(saturate(finalColor), baseColor.a);
}
//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    return float4(input.Tex, 0, 1); // 赤=U, 緑=V, 青=0, α=1
//}
