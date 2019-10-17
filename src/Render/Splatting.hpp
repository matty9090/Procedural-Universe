#pragma once

#include <d3d11.h>
#include <memory>
#include <wrl/client.h>
#include <SimpleMath.h>
#include <CommonStates.h>
#include <PostProcess.h>

#include "RenderCommon.hpp"

class Splatting
{
public:
    Splatting(ID3D11DeviceContext* context, int width, int height);

    void Render(unsigned int num, ID3D11ShaderResourceView *scene);

private:
    ID3D11DeviceContext* Context;

    std::unique_ptr<DirectX::CommonStates> States;
    std::unique_ptr<DirectX::DualPostProcess>  DualPostProcess;
    std::unique_ptr<DirectX::BasicPostProcess> BasicPostProcess;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> GeometryShader;

    RenderView Target0;
    RenderView Target1;
};