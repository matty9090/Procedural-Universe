#include "SandboxTarget.hpp"

#include <memory>
#include <CommonStates.h>

class GalaxyTarget : public SandboxTarget
{
public:
    GalaxyTarget(ID3D11DeviceContext* context, std::string name, DX::DeviceResources* resources, CShipCamera* camera, const std::vector<Particle>& seedData);

    void Render() override;
    Vector3 GetClosestObject(Vector3 pos) const override;

private:
    void StateIdle() override;
    void CreateParticlePipeline();

    struct GSConstantBuffer
    {
        DirectX::SimpleMath::Matrix ViewProj;
        DirectX::SimpleMath::Matrix InvView;
    };

    RenderView ParticleRenderTarget;
    RenderPipeline ParticlePipeline;

    std::vector<Particle> Particles;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleBuffer;
};