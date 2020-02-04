#include "GalaxyTarget.hpp"
#include "Sim/IParticleSeeder.hpp"

GalaxyTarget::GalaxyTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv)
    : SandboxTarget(context, "Galactic", "Star", resources, camera, rtv)
{
    Scale = 0.01f;
    BeginTransitionDist = 4000.0f;
    EndTransitionDist = 400.0f;

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
    GalaxyRenderer->Scale(4000.0f * Scale);
}

void GalaxyTarget::Render()
{
    RenderParentSkybox();
    RenderLerp(1.0f, 1.0f / Scale);
}

void GalaxyTarget::RenderTransitionChild(float t)
{
    RenderLerp(t, 1.0f, ParentLocationSpace, true);
}

void GalaxyTarget::RenderTransitionParent(float t)
{
    RenderParentSkybox();
    RenderLerp(t, 1.0f / Scale, Vector3::Zero, true);
}

void GalaxyTarget::MoveObjects(Vector3 v)
{
    GalaxyRenderer->Move(v);
    Centre += v;
}

void GalaxyTarget::ScaleObjects(float scale)
{
    GalaxyRenderer->Scale(scale);
}

void GalaxyTarget::ResetObjectPositions()
{
    GalaxyRenderer->Move(-Centre);
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
}

void GalaxyTarget::BakeSkybox(Vector3 object)
{
    SkyboxGenerator->Render([&](const ICamera& cam) {
        Parent->GetSkyBox().Draw(cam.GetViewMatrix() * cam.GetProjectionMatrix());
        GalaxyRenderer->Render(cam, 0.0f, 1.0f / Scale, Vector3::Zero, true);
    });
}
