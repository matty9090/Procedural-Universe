#pragma once

#include <string>
#include <d3d11.h>

#include <Mouse.h>
#include <SimpleMath.h>

/**
 * @brief Camera class
 * 
 */
class Camera
{
    public:
        /**
         * @brief Construct a new Camera object
         * 
         * @param width 
         * @param size_t 
         */
        Camera(size_t width = 0, size_t size_t = 0);

        /**
         * @brief Frame update
         * 
         * @param dt 
         */
        void Update(float dt);

        /**
         * @brief Handle events
         * 
         * @param mouse DirectXTK mouse
         * @param ms DirectXTK mouse state
         * @param dt 
         */
        void Events(DirectX::Mouse *mouse, DirectX::Mouse::State &ms, float dt);

        /**
         * @brief Move the camera
         * 
         * @param v 
         */
        void Move(DirectX::SimpleMath::Vector3 v) { m_cameraPos += v; }

        /**
         * @brief Rotate the camera
         * 
         * @param r 
         */
        void Rotate(DirectX::SimpleMath::Vector3 r) { m_yaw += r.y; m_pitch += r.z; }

        /**
         * @brief Set the rotation of the camera
         * 
         * @param r 
         */
        void SetRotation(DirectX::SimpleMath::Vector3 r) { m_yaw = r.y; m_pitch = r.z; }

        /**
         * @brief Set the position of the camera
         * 
         * @param p 
         */
        void SetPosition(DirectX::SimpleMath::Vector3 p) { m_cameraPos = p; }

        /**
         * @brief Get the view matrix
         * 
         * @return DirectX::XMMATRIX 
         */
        DirectX::XMMATRIX GetViewMatrix() const { return m_view; }

        /**
         * @brief Get the projection matrix
         * 
         * @return DirectX::XMMATRIX 
         */
        DirectX::XMMATRIX GetProjectionMatrix() const { return m_proj; }

        /**
         * @brief Get the position of the camera
         * 
         * @return DirectX::SimpleMath::Vector3 
         */
        DirectX::SimpleMath::Vector3 GetPosition() const { return m_cameraPos; }

        /**
         * @brief Get the rotation as a quaternion
         * 
         * @return DirectX::SimpleMath::Quaternion 
         */
        DirectX::SimpleMath::Quaternion GetQuaternion() const;

        /**
         * @brief Return camera position as a string
         * 
         * @return std::string 
         */
        std::string to_string();

    private:
        size_t m_width, m_height;
        float m_pitch, m_yaw;

        DirectX::SimpleMath::Matrix m_view, m_proj;
        DirectX::SimpleMath::Vector3 m_cameraPos;
};