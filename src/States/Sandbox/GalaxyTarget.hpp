#include "SandboxTarget.hpp"

#include "Render/Misc/Splatting.hpp"
#include "Render/Universe/Galaxy.hpp"

#include <memory>
#include <CommonStates.h>

class GalaxyTarget : public SandboxTarget
{
public:
    GalaxyTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv);

    void Render() override;
    void RenderTransitionChild(float t) override;
    void RenderTransitionParent(float t) override;

    void MoveObjects(Vector3 v) override;
    void ScaleObjects(float scale) override;
    void ResetObjectPositions() override;

    Vector3 GetClosestObject(Vector3 pos) override;

private:
    void RenderLerp(float t, float scale = 1.0f, Vector3 voffset = Vector3::Zero, bool single = false);
    void BakeSkybox(Vector3 object) override;
    void Seed(uint64_t seed) override;
    void OnStartTransitionDownParent(Vector3 object) override { GenerateSkybox(object); }

    size_t CurrentClosestObjectID;

    RenderView ParticleRenderTarget;

    std::unique_ptr<Galaxy> GalaxyRenderer;
    std::unique_ptr<CSplatting> Splatting;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
};