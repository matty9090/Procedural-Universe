#include "Scatter.hlsl"
#include "../Common.hlsl"

cbuffer MatrixBuffer : register(b0) {
    row_major matrix worldViewProj;
}

struct VS_OUTPUT {
	float4 Position : SV_POSITION;
	float3 Colour1	: COLOR0;
	float3 Colour2	: COLOR1;
	float3 T0	    : TEXCOORD0;
};

struct VS_INPUT {
	float3 vPosition : POSITION;
};

VS_OUTPUT main(VS_INPUT v_in) {
	VS_OUTPUT Output;
	
	scatter(v_in.vPosition * fOuterRadius);
	
	Output.Position = mul(float4(v_in.vPosition, 1.0f), worldViewProj);
	Output.Colour1	= PrimaryColour;
	Output.Colour2	= SecondaryColour;
	Output.T0	    = T0;

    Output.Position.z = LogDepthBuffer(Output.Position.w);

	return Output;
}