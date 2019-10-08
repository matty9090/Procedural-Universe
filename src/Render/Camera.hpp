#pragma once

#include <string>
#include <d3d11.h>

#include <Mouse.h>
#include <SimpleMath.h>

#include "Render/Particle.hpp"

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
         * @brief Set the position of the camera
         * 
         * @param p 
         */
        void SetPosition(DirectX::SimpleMath::Vector3 p) { m_cameraPos = p; }

        /**
         * @brief Track a particle
         * 
         * @param p 
         */
        void Track(Particle* p) { m_tracked = p; }

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
         * @brief Get the x and y pixel coordinates for the given world point
         * 
         * @return bool 
         */
        bool PixelFromWorldPoint(DirectX::SimpleMath::Vector3 worldPt, int& x, int& y);

        /**
         * @brief Get the world coordinates of a point on the near clip plane from a screen position
         * 
         * @return DirectX::SimpleMath::Vector3 
         */
        DirectX::SimpleMath::Vector3 WorldPointFromPixel(int x, int y);

        /**
         * @brief Return camera position as a string
         * 
         * @return std::string 
         */
        std::string to_string();

    private:
        size_t m_width, m_height;
        float m_initialLength = 1000.0f;
        float m_length = m_initialLength;
        float m_near = 1.0f, m_far = 10000.0f;

        DirectX::SimpleMath::Matrix m_view, m_proj;
        DirectX::SimpleMath::Vector3 m_cameraPos;

        Particle* m_tracked = nullptr;
        DirectX::SimpleMath::Vector3 m_lastPos;
};