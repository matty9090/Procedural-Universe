#include "Skybox.hpp"
#include "Services/ResourceManager.hpp"

CSkyBox::CSkyBox(ID3D11DeviceContext* context) : Context(context)
{
    ID3D11Device* device;
    Context->GetDevice(&device);

    Pipeline.Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    Pipeline.LoadVertex(L"shaders/Skybox.vsh");
    Pipeline.LoadPixel(L"shaders/Skybox.psh");
    Pipeline.CreateInputLayout(device, CreateInputLayoutPositionTexture());

    Sphere = std::make_unique<CModel>(device, RESM.GetMesh("assets/Sphere.obj"));
    Sphere->Scale(50000.0f);

    CommonStates = std::make_unique<DirectX::CommonStates>(device);
}

void CSkyBox::Draw(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Matrix viewProj)
{
    auto sampler = CommonStates->AnisotropicWrap();

    Sphere->SetPosition(position);
    Context->RSSetState(CommonStates->CullNone());
    Context->PSSetSamplers(0, 1, &sampler);
    Sphere->Draw(Context, viewProj, Pipeline);
}

void CSkyBox::SetTextureReceiveOwnership(ID3D11ShaderResourceView* tex)
{
    Sphere->SetTexture(tex);
    Texture.Attach(tex);
}
