struct PixelShaderInput
{
    float4 color : COLOR;
};

float4 main(PixelShaderInput IN) : SV_Target
{
    return IN.color;
}