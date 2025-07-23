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
    // ���K���@��
    float3 normal = normalize(input.Normal);

    // �����x�N�g���i�D���ȕ����E�����ł͍��㉜����j
    float3 lightDir = normalize(float3(1, 5, -6)); // �E���O����

    // �����x�N�g���iz�}�C�i�X�������J�����O���z��j
    float3 viewDir = normalize(float3(0, 0, -1));

    // Lambert�g�U
    float diff = max(dot(normal, lightDir), 0.0);

    // �����i�Â��������������邭����j
    float ambient = 0.7; // ���Â����������������


    // Blinn-Phong�X�y�L����
    float3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 8.0);

    // �x�[�X�J���[�i�e�N�X�`���~�F or �F�̂݁j
    float4 baseColor = (UseTexture != 0) ? tex.Sample(smp, input.Tex) * Color : Color;

    // �����iRGB�e�v�f�Łj
    float3 finalColor = (ambient + diff) * baseColor.rgb + spec * float3(1, 1, 1);
    return float4(saturate(finalColor), baseColor.a);
}
//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    return float4(input.Tex, 0, 1); // ��=U, ��=V, ��=0, ��=1
//}
