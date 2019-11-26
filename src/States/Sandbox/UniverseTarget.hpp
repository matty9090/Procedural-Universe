#include "SandboxTarget.hpp"
#include "Render/Misc/Splatting.hpp"

#include <memory>
#include <CommonStates.h>

class UniverseTarget : public SandboxTarget
{
public:
    UniverseTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv);

    void Render() override;
    void RenderTransitionParent(float t) override;
    void MoveObjects(Vector3 v) override;
    void ScaleObjects(float scale) override;

    Vector3 GetClosestObject(Vector3 pos) override;

private:
    void RenderLerp(float t, bool single = false);
    void Seed(uint64_t seed);
    void BakeSkybox(Vector3 object) override;
    void OnStartTransitionDownParent(Vector3 object) override { GenerateSkybox(object); }
    void CreateParticlePipeline();
    void RegenerateBuffer();

    struct GSConstantBuffer
    {
        DirectX::SimpleMath::Matrix ViewProj;
        DirectX::SimpleMath::Matrix InvView;
        Vector3 Translation;
        float Custom;
    };

    struct LerpConstantBuffer
    {
        float Alpha;
        float Custom1, Custom2, Custom3;
    };

    size_t CurrentClosestObjectID;

    RenderView ParticleRenderTarget;
    RenderPipeline ParticlePipeline;

    std::vector<Particle> Particles;
    std::unique_ptr<CSplatting> Splatting;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleBuffer;

    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;
    std::unique_ptr<ConstantBuffer<LerpConstantBuffer>> LerpBuffer;
};