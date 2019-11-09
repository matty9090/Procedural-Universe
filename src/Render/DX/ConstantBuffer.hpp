#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXHelpers.h>

#include "Core/Except.hpp"

/**
 * @brief DirectX constant buffer helper
 * 
 * @tparam T data type to pass to shader
 */
template <class T>
class ConstantBuffer
{
    public:
        /**
         * @brief Construct a new Constant Buffer object
         * 
         * @param device 
         */
        ConstantBuffer(ID3D11Device *device);

        /**
         * @brief Set the data of the buffer
         * 
         * @param deviceContext 
         * @param data 
         */
        void SetData(ID3D11DeviceContext *deviceContext, const T &data);

        /**
         * @brief Get the DirectX buffer
         * 
         * @return ID3D11Buffer** 
         */
        ID3D11Buffer **GetBuffer() { return m_buffer.GetAddressOf(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
};

template<class T>
inline ConstantBuffer<T>::ConstantBuffer(ID3D11Device *device)
{
    D3D11_BUFFER_DESC desc = {};

    desc.ByteWidth = sizeof(T);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    DX::ThrowIfFailed(device->CreateBuffer(&desc, nullptr, m_buffer.ReleaseAndGetAddressOf()));
}

template<class T>
inline void ConstantBuffer<T>::SetData(ID3D11DeviceContext *deviceContext, const T &data)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    DX::ThrowIfFailed(deviceContext->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    *static_cast<T*>(mappedResource.pData) = data;

    deviceContext->Unmap(m_buffer.Get(), 0);
}