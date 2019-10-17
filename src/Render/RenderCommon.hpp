#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXColors.h>

#include "Core/Except.hpp"

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

RenderView CreateTarget(ID3D11Device* device, int width, int height);