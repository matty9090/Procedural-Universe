struct VS_InputOutput
{
	float3 Position : POSITION;
    float4 Colour   : COLOR;
	float  Scale    : TEXCOORD;
};

VS_InputOutput main(VS_InputOutput i)
{
	return i;
}