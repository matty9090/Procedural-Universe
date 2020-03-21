#include "SandboxTarget.hpp"

#include <memory>
#include <CommonStates.h>

#include "Render/Planet/Planet.hpp"

class StarTarget : public SandboxTarget
{
public:
    StarTarget(ID3D11DeviceContext* context, DX::DeviceResources* resources, ICamera* camera, ID3D11RenderTargetView* rtv);

    void Render() override;
    void RenderObjectUI() override;
    void RenderInChildSpace(const ICamera& cam, float scale = 1.0f) override;
    void RenderTransitionChild(float t) override;
    void RenderTransitionParent(float t) override;

    void MoveObjects(Vector3 v) override;
    void ScaleObjects(float scale) override;
    void ResetObjectPositions() override;

    Vector3 GetClosestObject(Vector3 pos) override;
    size_t GetClosestObjectIndex() const override { return CurrentClosestObjectID; }

    Vector3 GetLightDirection() const;

private:
    void OnStartTransitionDownParent(Vector3 object) override;
    void OnStartTransitionDownChild(Vector3 object) override;
    void OnEndTransitionDownChild() override;
    void RenderLerp(float scale = 1.0f, float t = 1.0f, bool single = false);
    void BakeSkybox(Vector3 object) override;
    void Seed(uint64_t seed) override;
    void CreateStarPipeline();

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

    std::vector<LWParticle> Particles;
    std::vector<std::unique_ptr<CPlanet>> Planets;
    std::vector<CPlanetSeeder> ParticleInfo;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;

    std::unique_ptr<ConstantBuffer<LerpConstantBuffer>> LerpBuffer;
};