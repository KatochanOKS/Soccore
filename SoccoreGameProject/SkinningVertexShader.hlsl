// ----------------------------
// �X�L�j���O�Ή� ���_�V�F�[�_
// RootParameter
//   b0 : WorldViewProj
//   b1 : BoneMatrices[�ő�{�[����]
// InputLayout
//   POSITION, NORMAL, TEXCOORD, BLENDINDICES, BLENDWEIGHT
// ----------------------------

cbuffer cbPerObject : register(b0) // ���[���h�r���[�ˉe
{
    float4x4 gWorldViewProj;
};
cbuffer cbBones : register(b1) // �{�[���s��z��
{
    float4x4 gBoneMatrices[68]; // �ő�{�[����64�i�K�v�ɉ����đ���OK�j
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

    // ---- �X�L�j���O�v�Z�i�e�{�[���s�� * �E�F�C�g�̍����j ----
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

    // �ŏI���[���h�r���[�ˉe
    vout.pos = mul(skinnedPos, gWorldViewProj);

    // UV���̂܂�
    vout.uv = vin.uv;
    return vout;
}
