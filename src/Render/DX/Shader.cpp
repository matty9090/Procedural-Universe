#include "Shader.hpp"
#include "Services/Log.hpp"

using namespace std;

//#define _DEBUG

bool LoadVertexShader(ID3D11Device* device, const wstring& fileName, ID3D11VertexShader** vertexShader, ID3DBlob** shaderCode )
{
	ID3DBlob* errors = nullptr;

	UINT dbgFlags = 0;

#ifdef _DEBUG
	dbgFlags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif

	HRESULT hr = 
		D3DCompileFromFile( fileName.c_str(),
		                       NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		                       "main",
		                       "vs_5_0",
		                       dbgFlags, 0,
		                       shaderCode, 
		                       &errors);
	if (FAILED(hr))
	{
		if (errors)
		{
            void* errorMsg = errors->GetBufferPointer();
            LOGE((char*)errorMsg)
            errors->Release();

            LOGE("Failed to load vertex shader " + wstrtostr(fileName))
		}

		return false;
	}
    
	hr = device->CreateVertexShader( (DWORD*)(*shaderCode)->GetBufferPointer(), (*shaderCode)->GetBufferSize(), NULL, vertexShader );

	if (FAILED(hr))
	{
        LOGE("Failed to load vertex shader " + wstrtostr(fileName))
		return false;
	}

    LOGV("Loaded shader " + wstrtostr(fileName))

	return true;
}

bool LoadGeometryShader(ID3D11Device* device, const wstring& fileName, ID3D11GeometryShader** geometryShader )
{
	ID3DBlob* shaderCode = nullptr;
	ID3DBlob* errors = nullptr;

	UINT dbgFlags = 0;

#ifdef _DEBUG
	dbgFlags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif

	HRESULT hr = 
		D3DCompileFromFile( fileName.c_str(),
		                       NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		                       "main",
		                       "gs_5_0",
		                       dbgFlags, 0,
		                       &shaderCode,  
		                       &errors);
	if (FAILED(hr))
	{
		if (errors)
		{
			void* errorMsg = errors->GetBufferPointer();
            LOGE((char*)errorMsg)
			errors->Release();
		}

        LOGE("Failed to load geometry shader " + wstrtostr(fileName))

		return false;
	}
    
	hr = device->CreateGeometryShader( (DWORD*)shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 
	                                    NULL, geometryShader );
	
	shaderCode->Release();

	if (FAILED(hr))
	{
        LOGE("Failed to load geometry shader " + wstrtostr(fileName))
		return false;
	}

    LOGV("Loaded shader " + wstrtostr(fileName))

	return true;
}

bool LoadStreamOutGeometryShader(ID3D11Device* device, const wstring& fileName, D3D11_SO_DECLARATION_ENTRY* soDecl, unsigned int soNumEntries,
								  unsigned int soStride, ID3D11GeometryShader** geometryShader )
{
	ID3DBlob* shaderCode = nullptr;
	ID3DBlob* errors = nullptr;

	UINT dbgFlags = 0;

#ifdef _DEBUG
	dbgFlags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif

	HRESULT hr = 
		D3DCompileFromFile( fileName.c_str(), // File containing geometry shader (HLSL)
		                       NULL, NULL,       // Advanced compilation options - not needed here
		                       "main",           // Name of entry point in the shader
		                       "gs_5_0",         // Target geometry shader hardware - ps_1_1 is lowest level
		                                         // ps_2_0 works on most modern video cards, ps_4_0 required for DX10
		                       dbgFlags,         // Additional compilation flags (such as debug flags)
		                       0,                // More compilation flags (added in DX10)
		                       &shaderCode,      // Ptr to variable to hold compiled shader code
		                       &errors           // Ptr to variable to hold error messages
                            );

	if (FAILED(hr))
	{
		if (errors)
		{
			void* errorMsg = errors->GetBufferPointer();
            LOGE((char*)errorMsg)
			errors->Release();
		}

        LOGE("Failed to load geometry shader with stream output " + wstrtostr(fileName))

		return false;
	}

	hr = device->CreateGeometryShaderWithStreamOutput( (DWORD*)shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 
	                                                         soDecl, soNumEntries, NULL, soStride, D3D11_SO_NO_RASTERIZED_STREAM, NULL, geometryShader );
	
	shaderCode->Release();

	if (FAILED(hr))
	{
        LOGE("Failed to load geometry shader with stream output " + wstrtostr(fileName))
		return false;
	}

    LOGV("Loaded shader " + wstrtostr(fileName))

	return true;
}

bool LoadPixelShader(ID3D11Device* device, const wstring& fileName, ID3D11PixelShader** pixelShader )
{
	ID3DBlob* shaderCode = nullptr;
	ID3DBlob* errors = nullptr;

	UINT dbgFlags = 0;

#ifdef _DEBUG
	dbgFlags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif

	HRESULT hr = 
		D3DCompileFromFile( fileName.c_str(),
		                    NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		                    "main", "ps_5_0",       
							dbgFlags, 0,
							&shaderCode, &errors);
	if (FAILED(hr))
	{
		if (errors)
		{
			void* errorMsg = errors->GetBufferPointer();
            LOGE((char*)errorMsg)
			errors->Release();
		}

        LOGE("Failed to load pixel shader " + wstrtostr(fileName))

		return false;
	}

	hr = device->CreatePixelShader( (DWORD*)shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 
	                                    NULL, pixelShader );
	
	shaderCode->Release();

	if (FAILED(hr))
	{
        LOGE("Failed to load pixel shader " + wstrtostr(fileName))
		return false;
	}

    LOGV("Loaded shader " + wstrtostr(fileName))

	return true;
}

bool LoadComputeShader(ID3D11Device* device, const std::wstring& fileName, ID3D11ComputeShader** computeShader, const D3D_SHADER_MACRO* defines)
{
	ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                    "CSMain", "cs_5_0",
                                    0, 0, &shaderBlob, &errorBlob);

	
	if(FAILED(hr))
	{
		if(errorBlob)
		{
			void* errorMsg = errorBlob->GetBufferPointer();
            LOGE((char*)errorMsg)
			errorBlob->Release();
		}

		if(shaderBlob)
			shaderBlob->Release();
		
        LOGE("Failed to load compute shader " + wstrtostr(fileName))

		return false;
	}

	if(FAILED(device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, computeShader)))
	{
		LOGE("Failed to load compute shader " + wstrtostr(fileName))
		return false;
	}

    LOGV("Loaded shader " + wstrtostr(fileName))

	return true;
}