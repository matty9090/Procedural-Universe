#include "../Common.hlsl"

cbuffer cb0
{
    row_major matrix viewProj;
	float scale;
};

struct VS_Input
{
    float3 Position : POSITION;
	float2 UV : TEXCOORD;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
};

void main(in VS_Input v_in, out VS_Output v_out)
{
    v_out.Position = mul(float4(v_in.Position * scale, 1.0f), viewProj);
    v_out.Position.z = LogDepthBuffer(v_out.Position.w);
	v_out.UV = v_in.UV;
}