Texture2D tex : register(t0);
SamplerState smp : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 color = tex.Sample(smp, input.Tex);
    // そのまま返すとアルファも出力される
    return color;
}
