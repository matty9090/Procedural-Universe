#include "SandboxCamera.hpp"
#include "Services/Log.hpp"
#include "Core/Maths.hpp"

#include <algorithm>
#include <sstream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

CSandboxCamera::CSandboxCamera(size_t width, size_t height)
    : Width(width),
      Height(height),
      Speed(InitialSpeed)
{
    Proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(width) / float(height), NearPlane, FarPlane);
}

void CSandboxCamera::Update(float dt)
{
    float y = sinf(Pitch);
    float r = cosf(Pitch);
    float z = r * cosf(Yaw);
    float x = r * sinf(Yaw);

    View = Matrix::CreateLookAt(Position, Position + Vector3(x, y, z), Vector3::Up);
}

void CSandboxCamera::Events(DirectX::Mouse *mouse, DirectX::Mouse::State &ms, DirectX::Keyboard::State kb, float dt)
{
    if (ms.positionMode == Mouse::MODE_RELATIVE)
    {
        Vector3 delta = Vector3(static_cast<float>(ms.x), static_cast<float>(ms.y), 0.0f) * 0.007f;
        
        Pitch -= delta.y;
        Yaw -= delta.x;

        float limit = XM_PI / 2.0f - 0.01f;
        Pitch = (std::max)(-limit, Pitch);
        Pitch = (std::min)(+limit, Pitch);

        if (Yaw > XM_PI)
            Yaw -= XM_PI * 2.0f;
        else if (Yaw < -XM_PI)
            Yaw += XM_PI * 2.0f;
    }
    else
    {
        double t = -1.0f * ((static_cast<double>(-ms.scrollWheelValue) / 8000.0) - 1.0f);
        t *= t * t * t;

        Speed = (std::max)(InitialSpeed * static_cast<float>(t), 0.0f);
    }

    mouse->SetMode(ms.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);

    Vector3 move = Vector3::Zero;

    if (kb.W) move.z += 1.0f;
    if (kb.S) move.z -= 1.0f;
    if (kb.A) move.x += 1.0f;
    if (kb.D) move.x -= 1.0f;
    if (kb.Q) Roll   -= 2.0f * dt;
    if (kb.E) Roll   += 2.0f * dt;

    Quaternion q = Quaternion::CreateFromYawPitchRoll(Yaw, -Pitch, 0.0f);
    move = Vector3::Transform(move, q);
    move *= dt * Speed * VelocityScale;
    
    Position += move;
}

void CSandboxCamera::SetPosition(Vector3 p)
{
    Position = p;
}

void CSandboxCamera::Move(Vector3 v)
{
    Position += v;
}

std::string CSandboxCamera::ToString()
{
    auto v = View.Invert();
    std::ostringstream ss;
    ss << "(" << v.Translation().x << ", " << v.Translation().y << ", " << v.Translation().z << ")";
    return ss.str();
}

DirectX::XMMATRIX CSandboxCamera::GetViewMatrix() const
{
    return View;
}

bool CSandboxCamera::PixelFromWorldPoint(Vector3 worldPt, int& x, int& y)
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

Vector3 CSandboxCamera::WorldPointFromPixel(int x, int y)
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