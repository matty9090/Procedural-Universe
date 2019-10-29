#pragma once

#include <string>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXColors.h>
#include <functional>

#include "Core/Except.hpp"
#include "Render/Particle.hpp"

struct RenderView
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> Rt;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> Ds;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> Rtv;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> Dsv;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Srv;

    int Width, Height;

    void Clear(ID3D11DeviceContext* context);
};

struct RenderPipeline
{
    Microsoft::WRL::ComPtr<ID3D11InputLayout>     InputLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>    VertexShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader>  GeometryShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>     PixelShader;

    D3D_PRIMITIVE_TOPOLOGY Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    void LoadVertex(ID3D11Device* device, std::wstring file);
    void LoadPixel(ID3D11Device* device, std::wstring file);
    void LoadGeometry(ID3D11Device* device, std::wstring file);
    void CreateInputLayout(ID3D11Device* device, std::vector<D3D11_INPUT_ELEMENT_DESC> layout);

    void SetState(ID3D11DeviceContext* context, std::function<void()> state);

private:
    ID3DBlob* VertexCode;
};

RenderView CreateTarget(ID3D11Device* device, int width, int height);
void CreateParticleBuffer(ID3D11Device* device, ID3D11Buffer** buffer, const std::vector<Particle>& particles);