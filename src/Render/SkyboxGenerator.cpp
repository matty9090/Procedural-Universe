#include "SkyboxGenerator.hpp"
#include "Services/Log.hpp"

#include <wincodec.h>
#include <ScreenGrab.h>

bool CSkyboxGenerator::bMadeViews;
D3D11_TEXTURE2D_DESC  CSkyboxGenerator::TexArrayDesc;
std::array<RenderView, 6> CSkyboxGenerator::Views;
Microsoft::WRL::ComPtr<ID3D11Texture2D> CSkyboxGenerator::TexArray;

CSkyboxGenerator::CSkyboxGenerator(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height)
    : Device(device),
      Context(context),
      Camera()
{
    if (!bMadeViews)
    {
        for (int i = 0; i < 6; ++i)
        {
            Views[i] = CreateTarget(Device, 2048, 2048);
        }

        D3D11_TEXTURE2D_DESC texElementDesc;
        ((ID3D11Texture2D*)Views[0].Rt.Get())->GetDesc(&texElementDesc);

        TexArrayDesc.Width = texElementDesc.Width;
        TexArrayDesc.Height = texElementDesc.Height;
        TexArrayDesc.MipLevels = 1;
        TexArrayDesc.ArraySize = 6;
        TexArrayDesc.Format = texElementDesc.Format;
        TexArrayDesc.SampleDesc.Count = 1;
        TexArrayDesc.SampleDesc.Quality = 0;
        TexArrayDesc.Usage = D3D11_USAGE_DEFAULT;
        TexArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        TexArrayDesc.CPUAccessFlags = 0;
        TexArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        if (FAILED(Device->CreateTexture2D(&TexArrayDesc, 0, &TexArray)))
        {
            LOGE("Failed to create texture for skybox");
        }
    }

    bMadeViews = true;
}

void CSkyboxGenerator::Render(std::function<void(const ICamera&)> renderFunc)
{
    UINT numVp = 1;
    D3D11_VIEWPORT oldVp;
    Context->RSGetViewports(&numVp, &oldVp);

    D3D11_VIEWPORT vp = oldVp;
    vp.Width = 2048.0f;
    vp.Height = 2048.0f;

    Context->RSSetViewports(1, &vp);

    for (int i = 0; i < 6; ++i)
    {
        Views[i].Clear(Context);
        Camera.MakeLookAt(ForwardVectors[i], UpVectors[i]);
        Context->OMSetRenderTargets(1, Views[i].Rtv.GetAddressOf(), Views[i].Dsv.Get());
        renderFunc(Camera);
    }

    Context->RSSetViewports(1, &oldVp);

    Generate();
}

void CSkyboxGenerator::Generate()
{
    D3D11_BOX sourceRegion;

    for (UINT i = 0; i < 6; ++i)
    {
        sourceRegion.left = 0;
        sourceRegion.right = TexArrayDesc.Width;
        sourceRegion.top = 0;
        sourceRegion.bottom = TexArrayDesc.Height;
        sourceRegion.front = 0;
        sourceRegion.back = 1;

        if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
            break;

        Context->CopySubresourceRegion(
            TexArray.Get(),
            D3D11CalcSubresource(0, i, TexArrayDesc.MipLevels),
            0, 0, 0,
            Views[i].Rt.Get(), 0, &sourceRegion
        );
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    viewDesc.Format = TexArrayDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    viewDesc.TextureCube.MostDetailedMip = 0;
    viewDesc.TextureCube.MipLevels = TexArrayDesc.MipLevels;

    if (FAILED(Device->CreateShaderResourceView(TexArray.Get(), &viewDesc, Srv.ReleaseAndGetAddressOf())))
    {
        LOGE("Failed to create srv for skybox");
    }
}

CSkyboxCamera::CSkyboxCamera()
{
    Proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PIDIV2, 1.0f, 10.f, 2000.0f);
}

void CSkyboxCamera::MakeLookAt(DirectX::SimpleMath::Vector3 target, DirectX::SimpleMath::Vector3 up)
{
    View = DirectX::SimpleMath::Matrix::CreateLookAt(Position, Position + target, up);
}
