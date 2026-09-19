/*
ALTERNATIVE


cbuffer ViewMatrix : register(b0)
{
    matrix View;
}

cbuffer ProjectionMatrix : register(b1)
{
    matrix Projection;
}

cbuffer ModelMatrix : register(b2)
{
    matrix Model;
}

*/

struct ConstantBufferMatrix
{
    matrix mMatrix;
};

ConstantBuffer<ConstantBufferMatrix> View : register(b0);
ConstantBuffer<ConstantBufferMatrix> Projection : register(b1);
ConstantBuffer<ConstantBufferMatrix> Model : register(b2);

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
    
    matrix final = mul(Projection.mMatrix, View.mMatrix);
    final = mul(final, Model.mMatrix);
    OUT.position = mul(final, float4(IN.position, 1.0f));
    
    OUT.color = float4(IN.color, 1.0f);
    
    return OUT;
}