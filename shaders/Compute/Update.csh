struct Particle
{
    float3 Position;
    float4 Colour;
    float4 OriginalColour;

    double3 Velocity;
    double3 Forces;
    
    double Mass;
};

cbuffer FrameBuffer : register(b0)
{
    float FrameTime;
    double Scale;
};

StructuredBuffer<Particle> InBuffer : register(t0);
RWStructuredBuffer<Particle> OutBuffer : register(u0);

[numthreads(1, 1, 1)]
void CSMain(uint3 ID : SV_DispatchThreadID)
{
    OutBuffer[ID.x].Position        = InBuffer[ID.x].Position;
    OutBuffer[ID.x].Colour          = InBuffer[ID.x].Colour;
    OutBuffer[ID.x].OriginalColour  = InBuffer[ID.x].OriginalColour;
    OutBuffer[ID.x].Velocity        = InBuffer[ID.x].Velocity;
    OutBuffer[ID.x].Mass            = InBuffer[ID.x].Mass;

    double3 a = InBuffer[ID.x].Forces / InBuffer[ID.x].Mass;
    OutBuffer[ID.x].Velocity += a * FrameTime;

    double3 v = (OutBuffer[ID.x].Velocity * FrameTime) / Scale;
    OutBuffer[ID.x].Position += v;
    OutBuffer[ID.x].Forces = double3(0, 0, 0);
}