#include "SandboxTarget.hpp"
#include "Render/Misc/Splatting.hpp"
#include "Render/Universe/Galaxy.hpp"

#include <memory>
#include <CommonStates.h>

class UniverseTarget : public SandboxTarget
{
public:
    UniverseTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv);

    void Seed(uint64_t seed) override;
    void Render() override;
    void RenderTransitionParent(float t) override;
    void MoveObjects(Vector3 v) override;
    void ScaleObjects(float scale) override;

    Vector3 GetClosestObject(Vector3 pos) override;

private:
    void RenderLerp(float t, bool single = false);
    void BakeSkybox(Vector3 object) override;
    void OnStartTransitionDownParent(Vector3 object) override { GenerateSkybox(object); }

    size_t CurrentClosestObjectID;
    Vector3 UniversePosition;
    RenderView ParticleRenderTarget;

    std::unique_ptr<CSplatting> Splatting;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    
    std::vector<std::unique_ptr<Galaxy>> Galaxies;
};