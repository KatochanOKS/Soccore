cbuffer MatrixBuffer : register(b0)
{
    float4x4 WorldViewProj;
};
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};
VS_OUTPUT main(float3 pos : POSITION, float2 uv : TEXCOORD)
{
    VS_OUTPUT output;
    output.Pos = mul(float4(pos, 1.0f), WorldViewProj); // 3D行列でスクリーン変換
    output.Tex = uv;
    return output;
}
