#pragma once

#include <string>
#include <d3d11.h>

#include <Mouse.h>
#include <SimpleMath.h>

#include "Camera.hpp"
#include "Render/Model/Ship.hpp"
#include "Render/Misc/Particle.hpp"

using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

/**
 * @brief Camera class
 * 
 */
class CSandboxCamera : public ICamera
{
    public:
        /**
         * @brief Construct a new Camera object
         * 
         * @param width 
         * @param size_t 
         */
        CSandboxCamera(size_t width = 0, size_t height = 0);

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
        void Events(DirectX::Mouse *mouse, DirectX::Mouse::State &ms, DirectX::Keyboard::State kb, float dt);

        /**
         * @brief Set the position of the camera
         * 
         * @param p 
         */
        void SetPosition(Vector3 p) override;

        /**
         * @brief Move the camera
         *
         * @param p
         */
        void Move(Vector3 v) override;

        /**
         * @brief Get the view matrix
         * 
         * @return DirectX::XMMATRIX 
         */
        DirectX::XMMATRIX GetViewMatrix() const override;

        /**
         * @brief Get the projection matrix
         * 
         * @return DirectX::XMMATRIX 
         */
        DirectX::XMMATRIX GetProjectionMatrix() const override { return Proj; }

        /**
         * @brief Get the width and height of the viewport
         *
         * @return DirectX::SimpleMath::Vector2
         */
        DirectX::SimpleMath::Vector2 GetSize() const override;

        /**
         * @brief Get the position of the camera
         * 
         * @return Vector3 
         */
        Vector3 GetPosition() const override { return Position; }

        /**
        * @brief Get the forward vector of the camera
        *
        * @return Vector3
        */
        Vector3 GetForward() const { return View.Invert().Forward(); }

        /**
        * @brief Get the forward vector of the camera
        *
        * @return Vector3
        */
        Vector3 GetUp() const { return View.Invert().Up(); }

        /**
        * @brief Get the forward vector of the camera
        *
        * @return Vector3
        */
        Vector3 GetRight() const { return View.Invert().Right(); }

        /**
         * @brief Get the x and y pixel coordinates for the given world point
         * 
         * @return bool 
         */
        bool PixelFromWorldPoint(Vector3 worldPt, int& x, int& y);

        /**
         * @brief Get the world coordinates of a point on the near clip plane from a screen position
         * 
         * @return Vector3 
         */
        Vector3 WorldPointFromPixel(int x, int y);

        /**
         * @brief Get the current speed of the camera
         *
         * @return float
         */
        float GetSpeed() const { return Speed * VelocityScale; }

        /**
         * @brief Get the current speed of the camera
         *
         * @return void
         */
        void SetEnableInput(bool enable) { EnableInput = enable; }

        /**
         * @brief Set the camera matrix
         *
         * @return void
         */
        void SetMatrix(Matrix matrix) { View = matrix; }

        /**
         * @brief Return camera position as a string
         * 
         * @return std::string 
         */
        std::string ToString();

        size_t GetWidth() const { return Width; }
        size_t GetHeight() const { return Height; }

        float VelocityScale = 1.0f;
        float Yaw = 0.0f, Pitch = 0.0f, Roll = 0.0f;

    private:
        Matrix View, Proj;
        size_t Width, Height;

        bool EnableInput = true;
        float NearPlane = 0.01f, FarPlane = 2000.0f;
        float InitialSpeed = 10000.0f, Speed;

        Vector3 Position;
};