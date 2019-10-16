#include "Render/PostProcess.hpp"
#include "Core/Except.hpp"
#include "Core/Event.hpp"

#include <DirectXColors.h>

PostProcess::PostProcess(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height)
    : Device(device), Context(context), Width(width), Height(height)
{
    DualPostProcess  = std::make_unique<DirectX::DualPostProcess>(device);
    BasicPostProcess = std::make_unique<DirectX::BasicPostProcess>(device);

    Targets[Blur]         = CreateTarget(Width, Height);
    Targets[BloomExtract] = CreateTarget(Width, Height);
    Targets[BloomTarget0] = CreateTarget(Width / 2, Height / 2);
    Targets[BloomTarget1] = CreateTarget(Width / 4, Height / 4);
    Targets[BloomTarget2] = CreateTarget(Width / 8, Height / 8);
    Targets[BloomTarget3] = CreateTarget(Width / 8, Height / 8);
    Targets[BloomTarget4] = CreateTarget(Width / 8, Height / 8);
    Targets[BloomTarget5] = CreateTarget(Width, Height);
    Targets[BloomCombine] = CreateTarget(Width, Height);

    EventStream::Register(EEvent::GaussianBlurChanged, [this](const EventData& data) {
        GaussianBlur = static_cast<const FloatEventData&>(data).Value;
    });

    EventStream::Register(EEvent::BloomBaseChanged, [this](const EventData& data) {
        BloomBase = static_cast<const FloatEventData&>(data).Value;
    });

    EventStream::Register(EEvent::BloomAmountChanged, [this](const EventData& data) {
        BloomAmount = static_cast<const FloatEventData&>(data).Value;
    });
}

void PostProcess::Render(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView* sceneTex)
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
        DualPostProcess->SetBloomCombineParameters(BloomAmount, BloomBase, 1.0f, 1.0f);
        DualPostProcess->Process(Context);
    });

    RenderPP(rtv, dsv, [&]() {
        BasicPostProcess->SetEffect(DirectX::BasicPostProcess::Copy);
        BasicPostProcess->SetSourceTexture(Targets[BloomCombine].Srv.Get());
        BasicPostProcess->Process(Context);
    });
}

void PostProcess::RenderPP(Target target, std::function<void()> func)
{
    target.Clear(Context);
    SetViewport(target.Width, target.Height);
    Context->OMSetRenderTargets(1, target.Rtv.GetAddressOf(), target.Dsv.Get());
    func();
    Context->Draw(0, 0);
}

void PostProcess::RenderPP(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, std::function<void()> func)
{
    SetViewport(Width, Height);
    Context->OMSetRenderTargets(1, &rtv, dsv);
    func();
    Context->Draw(0, 0);

    ID3D11ShaderResourceView *const nullSrv[2] = { nullptr, nullptr };
    Context->PSSetShaderResources(0, 2, nullSrv);
}

void PostProcess::SetViewport(int width, int height)
{
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<float>(width), vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f, vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0, vp.TopLeftY = 0;
    Context->RSSetViewports(1, &vp);
}

PostProcess::Target PostProcess::CreateTarget(int width, int height)
{
    Target t;
    t.Width = width;
    t.Height = height;

    CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height,
                          1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

    CD3D11_TEXTURE2D_DESC dsDesc(DXGI_FORMAT_D32_FLOAT, width, height,
            1,  1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, 1, 0 );

    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);

    DX::ThrowIfFailed(Device->CreateTexture2D(&desc, nullptr, t.Rt.GetAddressOf()));
    DX::ThrowIfFailed(Device->CreateTexture2D(&dsDesc, nullptr, t.Ds.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(Device->CreateShaderResourceView(t.Rt.Get(), nullptr, t.Srv.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(Device->CreateRenderTargetView(t.Rt.Get(), nullptr, t.Rtv.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(Device->CreateDepthStencilView(t.Ds.Get(), &dsvDesc, t.Dsv.ReleaseAndGetAddressOf()));

    return t;
}

void PostProcess::Clear()
{

}

void PostProcess::Target::Clear(ID3D11DeviceContext* context)
{
    context->ClearRenderTargetView(Rtv.Get(), DirectX::Colors::Black);
}