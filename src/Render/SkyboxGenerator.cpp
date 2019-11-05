#include "SkyboxGenerator.hpp"
#include "Services/Log.hpp"

#include <wincodec.h>
#include <ScreenGrab.h>

CSkyboxGenerator::CSkyboxGenerator(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height)
    : Device(device),
      Context(context),
      Camera()
{
    for (int i = 0; i < 6; ++i)
    {
        Views[i] = CreateTarget(Device, 2048, 2048);
    }
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

    /*for (int i = 0; i < 6; ++i)
    {
        auto file = std::wstring(L"assets/Skybox") + std::to_wstring(i + 1).c_str() + L".png";
        DirectX::SaveWICTextureToFile(Context, Views[i].Rt.Get(), GUID_ContainerFormatPng, file.c_str());
    }*/

    bSuccess = Generate();
}

bool CSkyboxGenerator::Generate()
{
    D3D11_TEXTURE2D_DESC texElementDesc;
    ((ID3D11Texture2D*)Views[0].Rt.Get())->GetDesc(&texElementDesc);

    // TODO: Optimise by precalculating desc?
    D3D11_TEXTURE2D_DESC texArrayDesc;
    texArrayDesc.Width = texElementDesc.Width;
    texArrayDesc.Height = texElementDesc.Height;
    texArrayDesc.MipLevels = 1;
    texArrayDesc.ArraySize = 6;
    texArrayDesc.Format = texElementDesc.Format;
    texArrayDesc.SampleDesc.Count = 1;
    texArrayDesc.SampleDesc.Quality = 0;
    texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
    texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texArrayDesc.CPUAccessFlags = 0;
    texArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    ID3D11Texture2D* texArray = 0;

    if (FAILED(Device->CreateTexture2D(&texArrayDesc, 0, &texArray)))
    {
        LOGE("Failed to create texture for skybox");
        return false;
    }

    D3D11_BOX sourceRegion;

    for (UINT i = 0; i < 6; ++i)
    {
        sourceRegion.left = 0;
        sourceRegion.right = texArrayDesc.Width;
        sourceRegion.top = 0;
        sourceRegion.bottom = texArrayDesc.Height;
        sourceRegion.front = 0;
        sourceRegion.back = 1;

        if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
            break;

        Context->CopySubresourceRegion(
            texArray,
            D3D11CalcSubresource(0, i, texArrayDesc.MipLevels),
            0, 0, 0,
            Views[i].Rt.Get(), 0, &sourceRegion
        );
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    viewDesc.Format = texArrayDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    viewDesc.TextureCube.MostDetailedMip = 0;
    viewDesc.TextureCube.MipLevels = texArrayDesc.MipLevels;

    if (FAILED(Device->CreateShaderResourceView(texArray, &viewDesc, Srv.ReleaseAndGetAddressOf())))
    {
        LOGE("Failed to create srv for skybox");
        return false;
    }

    return true;
}

CSkyboxCamera::CSkyboxCamera()
{
    Proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI / 2.0f, 1.0f, 10.f, 2000.0f);
}

void CSkyboxCamera::MakeLookAt(DirectX::SimpleMath::Vector3 target, DirectX::SimpleMath::Vector3 up)
{
    View = DirectX::SimpleMath::Matrix::CreateLookAt(Position, Position + target, up);
}
