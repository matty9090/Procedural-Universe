#include "Model.hpp"
#include "Mesh.hpp"

using DirectX::SimpleMath::Matrix;

CModel::CModel(ID3D11Device* device, CMesh* mesh) : MatrixBuffer(device), Mesh(mesh)
{

}

void CModel::Move(DirectX::SimpleMath::Vector3 v)
{
    Position += v;
    UpdateMatrices();
}

void CModel::SetPosition(DirectX::SimpleMath::Vector3 p)
{
    Position = p;
    UpdateMatrices();
}

void CModel::Rotate(DirectX::SimpleMath::Vector3 r)
{
    Rotation *= DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(r.y, r.x, r.z);
    UpdateMatrices();
}

void CModel::Scale(float s)
{
    RelativeScale *= s;
    UpdateMatrices();
}

void CModel::Draw(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix ViewProj, const RenderPipeline& Pipeline)
{
    MatrixBuffer.SetData(context, { World * ViewProj });

    Pipeline.SetState(context, [&]() {
        unsigned int offset = 0;
        unsigned int stride = sizeof(MeshVertex);

        context->IASetVertexBuffers(0, 1, Mesh->VertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(Mesh->IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->VSSetConstantBuffers(0, 1, MatrixBuffer.GetBuffer());
        context->PSSetShaderResources(0, 1, Mesh->Texture.GetAddressOf());

        context->DrawIndexed(Mesh->NumIndices, 0, 0);
    });
}

void CModel::UpdateMatrices()
{
    World = Matrix::CreateScale(RelativeScale) *
            Matrix::CreateFromQuaternion(Rotation) *
            Matrix::CreateTranslation(Position);
}
