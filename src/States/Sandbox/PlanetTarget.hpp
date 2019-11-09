#include "SandboxTarget.hpp"

#include <memory>
#include <CommonStates.h>

class PlanetTarget : public SandboxTarget
{
public:
    PlanetTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv, const std::vector<Particle>& seedData);

    void Render() override;
    void RenderTransitionChild(float t) override;
    void RenderTransitionParent(float t) override;

    void MoveObjects(Vector3 v) override;
    void ScaleObjects(float scale) override;
    void ResetObjectPositions() override;

    Vector3 GetClosestObject(Vector3 pos) override;

private:
    void OnStartTransitionDownParent(Vector3 object) override { GenerateSkybox(object); }
    void RenderLerp(float scale = 1.0f, Vector3 offset = Vector3::Zero, float t = 1.0f, bool single = false);
    void BakeSkybox(Vector3 object) override;
    void CreatePlanetPipeline();

    struct LerpConstantBuffer
    {
        float Alpha;
        float Custom1, Custom2, Custom3;
    };

    size_t CurrentClosestObjectID;
    std::unique_ptr<CModel> Planet;

    RenderView ParticleRenderTarget;
    RenderPipeline StarPipeline;

    std::vector<Particle> Particles;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    std::unique_ptr<ConstantBuffer<LerpConstantBuffer>> LerpBuffer;
};