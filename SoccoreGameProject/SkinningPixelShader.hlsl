// ----------------------
// �X�L�j���O�Ή� �s�N�Z���V�F�[�_
// RootParameter
//   t0 : �e�N�X�`��SRV
// �T���v���[��root sig��static
// ----------------------

Texture2D gTex : register(t0);
SamplerState gSmp : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PS_IN pin) : SV_TARGET
{
    return gTex.Sample(gSmp, pin.uv);
}
