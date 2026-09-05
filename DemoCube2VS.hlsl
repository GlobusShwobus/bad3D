struct ViewMatrix
{
    matrix View;
};

struct ProjectionMatrix
{
    matrix Projection;
};

struct ModelMatrix
{
    matrix Model;
};

ConstantBuffer<ViewMatrix> ViewCB : register(b0);
ConstantBuffer<ProjectionMatrix> ProjectionCB : register(b1);
ConstantBuffer<ModelMatrix> ModelCB : register(b2);

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
    
    matrix final = mul(ProjectionCB.Projection, ViewCB.View);
    final = mul(final, ModelCB.Model);
    OUT.position = mul(final, float4(IN.position, 1.0f));
    
    OUT.color = float4(IN.color, 1.0f);
    
    return OUT;
}