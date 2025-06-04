cbuffer ObjectBuffer : register(b0) // b0: 行列と色
{
    float4x4 WorldViewProj;
    float4 Color; // 追加
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
    // UVが(0,0)なら「色」で塗る、それ以外はテクスチャ
    if (input.Tex.x != 0 || input.Tex.y != 0)
        return tex.Sample(smp, input.Tex);
    else
        return Color; // 定数バッファから色取得
}
