cbuffer ObjectBuffer : register(b0) // b0: �s��ƐF
{
    float4x4 WorldViewProj;
    float4 Color; // �ǉ�
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
    // UV��(0,0)�Ȃ�u�F�v�œh��A����ȊO�̓e�N�X�`��
    if (input.Tex.x != 0 || input.Tex.y != 0)
        return tex.Sample(smp, input.Tex);
    else
        return Color; // �萔�o�b�t�@����F�擾
}
