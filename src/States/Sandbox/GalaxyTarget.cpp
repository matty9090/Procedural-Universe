#include "GalaxyTarget.hpp"

GalaxyTarget::GalaxyTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Galactic", resources, camera, rtv)
{
    Scale = 0.0005f;
    BeginTransitionDist = 4000.0f;
    EndTransitionDist = 100.0f;

    auto vp = Resources->GetScreenViewport();
    unsigned int width = static_cast<size_t>(vp.Width);
    unsigned int height = static_cast<size_t>(vp.Height);

    Splatting = std::make_unique<CSplatting>(Context, width, height);
    PostProcess = std::make_unique<CPostProcess>(Device, Context, width, height);
    CommonStates = std::make_unique<DirectX::CommonStates>(Device);
    GalaxyRenderer = std::make_unique<Galaxy>(Context);
}

void GalaxyTarget::Seed(uint64_t seed)
{
    GalaxyRenderer->Seed(seed);
}

void GalaxyTarget::Render()
{
    RenderParentSkybox();
    RenderLerp(1.0f);
}

void GalaxyTarget::RenderTransitionChild(float t)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);

    RenderLerp(t, Scale, ParentLocationSpace, true);
}

void GalaxyTarget::RenderTransitionParent(float t)
{
    RenderParentSkybox();
    RenderLerp(t, 1.0f, Vector3::Zero, true);
}

void GalaxyTarget::MoveObjects(Vector3 v)
{
    Centre += v;
    GalaxyRenderer->Move(v);
}

void GalaxyTarget::ScaleObjects(float scale)
{
    GalaxyRenderer->Scale(scale);
}

void GalaxyTarget::ResetObjectPositions()
{
    MoveObjects(-Centre);
    Centre = Vector3::Zero;
}

Vector3 GalaxyTarget::GetClosestObject(Vector3 pos)
{
    return GalaxyRenderer->GetClosestObject(pos);
}

void GalaxyTarget::RenderLerp(float t, float scale, Vector3 voffset, bool single)
{
    auto dsv = Resources->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RenderTarget, dsv);
    GalaxyRenderer->Render(*Camera, t, scale, voffset, single);

    //Splatting->Render(static_cast<UINT>(Particles.size()), Camera->GetPosition());
}

void GalaxyTarget::BakeSkybox(Vector3 object)
{
    SkyboxGenerator->Render([&](const ICamera& cam) {
        GalaxyRenderer->Render(cam, 1.0f);
    });
}