#include "SandboxTarget.hpp"

#include <memory>
#include <CommonStates.h>

class StarTarget : public SandboxTarget
{
public:
    StarTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv);

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
    void Seed(uint64_t seed) override;
    void UpdateParticleBuffer();

    void CreateStarPipeline();
    void CreateParticlePipeline();

    struct GSConstantBuffer
    {
        DirectX::SimpleMath::Matrix ViewProj;
        DirectX::SimpleMath::Matrix InvView;
        DirectX::SimpleMath::Vector3 Translation;
        float Custom;
    };

    struct LerpConstantBuffer
    {
        float Alpha;
        float Custom1, Custom2, Custom3;
    };

    size_t CurrentClosestObjectID;
    std::unique_ptr<CModel> Star;

    RenderView ParticleRenderTarget;
    RenderPipeline StarPipeline;
    RenderPipeline ParticlePipeline;

    std::vector<Particle> Particles;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleBuffer;

    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;
    std::unique_ptr<ConstantBuffer<LerpConstantBuffer>> LerpBuffer;
};