#include "SandboxTarget.hpp"

#include <deque>
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
    void CreateOrbitPipeline();
    void StateTransitioning(float dt) override;

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

    struct Vertex
    {
        DirectX::SimpleMath::Vector3 Position;
        DirectX::SimpleMath::Vector3 Normal;
        DirectX::SimpleMath::Vector2 UV;
    };

    struct Orbit
    {
        float Radius;
        DirectX::SimpleMath::Color Colour;
        DirectX::SimpleMath::Matrix Orientation;
        Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
    };

    int SeedFrames = 0;
    size_t CurrentClosestObjectID;

    std::unique_ptr<CModel> Star;
    std::vector<Orbit> Orbits;

    RenderView ParticleRenderTarget;
    RenderPipeline StarPipeline;
    RenderPipeline OrbitPipeline;

    std::deque<uint64_t> SeedQueue;
    std::vector<LWParticle> Particles;
    std::vector<std::unique_ptr<CPlanet>> Planets;
    std::vector<CPlanetSeeder> ParticleInfo;
    std::unique_ptr<CPostProcess> PostProcess;
    std::unique_ptr<DirectX::CommonStates> CommonStates;
    std::unique_ptr<ConstantBuffer<LerpConstantBuffer>> LerpBuffer;
    
    struct OrbitVSBuffer
    {
        DirectX::SimpleMath::Matrix WorldViewProj;
        DirectX::SimpleMath::Matrix World;
    };

    struct OrbitPSBuffer
    {
        DirectX::SimpleMath::Color Colour;
    };

    UINT NumOrbitIndices;
    float OrbitThickness = 1.0f;

    Microsoft::WRL::ComPtr<ID3D11Buffer> OrbitIB;

    std::unique_ptr<ConstantBuffer<OrbitVSBuffer>> OrbitVCB;
    std::unique_ptr<ConstantBuffer<OrbitPSBuffer>> OrbitPCB;
};