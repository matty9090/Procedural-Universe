#include "../Common.hlsl"

cbuffer cb0
{
    row_major matrix WorldViewProj;
    row_major matrix World;
};

struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

void main(in VS_Input v_in, out VS_Output v_out)
{
    v_out.Position = mul(float4(v_in.Position, 1.0f), WorldViewProj);
    v_out.Normal   = mul(float4(v_in.Normal, 1.0f), World).xyz;
    v_out.TexCoord = v_in.TexCoord;
    v_out.Position.z = LogDepthBuffer(v_out.Position.w);
}