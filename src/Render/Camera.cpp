#include "Render/Camera.hpp"
#include "Services/Log.hpp"

#include <algorithm>
#include <sstream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

CCamera::CCamera(size_t width, size_t height)
    : Width(width),
      Height(height),
      View(Matrix::CreateTranslation(0.0f, 0.0f, -1000.0f)),
      CameraMode(std::make_unique<ArcballCameraMode>(width, height, View))
{
    Proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(width) / float(height), NearPlane, FarPlane);
}

void CCamera::Update(float dt)
{
    CameraMode->Update(dt);
}

void CCamera::Events(DirectX::Mouse *mouse, DirectX::Mouse::State &ms, float dt)
{
    if ((ms.positionMode == Mouse::MODE_ABSOLUTE && ms.x > 260) || ms.positionMode == Mouse::MODE_RELATIVE)
        CameraMode->Events(mouse, ms, dt);
}

std::string CCamera::ToString()
{
    std::ostringstream ss;
    ss << "(" << View.Translation().x << ", " << View.Translation().y << ", " << View.Translation().z << ")";
    return ss.str();
}

DirectX::XMMATRIX CCamera::GetViewMatrix() const
{
    auto view = View;
    auto inv = view.Invert();
    
    if (AttachedObject)
    {
        inv.Translation(inv.Translation() + AttachedObject->GetPosition());
    }

    return inv.Invert();
}

bool CCamera::PixelFromWorldPoint(Vector3 worldPt, int& x, int& y)
{
    Matrix viewProj = View * Proj;
    Vector3 viewportPt = Vector3::Transform(worldPt, viewProj);

    if(viewportPt.z < 0)
        return false;

    viewportPt.x /= viewportPt.z;
    viewportPt.y /= viewportPt.z;

    x = static_cast<int>((viewportPt.x + 1.0f) * Width  * 0.5f);
    y = static_cast<int>((1.0f - viewportPt.y) * Height * 0.5f);

    return true;
}

Vector3 CCamera::WorldPointFromPixel(int x, int y)
{
    Matrix viewProj = View * Proj;
    Vector4 Q;

    Q.x = static_cast<float>(x) / (static_cast<float>(Width) / 2.0f) - 1.0f;
    Q.y = 1 - static_cast<float>(y) / (static_cast<float>(Height) / 2.0f);
    Q.z = 0.0f;
    Q.w = NearPlane;

    Q.x *= Q.w;
    Q.y *= Q.w;
    Q.z *= Q.w;

    Matrix invViewProj = viewProj.Invert();

    Q = Vector4::Transform(Q, invViewProj);
    return Vector3(Q);
}

void ArcballCameraMode::Update(float dt)
{
    float dx = Radius * cosf(Theta) * sinf(Phi);
    float dy = Radius * cosf(Phi);
    float dz = Radius * sinf(Theta) * sinf(Phi);

    Position = Vector3(dx, -dy, dz);

    View = Matrix::CreateLookAt(Position, Target, Vector3::UnitY);
}

void ArcballCameraMode::Events(DirectX::Mouse* mouse, DirectX::Mouse::State& ms, float dt)
{
    if (ms.positionMode == Mouse::MODE_RELATIVE)
    {
        Theta += static_cast<float>(ms.x) * 0.01f;
        Phi   += static_cast<float>(ms.y) * 0.01f;
    }
    else
    {
        Radius = InitialRadius - ms.scrollWheelValue * 0.4f;
    }

    mouse->SetMode(ms.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
}