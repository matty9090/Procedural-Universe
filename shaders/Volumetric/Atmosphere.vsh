#include "Scatter.hlsl"
#include "../Common.hlsl"

cbuffer MatrixBuffer : register(b0) {
    row_major matrix WorldViewProj;
    row_major matrix World;
}

struct VS_OUTPUT {
	float4 Position : SV_POSITION;
	float3 Colour1	: COLOR0;
	float3 Colour2	: COLOR1;
	float3 T0	    : TEXCOORD0;
	float  Depth    : TEXCOORD1;
};

struct VS_INPUT {
	float3 vPosition : POSITION;
};

VS_OUTPUT main(VS_INPUT v_in) {
	VS_OUTPUT Output;
	
	float3 objPos = float3(World[3][0], World[3][1], World[3][2]);
	float3 pos = mul(float4(v_in.Position, 1.0f), World);
	scatterSkyFromSpace(pos);
	
	Output.Position = mul(float4(v_in.Position, 1.0f), WorldViewProj);
	Output.Colour1	= PrimaryColour;
	Output.Colour2	= SecondaryColour;
	Output.T0	    = T0;
	Output.Depth    = Depth;

    Output.Position.z = LogDepthBuffer(Output.Position.w);

	return Output;
}