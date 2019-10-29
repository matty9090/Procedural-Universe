#pragma once

#include <string>
#include <d3d11.h>

#include <Mouse.h>
#include <SimpleMath.h>

#include "Render/Particle.hpp"
#include "Render/Model.hpp"

using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

class ICameraMode
{
public:
    virtual void Update(float dt) = 0;
    virtual void Events(DirectX::Mouse* mouse, DirectX::Mouse::State& ms, float dt) = 0;
};

class ArcballCameraMode : public ICameraMode
{
public:
    ArcballCameraMode(size_t width, size_t height, Matrix& view)
        : Width(width), Height(height), View(view), Radius(1400.0f),
          InitialRadius(1400.0f), Position(0.0f, 0.0f, -1400.0f)
    {}

    void Update(float dt);
    void Events(DirectX::Mouse* mouse, DirectX::Mouse::State& ms, float dt);

private:
    float Radius, InitialRadius;
    float Theta = 4.6f, Phi = 1.6f;
    size_t Width, Height;
    Matrix& View;
    Vector3 Position, Target;
};

/**
 * @brief Camera class
 * 
 */
class CCamera
{
    public:
        enum class Mode { Arcball, Roam };

        /**
         * @brief Construct a new Camera object
         * 
         * @param width 
         * @param size_t 
         */
        CCamera(size_t width = 0, size_t height = 0);

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
        void SetPosition(Vector3 p) { View(3, 0) = p.x, View(3, 1) = p.y, View(3, 2) = p.z; }

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
        Vector3 GetPosition() const { return View.Translation(); }

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
        std::unique_ptr<ICameraMode> CameraMode;

        Matrix View, Proj;
        size_t Width, Height;
        float NearPlane = 10.0f, FarPlane = 30000.0f;

        Particle* TrackedParticle = nullptr;
        CModel* AttachedObject = nullptr;
};