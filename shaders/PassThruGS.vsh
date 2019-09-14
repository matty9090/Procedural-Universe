struct VS_InputOutput
{
	float3 Position : POSITION;
};

void main( in VS_InputOutput i, out VS_InputOutput o ) 
{
	o = i;
}