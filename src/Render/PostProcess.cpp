#include "Render/PostProcess.hpp"
#include "Render/Shader.hpp"

#include "Services/Log.hpp"
#include "Services/ResourceManager.hpp"

#include "Core/Except.hpp"
#include "Core/Event.hpp"

#include <DirectXColors.h>

CPostProcess::CPostProcess(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height)
    : Device(device), Context(context), Width(width), Height(height)
{
    DualPostProcess  = std::make_unique<DirectX::DualPostProcess>(device);
    BasicPostProcess = std::make_unique<DirectX::BasicPostProcess>(device);

    Targets[Blur]         = CreateTarget(Device, Width, Height);
    Targets[BloomExtract] = CreateTarget(Device, Width, Height);
    Targets[BloomTarget0] = CreateTarget(Device, Width / 2, Height / 2);
    Targets[BloomTarget1] = CreateTarget(Device, Width / 4, Height / 4);
    Targets[BloomTarget2] = CreateTarget(Device, Width / 8, Height / 8);
    Targets[BloomTarget3] = CreateTarget(Device, Width / 8, Height / 8);
    Targets[BloomTarget4] = CreateTarget(Device, Width / 8, Height / 8);
    Targets[BloomTarget5] = CreateTarget(Device, Width, Height);
    Targets[BloomCombine] = CreateTarget(Device, Width, Height);

    DepthShader = RESM.GetPixelShader(L"shaders/Depth.psh");

    EventStream::Register(EEvent::UseBloomChanged, [this](const EventData& data) {
        UseBloom = static_cast<const BoolEventData&>(data).Value;
    });

    EventStream::Register(EEvent::GaussianBlurChanged, [this](const EventData& data) {
        GaussianBlur = static_cast<const FloatEventData&>(data).Value;
    });

    EventStream::Register(EEvent::BloomBaseChanged, [this](const EventData& data) {
        BloomBase = static_cast<const FloatEventData&>(data).Value;
    });

    EventStream::Register(EEvent::BloomAmountChanged, [this](const EventData& data) {
        BloomAmount = static_cast<const FloatEventData&>(data).Value;
    });

    EventStream::Register(EEvent::BloomSatChanged, [this](const EventData& data) {
        BloomSat = static_cast<const FloatEventData&>(data).Value;
    });

    EventStream::Register(EEvent::BloomBaseSatChanged, [this](const EventData& data) {
        BloomBaseSat = static_cast<const FloatEventData&>(data).Value;
    });
}

void CPostProcess::Render(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView* sceneTex)
{
    if(UseBloom)
    {
        RenderPP(Targets[Blur], [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::GaussianBlur_5x5);
            BasicPostProcess->SetGaussianParameter(GaussianBlur);
            BasicPostProcess->SetSourceTexture(sceneTex);
            BasicPostProcess->Process(Context);
        });

        RenderPP(Targets[BloomExtract], [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::BloomExtract);
            BasicPostProcess->SetSourceTexture(Targets[Blur].Srv.Get());
            BasicPostProcess->Process(Context);
        });

        RenderPP(Targets[BloomTarget0], [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::DownScale_2x2);
            BasicPostProcess->SetSourceTexture(Targets[BloomExtract].Srv.Get());
            BasicPostProcess->Process(Context);
        });

        RenderPP(Targets[BloomTarget1], [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::DownScale_2x2);
            BasicPostProcess->SetSourceTexture(Targets[BloomTarget0].Srv.Get());
            BasicPostProcess->Process(Context);
        });

        RenderPP(Targets[BloomTarget2], [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::DownScale_2x2);
            BasicPostProcess->SetSourceTexture(Targets[BloomTarget1].Srv.Get());
            BasicPostProcess->Process(Context);
        });

        RenderPP(Targets[BloomTarget3], [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::DownScale_2x2);
            BasicPostProcess->SetSourceTexture(Targets[BloomTarget2].Srv.Get());
            BasicPostProcess->Process(Context);
        });

        RenderPP(Targets[BloomTarget4], [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::BloomBlur);
            BasicPostProcess->SetSourceTexture(Targets[BloomTarget3].Srv.Get());
            BasicPostProcess->Process(Context);
        });

        RenderPP(Targets[BloomTarget5], [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::Copy);
            BasicPostProcess->SetSourceTexture(Targets[BloomTarget4].Srv.Get());
            BasicPostProcess->Process(Context);
        });

        RenderPP(Targets[BloomCombine], [&]() {
            DualPostProcess->SetEffect(DirectX::DualPostProcess::BloomCombine);
            DualPostProcess->SetSourceTexture(Targets[BloomTarget5].Srv.Get());
            DualPostProcess->SetSourceTexture2(sceneTex);
            DualPostProcess->SetBloomCombineParameters(BloomAmount, BloomBase, BloomSat, BloomBaseSat);
            DualPostProcess->Process(Context);
        });

        RenderPP(rtv, dsv, [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::Copy);
            BasicPostProcess->SetSourceTexture(Targets[BloomCombine].Srv.Get());
            BasicPostProcess->Process(Context);
        });
    }
    else
    {
        RenderPP(rtv, dsv, [&]() {
            BasicPostProcess->SetEffect(DirectX::BasicPostProcess::Copy);
            BasicPostProcess->SetSourceTexture(sceneTex);
            BasicPostProcess->Process(Context);
        });
    }
}

void CPostProcess::RenderDepth(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView* depthTex)
{
    RenderPP(rtv, Targets[0].Dsv.Get(), [&]() {
        BasicPostProcess->SetEffect(DirectX::BasicPostProcess::Copy);
        BasicPostProcess->SetSourceTexture(depthTex);
        BasicPostProcess->Process(Context, [&]() {
            Context->PSSetShader(DepthShader, 0, 0);
        });
    });
}

void CPostProcess::RenderPP(RenderView target, std::function<void()> func)
{
    target.Clear(Context);
    SetViewport(target.Width, target.Height);
    SetRenderTarget(Context, target);
    func();
}

void CPostProcess::RenderPP(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, std::function<void()> func)
{
    SetViewport(Width, Height);
    Context->OMSetRenderTargets(1, &rtv, dsv);
    func();

    ID3D11ShaderResourceView *const nullSrv[2] = { nullptr, nullptr };
    Context->PSSetShaderResources(0, 2, nullSrv);
}

void CPostProcess::SetViewport(int width, int height)
{
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<float>(width), vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f, vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0, vp.TopLeftY = 0;
    Context->RSSetViewports(1, &vp);
}