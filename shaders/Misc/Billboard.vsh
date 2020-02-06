struct VS_InputOutput
{
	float3 Position : POSITION;
	float Scale : TEXCOORD;
	float4 Colour : COLOR;
};

void main(in VS_InputOutput i, out VS_InputOutput o)
{
	o = i;
}