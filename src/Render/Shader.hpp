#pragma once

#include <string>
#include <d3d11.h>
#include <d3dcompiler.h>

bool LoadVertexShader(ID3D11Device* device, const std::wstring& fileName, ID3D11VertexShader** vertexShader, ID3DBlob** shaderCode );
bool LoadGeometryShader(ID3D11Device* device, const std::wstring& fileName, ID3D11GeometryShader** geometryShader );
bool LoadPixelShader(ID3D11Device* device, const std::wstring& fileName, ID3D11PixelShader** pixelShader );
bool LoadComputeShader(ID3D11Device* device, const std::wstring& fileName, ID3D11ComputeShader** computeShader);
bool LoadStreamOutGeometryShader(ID3D11Device* device, const std::wstring& fileName, D3D11_SO_DECLARATION_ENTRY* soDecl, unsigned int soNumEntries,
								  unsigned int soStride, ID3D11GeometryShader** geometryShader );
