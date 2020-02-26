#include "SandboxTarget.hpp"
#include "Render/Planet/Planet.hpp"

#include <memory>
#include <CommonStates.h>

class PlanetTarget : public SandboxTarget
{
public:
    PlanetTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv);

    void Render() override;
    void RenderUI() override;
    void RenderTransitionChild(float t) override;
    void RenderTransitionParent(float t) override;

    void MoveObjects(Vector3 v) override;
    void ScaleObjects(float scale) override;
    void ResetObjectPositions() override;

    Vector3 GetClosestObject(Vector3 pos) override;

private:
    void OnStartTransitionDownParent(Vector3 object) override { GenerateSkybox(object); }
    void OnStartTransitionDownChild(Vector3 location) override;
    void OnEndTransitionDownChild() override { /*Planet->SetScale(1.0f / Scale);*/ }
    void CreatePlanetPipeline();
    void StateIdle(float dt) override;

    struct PlanetConstantBuffer
    {
        Vector3 LightDir;
        float Lerp;
    };

    size_t CurrentClosestObjectID;
    std::unique_ptr<CPlanet> Planet;

    RenderView ParticleRenderTarget;
    RenderPipeline StarPipeline;

    std::vector<Particle> Particles;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    std::unique_ptr<ConstantBuffer<PlanetConstantBuffer>> PlanetBuffer;
};