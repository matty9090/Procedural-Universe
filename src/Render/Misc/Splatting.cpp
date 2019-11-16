#include "Splatting.hpp"
#include "Render/DX/Shader.hpp"
#include "Services/ResourceManager.hpp"

#include <DirectXColors.h>

CSplatting::CSplatting(ID3D11DeviceContext* context, int width, int height)
    : Context(context)
{
    ID3D11Device* device;
    context->GetDevice(&device);

    Buffer = std::make_unique<ConstantBuffer<CamBuffer>>(device);
    States = std::make_unique<DirectX::CommonStates>(device);
    DualPostProcess  = std::make_unique<DirectX::DualPostProcess>(device);
    BasicPostProcess = std::make_unique<DirectX::BasicPostProcess>(device);

    PixelShader = RESM.GetPixelShader(L"shaders/Splat.psh");
    GeometryShader = RESM.GetGeometryShader(L"shaders/Splat.gsh");

    Target0 = CreateTarget(device, width, height);
    Target1 = CreateTarget(device, width, height);
}

void CSplatting::Render(unsigned int num, DirectX::SimpleMath::Vector3 cam)
{
    //ID3D11RenderTargetView* OriginalTarget;
    //ID3D11DepthStencilView* OriginalDepthView;
    //Context->OMGetRenderTargets(1, &OriginalTarget, &OriginalDepthView);
    
    Buffer->SetData(Context, { cam });

    /////
    //Target0.Clear(Context);
    Context->GSSetShader(GeometryShader, 0, 0);
    Context->PSSetShader(PixelShader, 0, 0);
    Context->GSSetConstantBuffers(1, 1, Buffer->GetBuffer());
    Context->OMSetBlendState(States->NonPremultiplied(), DirectX::Colors::Black, 0xFFFFFFFF);
    //Context->OMSetRenderTargets(1, &OriginalTarget, OriginalDepthView);
    //Context->OMSetRenderTargets(1, Target0.Rtv.GetAddressOf(), Target0.Dsv.Get());
    Context->Draw(num, 0);
    Context->GSSetShader(nullptr, 0, 0);

    /////
   /* BasicPostProcess->SetEffect(DirectX::BasicPostProcess::GaussianBlur_5x5);
    BasicPostProcess->SetGaussianParameter(1.0f);
    BasicPostProcess->SetSourceTexture(Target0.Srv.Get());
    Context->OMSetBlendState(States->Additive(), DirectX::Colors::Black, 0xFFFFFFFF);
    Context->OMSetRenderTargets(1, &OriginalTarget, OriginalDepthView);
    BasicPostProcess->Process(Context);
    Context->GSSetShader(nullptr, 0, 0);*/
}