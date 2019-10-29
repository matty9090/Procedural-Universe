cbuffer cb0
{
    row_major matrix worldViewProj;
};

struct VS_Input
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

void main(in VS_Input v_in, out VS_Output v_out)
{
    v_out.Position = mul(float4(v_in.Position, 1.0f), worldViewProj);
    v_out.TexCoord = v_in.TexCoord;
}