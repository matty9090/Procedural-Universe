#include "SandboxTarget.hpp"

#include <memory>
#include <CommonStates.h>

class StarTarget : public SandboxTarget
{
public:
    StarTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, CShipCamera* camera, const std::vector<Particle>& seedData);

    void Render() override;
    void RenderTransition(float t) override;
    void MoveObjects(Vector3 v) override;
    void ScaleObjects(float scale) override;
    void ResetObjectPositions() override;

    Vector3 GetClosestObject(Vector3 pos) const override;
    Vector3 GetMainObject() const override;

private:
    void StateIdle() override;
    void CreateParticlePipeline();

    struct GSConstantBuffer
    {
        DirectX::SimpleMath::Matrix ViewProj;
        DirectX::SimpleMath::Matrix InvView;
        DirectX::SimpleMath::Vector3 Translation;
        float Custom;
    };

    Vector3 StarPosition;

    RenderView ParticleRenderTarget;
    RenderPipeline ParticlePipeline;

    std::vector<Particle> Particles;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleBuffer;
};