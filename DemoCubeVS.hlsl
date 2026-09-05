struct ModelViewProjection
{
    matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexPosColor
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct VertexShaderOutput
{
    float4 color : COLOR;
    float4 position : SV_Position;
};

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    
    OUT.position = mul(ModelViewProjectionCB.MVP, float4(IN.position, 1.0f));
    OUT.color = float4(IN.color, 1.0f);
    
    return OUT;
}
