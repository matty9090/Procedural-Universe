#include "../Common.hlsl"

cbuffer cb0
{
    row_major matrix WorldViewProj;
    row_major matrix World;
};

struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Colour : COLOR;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
	float3 WorldPos : POSITION;
    float3 Normal : NORMAL;
	float4 Colour : COLOR;
};

void main(in VS_Input v_in, out VS_Output v_out)
{
    v_out.Position = mul(float4(v_in.Position, 1.0f), WorldViewProj);
    v_out.WorldPos = mul(float4(v_in.Position, 1.0f), World);
    v_out.Normal = normalize(v_in.Normal);
    v_out.Colour = v_in.Colour;

    v_out.Position.z = LogDepthBuffer(v_out.Position.w);
}