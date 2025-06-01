Texture2D tex : register(t0);
SamplerState smp : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // 「uvが0,0以外の時だけtex.Sample、それ以外は色」
    if (input.Tex.x != 0 || input.Tex.y != 0)
        return tex.Sample(smp, input.Tex);
    else
        return float4(0.2f, 0.7f, 0.3f, 1.0f); // 単色
}
