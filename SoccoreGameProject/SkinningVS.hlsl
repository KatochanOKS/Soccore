// SkinningVS.hlsl
cbuffer cbPerObject : register(b0) // WorldViewProj
{
    float4x4 gWorldViewProj;
};
cbuffer cbBones : register(b1) // ボーン行列配列
{
    float4x4 gBoneMatrices[64]; // 最大ボーン数
}
struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    uint4 boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
};
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};
VS_OUT main(VS_IN vin)
{
    float4 skinnedPos = float4(0, 0, 0, 0);
    for (int i = 0; i < 4; ++i)
    {
        skinnedPos += mul(float4(vin.pos, 1), gBoneMatrices[vin.boneIndices[i]]) * vin.boneWeights[i];
    }
    VS_OUT vout;
    vout.pos = mul(skinnedPos, gWorldViewProj);
    vout.uv = vin.uv;
    return vout;
}
