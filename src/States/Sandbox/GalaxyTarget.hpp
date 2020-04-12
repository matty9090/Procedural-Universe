#include "SandboxTarget.hpp"
#include "Render/Misc/Splatting.hpp"
#include "Render/Universe/Galaxy.hpp"

#include <memory>
#include <CommonStates.h>

class GalaxyTarget : public SandboxTarget
{
public:
    GalaxyTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv);

    void Render() override;
    void RenderUI() override;
    void RenderInChildSpace(const ICamera& cam, float scale = 1.0f) override;
    void RenderTransitionChild(float t) override;
    void RenderTransitionParent(float t) override;

    void MoveObjects(Vector3 v) override;
    void ScaleObjects(float scale) override;
    void ResetObjectPositions() override;

    std::string GetObjectName() const override;
    Vector3 GetClosestObject(Vector3 pos) override;
    size_t GetClosestObjectIndex() const override { return GalaxyRenderer->GetClosestObjectIndex(); }
    LWParticle GetParticle(size_t index) const { return GalaxyRenderer->GetParticle(index); }

private:
    void OnStartTransitionDownParent(Vector3 object) override { GenerateSkybox(object); }
    void OnStartTransitionDownChild(Vector3 location) override;
    void OnEndTransitionDownChild() override;

    void RenderLerp(float t = 1.0f, float scale = 1.0f, Vector3 voffset = Vector3::Zero, bool single = false);
    void BakeSkybox(Vector3 object) override;
    void Seed(uint64_t seed) override;

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

    std::vector<LWParticle> SeedParticles;
    std::vector<BillboardInstance> SeedDustClouds;

    std::unique_ptr<Galaxy> GalaxyRenderer;
    std::unique_ptr<CSplatting> Splatting;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleBuffer;

    std::unique_ptr<ConstantBuffer<GSConstantBuffer>> GSBuffer;
    std::unique_ptr<ConstantBuffer<LerpConstantBuffer>> LerpBuffer;
};