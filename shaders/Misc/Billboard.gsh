#include "../Common.hlsl"

cbuffer cb0
{
    row_major float4x4 ViewProjMatrix;
    row_major float4x4 InvViewMatrix;
    float3 Cam;
    float  Pad0;
    float3 Fade;
    float  Pad1;
};

struct GS_VertIn
{
    float3 Position : POSITION;
    float  Scale : TEXCOORD;
    float4 Colour : COLOR;
};

struct GS_VertOut
{
	float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float4 Colour : COLOR;
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
        float3 corner = Corners[i] * inParticle[0].Scale;
        float3 worldPosition = inParticle[0].Position + mul(corner, (float3x3)InvViewMatrix);
        
		outVert.Position = mul(float4(worldPosition, 1.0f), ViewProjMatrix);
        outVert.Position.z = LogDepthBuffer(outVert.Position.w);
        outVert.Colour = inParticle[0].Colour;

        float t = (length(Cam - inParticle[0].Position) + Fade.y) / Fade.z;
        outVert.Colour.a = lerp(outVert.Colour.a, clamp(lerp(0.0f, inParticle[0].Colour.a, t), 0.0f, inParticle[0].Colour.a), Fade.x);

        outVert.UV = UVs[i];
		outStrip.Append(outVert);
	}
    
	outStrip.RestartStrip();
}