// ----------------------------
// スキニング対応 頂点シェーダ
// RootParameter
//   b0 : WorldViewProj
//   b1 : BoneMatrices[最大ボーン数]
// InputLayout
//   POSITION, NORMAL, TEXCOORD, BLENDINDICES, BLENDWEIGHT
// ----------------------------

cbuffer cbPerObject : register(b0) // ワールドビュー射影
{
    float4x4 gWorldViewProj;
};
cbuffer cbBones : register(b1) // ボーン行列配列
{
    float4x4 gBoneMatrices[68]; // 最大ボーン数64（必要に応じて増減OK）
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
    VS_OUT vout;

    // ---- スキニング計算（各ボーン行列 * ウェイトの合成） ----
    float4 skinnedPos = float4(0, 0, 0, 0);

    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        float w = vin.boneWeights[i];
        uint idx = vin.boneIndices[i];
        if (w > 0)
        {
            skinnedPos += mul(float4(vin.pos, 1.0f), gBoneMatrices[idx]) * w;
        }
    }

    // 最終ワールドビュー射影
    vout.pos = mul(skinnedPos, gWorldViewProj);

    // UVそのまま
    vout.uv = vin.uv;
    return vout;
}
