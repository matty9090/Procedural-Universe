#pragma once

#include <d3d11.h>
#include <PostProcess.h>
#include <wrl/client.h>

#include <map>
#include <functional>

#include "RenderCommon.hpp"

class CPostProcess
{
public:
    CPostProcess(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height);

    void Render(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView* sceneTex);

    bool UseBloom = true;
    float GaussianBlur = 2.0f;
    float BloomAmount = 1.0f, BloomBase = 1.4f;
    float BloomSat = 1.0f, BloomBaseSat = 1.0f;

private:
    void RenderPP(RenderView target, std::function<void()> func);
    void RenderPP(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, std::function<void()> func);
    void SetViewport(int width, int height);

    ID3D11Device* Device;
    ID3D11DeviceContext *Context;

    int Width, Height;

    std::unique_ptr<DirectX::DualPostProcess>  DualPostProcess;
    std::unique_ptr<DirectX::BasicPostProcess> BasicPostProcess;

    enum ETargets
    {
        Blur,
        BloomExtract,
        BloomTarget0,
        BloomTarget1,
        BloomTarget2,
        BloomTarget3,
        BloomTarget4,
        BloomTarget5,
        BloomCombine
    };

    std::map<int, RenderView> Targets;
};