#include "Common.hlsl"

cbuffer cb0
{
    row_major float4x4 ViewProjMatrix;
    row_major float4x4 InvViewMatrix;
    float3 Translation;
};

cbuffer cb1
{
    float Lerp;
};

struct GS_VertIn
{
    float3 Position : POSITION;
    float4 Colour   : COLOR;
    float  Scale : TEXCOORD;
};

struct GS_VertOut
{
	float4 ViewportPosition : SV_Position;
    float4 Colour : COLOR;
    float2 UV : TEXCOORD0;
};

[maxvertexcount(4)]  
void main
(
	point GS_VertIn                  inParticle[1], 
	inout TriangleStream<GS_VertOut> outStrip
)
{
    const float3 Corners[4] =
    {
        float3(-1,  1, 0),
        float3( 1,  1, 0),
        float3(-1, -1, 0),
        float3( 1, -1, 0),
    };

    const float2 UVs[4] =
    {
        float2( 0, 0),
        float2( 1, 0),
        float2( 0, 1),
        float2( 1, 1),
    };
    
    GS_VertOut outVert;

	for (int i = 0; i < 4; ++i)
	{
		const float scale = 300.0f;
        float3 corner = Corners[i] * scale * Lerp;
        float3 worldPosition = inParticle[0].Position + mul(corner, (float3x3)InvViewMatrix) + Translation;
        
		outVert.ViewportPosition = mul( float4(worldPosition, 1.0f), ViewProjMatrix );
        outVert.ViewportPosition.z = LogDepthBuffer(outVert.ViewportPosition.w);

        outVert.Colour = float4(inParticle[0].Colour.rgb, Lerp);
        outVert.UV = UVs[i];
		outStrip.Append( outVert );
	}
    
	outStrip.RestartStrip();
}