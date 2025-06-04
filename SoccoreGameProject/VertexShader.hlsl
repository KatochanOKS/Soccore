cbuffer ObjectBuffer : register(b0) // b0: çsóÒÇ∆êF
{
    float4x4 WorldViewProj;
    float4 Color; // í«â¡
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

VS_OUTPUT main(float3 pos : POSITION, float2 uv : TEXCOORD)
{
    VS_OUTPUT output;
    output.Pos = mul(float4(pos, 1.0f), WorldViewProj);
    output.Tex = uv;
    return output;
}
