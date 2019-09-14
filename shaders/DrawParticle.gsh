cbuffer cb0
{
    row_major float4x4 ViewProjMatrix;
    row_major float4x4 InvViewMatrix;
};

struct GS_VertIn
{
	float3 Position : POSITION;
};

struct GS_VertOut
{
	float4 ViewportPosition : SV_Position;
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
    
    GS_VertOut outVert;

	for (int i = 0; i < 4; ++i)
	{
		const float scale = 1.0f;
        float3 corner = Corners[i] * scale;
        float3 worldPosition = inParticle[0].Position + mul( corner, (float3x3)InvViewMatrix );
        
		outVert.ViewportPosition = mul( float4(worldPosition, 1.0f), ViewProjMatrix );
		outStrip.Append( outVert );
	}
    
	outStrip.RestartStrip();
}