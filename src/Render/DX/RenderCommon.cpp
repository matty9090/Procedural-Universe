#include "RenderCommon.hpp"
#include "Render/DX/Shader.hpp"

#include "Services/ResourceManager.hpp"

RenderView CreateTarget(ID3D11Device* device, int width, int height)
{
    RenderView t;
    t.Width = width;
    t.Height = height;

    CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height,
                               1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
                               D3D11_USAGE_DEFAULT, 0, 1, 0);

    CD3D11_TEXTURE2D_DESC dsDesc(DXGI_FORMAT_D32_FLOAT, width, height,
                                 1, 1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, 1, 0 );

    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D);
    CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D);

    DX::ThrowIfFailed(device->CreateTexture2D(&desc, nullptr, t.Rt.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateTexture2D(&dsDesc, nullptr, t.Ds.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateShaderResourceView(t.Rt.Get(), &srvDesc, t.Srv.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateRenderTargetView(t.Rt.Get(), &rtvDesc, t.Rtv.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateDepthStencilView(t.Ds.Get(), &dsvDesc, t.Dsv.ReleaseAndGetAddressOf()));

    return t;
}

void SetRenderTarget(ID3D11DeviceContext* context, RenderView& view)
{
    context->OMSetRenderTargets(1, view.Rtv.GetAddressOf(), view.Dsv.Get());
}

void CreateParticleBuffer(ID3D11Device* device, ID3D11Buffer** buffer, const std::vector<Particle>& particles)
{
    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = static_cast<unsigned int>(particles.size()) * sizeof(Particle);
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA init;
    init.pSysMem = &particles[0];

    DX::ThrowIfFailed(device->CreateBuffer(&desc, &init, buffer));
}

void CreateParticleBuffer(ID3D11Device* device, ID3D11Buffer** buffer, unsigned int size)
{
    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = size * sizeof(Particle);
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;

    DX::ThrowIfFailed(device->CreateBuffer(&desc, nullptr, buffer));
}

std::vector<D3D11_INPUT_ELEMENT_DESC> CreateInputLayoutPosition()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    return layout;
}

std::vector<D3D11_INPUT_ELEMENT_DESC> CreateInputLayoutPositionColour()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    return layout;
}

std::vector<D3D11_INPUT_ELEMENT_DESC> CreateInputLayoutPositionTexture()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    return layout;
}

std::vector<D3D11_INPUT_ELEMENT_DESC> CreateInputLayoutPositionNormalColour()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    return layout;
}

std::vector<D3D11_INPUT_ELEMENT_DESC> CreateInputLayoutPositionNormalTexture()
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    return layout;
}

void RenderView::Clear(ID3D11DeviceContext* context)
{
    context->ClearRenderTargetView(Rtv.Get(), DirectX::Colors::Black);
    context->ClearDepthStencilView(Dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RenderPipeline::LoadVertex(std::wstring file)
{
    VertexShader = RESM.GetVertexShader(file);
    VertexCode = RESM.GetVertexCode(file);
}

void RenderPipeline::LoadPixel(std::wstring file)
{
    PixelShader = RESM.GetPixelShader(file);
}

void RenderPipeline::LoadGeometry(std::wstring file)
{
    GeometryShader = RESM.GetGeometryShader(file);
}

void RenderPipeline::CreateRasteriser(ID3D11Device* device, ECullMode cullMode)
{
    D3D11_CULL_MODE cull = D3D11_CULL_NONE;

    if(cullMode == ECullMode::Clockwise) cull = D3D11_CULL_FRONT;
    if(cullMode == ECullMode::Anticlockwise) cull = D3D11_CULL_BACK;

    CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, cull, FALSE,
        D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, FALSE, TRUE);

    DX::ThrowIfFailed(device->CreateRasterizerState(&rastDesc, Raster.ReleaseAndGetAddressOf()));
}

void RenderPipeline::CreateInputLayout(ID3D11Device* device, std::vector<D3D11_INPUT_ELEMENT_DESC> layout)
{
    DX::ThrowIfFailed(
        device->CreateInputLayout(
            layout.data(),
            static_cast<unsigned int>(layout.size()),
            VertexCode->GetBufferPointer(),
            VertexCode->GetBufferSize(),
            InputLayout.ReleaseAndGetAddressOf()
        )
    );
}

void RenderPipeline::SetState(ID3D11DeviceContext* context, std::function<void()> state) const
{
    context->IASetInputLayout(InputLayout.Get());
    context->IASetPrimitiveTopology(Topology);
    context->VSSetShader(VertexShader, 0, 0);
    context->GSSetShader(GeometryShader, 0, 0);
    context->PSSetShader(PixelShader, 0, 0);
    context->RSSetState(Raster.Get());

    state();
}
