#pragma once

#include <string>
#include <d3d11.h>

#include <Mouse.h>
#include <SimpleMath.h>

#include "Camera.hpp"
#include "Render/Model/Model.hpp"
#include "Render/Misc/Particle.hpp"

using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

/**
 * @brief Camera class
 * 
 */
class CArcballCamera : public ICamera
{
    public:
        /**
         * @brief Construct a new Camera object
         * 
         * @param width 
         * @param size_t 
         */
        CArcballCamera(unsigned int width = 0, unsigned int height = 0);

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
         * @brief Set the position of the camera
         * 
         * @param p 
         */
        void SetPosition(Vector3 p);

        /**
         * @brief Track a particle
         * 
         * @param p 
         */
        void Track(Particle* p) { TrackedParticle = p; }

        /**
         * @brief Attach to a model
         *
         * @param p
         */
        void Attach(CModel* obj) { AttachedObject = obj; }

        /**
         * @brief Get the view matrix
         * 
         * @return DirectX::XMMATRIX 
         */
        DirectX::XMMATRIX GetViewMatrix() const;

        /**
         * @brief Get the projection matrix
         * 
         * @return DirectX::XMMATRIX 
         */
        DirectX::XMMATRIX GetProjectionMatrix() const { return Proj; }

        /**
         * @brief Get the position of the camera
         * 
         * @return Vector3 
         */
        Vector3 GetPosition() const;

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
         * @brief Return camera position as a string
         * 
         * @return std::string 
         */
        std::string ToString();

        size_t GetWidth() const { return Width; }
        size_t GetHeight() const { return Height; }

    private:
        Matrix View, Proj;
        unsigned int Width, Height;
        float NearPlane = 10.0f, FarPlane = 30000.0f;

        Particle* TrackedParticle = nullptr;
        CModel* AttachedObject = nullptr;

        float Radius, InitialRadius;
        float Theta = 4.6f, Phi = 1.6f;
        Vector3 Position, Target;
};