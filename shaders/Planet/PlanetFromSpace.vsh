#include "../Volumetric/Scatter.hlsl"
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
	float4 Colour   : COLOR;
};

struct VS_Output
{
    float4 Position  : SV_POSITION;
	float3 WorldPos  : POSITION;
    float3 Normal    : NORMAL;
	float4 Colour0	 : COLOR0;
	float4 Colour1	 : COLOR1;
	float4 Colour2	 : COLOR2;
	float  Intensity : COLOR3;
};

void main(in VS_Input v_in, out VS_Output v_out)
{
	float3 objPos = float3(World[3][0], World[3][1], World[3][2]);
	float3 pos = mul(float4(v_in.Position, 1.0f), World).xyz;
	scatterGroundFromSpace(pos, objPos);
	
    v_out.Position  = mul(float4(v_in.Position, 1.0f), WorldViewProj);
    v_out.WorldPos  = pos;
    v_out.Normal    = normalize(mul(float4(v_in.Normal, 1.0), World).xyz);
    v_out.Colour0   = v_in.Colour;
    v_out.Colour1   = float4(PrimaryColour, 0.0f);
    v_out.Colour2   = float4(SecondaryColour, 0.0f);
    v_out.Intensity = (dot(v3LightPos, v_out.Normal) + 1.0) * 0.5;
    v_out.Position.z = LogDepthBuffer(v_out.Position.w);
}