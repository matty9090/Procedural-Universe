#include "Render/ArcballCamera.hpp"
#include "Services/Log.hpp"

#include <algorithm>
#include <sstream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

CArcballCamera::CArcballCamera(size_t width, size_t height)
    : Width(width),
      Height(height),
      View(Matrix::CreateTranslation(0.0f, 0.0f, -1000.0f)),
      Radius(View.Invert().Translation().Length()),
      InitialRadius(View.Invert().Translation().Length()),
      Position(View.Invert().Translation())
{
    Proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(width) / float(height), NearPlane, FarPlane);
}

void CArcballCamera::Update(float dt)
{
    float dx = Radius * cosf(Theta) * sinf(Phi);
    float dy = Radius * cosf(Phi);
    float dz = Radius * sinf(Theta) * sinf(Phi);

    Position = Vector3(dx, -dy, dz);

    View = Matrix::CreateLookAt(Position, Target, Vector3::UnitY);
}

void CArcballCamera::Events(DirectX::Mouse *mouse, DirectX::Mouse::State &ms, float dt)
{
    if ((ms.positionMode == Mouse::MODE_ABSOLUTE && ms.x > 260) || ms.positionMode == Mouse::MODE_RELATIVE)
    {
        if (ms.positionMode == Mouse::MODE_RELATIVE)
        {
            Theta += static_cast<float>(ms.x) * 0.01f;
            Phi += static_cast<float>(ms.y) * 0.01f;
        }
        else
        {
            Radius = InitialRadius - ms.scrollWheelValue * 0.4f;
        }

        mouse->SetMode(ms.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
    }
}

void CArcballCamera::SetPosition(Vector3 p)
{
    auto v = View.Invert();
    v.Translation(p);
    View = v.Invert();
    InitialRadius = Radius = p.Length();
}

std::string CArcballCamera::ToString()
{
    auto v = View.Invert();
    std::ostringstream ss;
    ss << "(" << v.Translation().x << ", " << v.Translation().y << ", " << v.Translation().z << ")";
    return ss.str();
}

DirectX::XMMATRIX CArcballCamera::GetViewMatrix() const
{
    if (AttachedObject)
    {
        auto inv = View.Invert();
        inv.Translation(inv.Translation() + AttachedObject->GetPosition());
        return inv.Invert();
    }

    return View;
}

bool CArcballCamera::PixelFromWorldPoint(Vector3 worldPt, int& x, int& y)
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

Vector3 CArcballCamera::WorldPointFromPixel(int x, int y)
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