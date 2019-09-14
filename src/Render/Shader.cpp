#include "Shader.hpp"
#include "Services/Log.hpp"

using namespace std;

bool LoadVertexShader(ID3D11Device* device, const wstring& fileName, ID3D11VertexShader** vertexShader, ID3DBlob** shaderCode )
{
	ID3DBlob* errors;

	HRESULT hr = 
		D3DCompileFromFile( fileName.c_str(), // File containing pixel shader (HLSL)
		                       NULL, NULL,       // Advanced compilation options - not needed here
		                       "main",           // Name of entry point in the shader
		                       "vs_4_0",         // Target vertex shader hardware - vs_1_1 is lowest level
		                                         // vs_2_0 works on most modern video cards, vs_4_0 required for DX10
		                       0,                // Additional compilation flags (such as debug flags)
		                       0,                // More compilation flags (added in DX10)
		                       shaderCode,       // Ptr to variable to hold compiled shader code
		                       &errors);
	if (FAILED(hr))
	{
		if (errors)
		{
			void* errorMsg = errors->GetBufferPointer();
            FLog::Get().Log((char*)errorMsg);
			errors->Release();
		}

		return false;
	}
    
	hr = device->CreateVertexShader( (DWORD*)(*shaderCode)->GetBufferPointer(), (*shaderCode)->GetBufferSize(), NULL, vertexShader );

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool LoadGeometryShader(ID3D11Device* device, const wstring& fileName, ID3D11GeometryShader** geometryShader )
{
	ID3DBlob* shaderCode;
	ID3DBlob* errors;

	HRESULT hr = 
		D3DCompileFromFile( fileName.c_str(), // File containing geometry shader (HLSL)
		                       NULL, NULL,       // Advanced compilation options - not needed here
		                       "main",           // Name of entry point in the shader
		                       "gs_4_0",         // Target geometry shader hardware - ps_1_1 is lowest level
		                                         // ps_2_0 works on most modern video cards, ps_4_0 required for DX10
		                       0,                // Additional compilation flags (such as debug flags)
		                       0,                // More compilation flags (added in DX10)
		                       &shaderCode,      // Ptr to variable to hold compiled shader code
		                       &errors);
	if (FAILED(hr))
	{
		if (errors)
		{
			void* errorMsg = errors->GetBufferPointer();
            FLog::Get().Log((char*)errorMsg);
			errors->Release();
		}

		return false;
	}
    
	hr = device->CreateGeometryShader( (DWORD*)shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 
	                                    NULL, geometryShader );
	
	shaderCode->Release();

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool LoadStreamOutGeometryShader(ID3D11Device* device, const wstring& fileName, D3D11_SO_DECLARATION_ENTRY* soDecl, unsigned int soNumEntries,
								  unsigned int soStride, ID3D11GeometryShader** geometryShader )
{
	ID3DBlob* shaderCode;
	ID3DBlob* errors;

	HRESULT hr = 
		D3DCompileFromFile( fileName.c_str(), // File containing geometry shader (HLSL)
		                       NULL, NULL,       // Advanced compilation options - not needed here
		                       "main",           // Name of entry point in the shader
		                       "gs_4_0",         // Target geometry shader hardware - ps_1_1 is lowest level
		                                         // ps_2_0 works on most modern video cards, ps_4_0 required for DX10
		                       0,                // Additional compilation flags (such as debug flags)
		                       0,                // More compilation flags (added in DX10)
		                       &shaderCode,      // Ptr to variable to hold compiled shader code
		                       &errors           // Ptr to variable to hold error messages
                            );

	if (FAILED(hr))
	{
		if (errors)
		{
			void* errorMsg = errors->GetBufferPointer();
            FLog::Get().Log((char*)errorMsg);
			errors->Release();
		}

		return false;
	}

	hr = device->CreateGeometryShaderWithStreamOutput( (DWORD*)shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 
	                                                         soDecl, soNumEntries, NULL, soStride, D3D11_SO_NO_RASTERIZED_STREAM, NULL, geometryShader );
	
	shaderCode->Release();

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool LoadPixelShader(ID3D11Device* device, const wstring& fileName, ID3D11PixelShader** pixelShader )
{
	ID3DBlob* shaderCode;
	ID3DBlob* errors;

	HRESULT hr = 
		D3DCompileFromFile( fileName.c_str(), // File containing pixel shader (HLSL)
		                       NULL, NULL,       // Advanced compilation options - not needed here
		                       "main",           // Name of entry point in the shader
		                       "ps_4_0",         // Target pixel shader hardware - ps_1_1 is lowest level
		                                         // ps_2_0 works on most modern video cards, ps_4_0 required for DX10
		                       0,                // Additional compilation flags (such as debug flags)
		                       0,                // More compilation flags (added in DX10)
		                       &shaderCode,      // Ptr to variable to hold compiled shader code
		                       &errors);       
	if (FAILED(hr))
	{
		if (errors)
		{
			void* errorMsg = errors->GetBufferPointer();
			FLog::Get().Log((char*)errorMsg);
			errors->Release();
		}

		return false;
	}

	hr = device->CreatePixelShader( (DWORD*)shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 
	                                    NULL, pixelShader );
	
	shaderCode->Release();

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}