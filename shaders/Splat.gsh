#include "Common.hlsl"
#include "Volumetric.hlsl"

cbuffer cb0
{
    row_major float4x4 ViewProjMatrix;
    row_major float4x4 InvViewMatrix;
};

cbuffer cb1
{
    float3 Cam;
};

struct GS_VertIn
{
	float3 Position : POSITION;
    float4 Colour : COLOR;
};

struct GS_VertOut
{
    float4 Position : SV_Position;
    float3 Direction : TEXCOORD0;
    float3 RayStart : TEXCOORD1;
    float3 RayEnd : TEXCOORD2;
    float  Size : TEXCOORD3;
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
		const float scale = 30.0f;
        float3 corner = Corners[i] * scale;
        float3 worldPosition = inParticle[0].Position + mul(corner, (float3x3)InvViewMatrix);
        
		outVert.Position = mul(float4(worldPosition, 1.0f), ViewProjMatrix);
        outVert.Position.z = LogDepthBuffer(outVert.Position.w);
        outVert.RayStart = Cam;
        outVert.RayEnd = worldPosition;
        outVert.Size = scale / 2;
        outVert.Direction = inParticle[0].Position + outVert.RayEnd;

		outStrip.Append( outVert );
	}
    
	outStrip.RestartStrip();
}