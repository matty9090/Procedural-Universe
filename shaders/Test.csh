struct TestBuffer
{
    int i;
};

StructuredBuffer<TestBuffer> InBuffer : register(t0);
RWStructuredBuffer<TestBuffer> OutBuffer : register(u0);

[numthreads(1, 1, 1)]
void CSMain(uint3 ID : SV_DispatchThreadID)
{
    OutBuffer[ID.x].i = 2 * InBuffer[ID.x].i;
}