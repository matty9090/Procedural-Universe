struct TestBuffer
{
    int i;
};

RWStructuredBuffer<TestBuffer> Buffer0 : register(u0);

[numthreads(1, 1, 1)]
void CSMain(uint3 ID : SV_DispatchThreadID)
{
    Buffer0[ID.x].i = 2 * Buffer0[ID.x].i;
}