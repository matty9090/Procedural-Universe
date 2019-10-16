#pragma once

#include <d3d11.h>
#include <PostProcess.h>
#include <wrl/client.h>

#include <map>
#include <functional>

class PostProcess
{
private:
    struct Target
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> Rt;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> Ds;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> Rtv;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> Dsv;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Srv;

        int Width, Height;

        void Clear(ID3D11DeviceContext* context);
    };

public:
    PostProcess(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height);

    void Render(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView* sceneTex);

private:
    void Clear();
    void RenderPP(Target target, std::function<void()> func);
    void RenderPP(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, std::function<void()> func);
    void SetViewport(int width, int height);

    Target CreateTarget(int width, int height);

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

    std::map<int, Target> Targets;

    bool UseBloom = true;
    float GaussianBlur = 2.0f;
    float BloomAmount = 1.0f, BloomBase = 1.4f;
    float BloomSat = 1.0f, BloomBaseSat = 1.0f;
};