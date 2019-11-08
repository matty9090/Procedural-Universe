#include "SandboxTarget.hpp"

#include <memory>
#include <CommonStates.h>

class GalaxyTarget : public SandboxTarget
{
public:
    GalaxyTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, ID3D11RenderTargetView* rtv, const std::vector<Particle>& seedData);

    void Render() override;
    void RenderTransitionParent(float t) override;
    void MoveObjects(Vector3 v) override;

    Vector3 GetClosestObject(Vector3 pos) override;
    Vector3 GetMainObject() const override;

private:
    void RenderLerp(float t);
    void BakeSkybox(Vector3 object) override;

    void StateIdle() override;
    void CreateParticlePipeline();

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
    Vector3 GalaxyPosition;

    RenderView ParticleRenderTarget;
    RenderPipeline ParticlePipeline;

    std::vector<Particle> Particles;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleBuffer;

    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;
    std::unique_ptr<ConstantBuffer<LerpConstantBuffer>> LerpBuffer;
};