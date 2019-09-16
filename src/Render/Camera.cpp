#include "Render/Camera.hpp"
#include "Services/Log.hpp"

#include <algorithm>
#include <sstream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera(size_t width, size_t height)
    : m_width(width),
      m_height(height),
      m_cameraPos(0.0f, 0.0f, -1000.0f)
{
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(width) / float(height), m_near, m_far);
}

void Camera::Update(float dt)
{    
    m_view = Matrix::CreateLookAt(m_cameraPos, Vector3::Zero, Vector3::Up);
}

void Camera::Events(DirectX::Mouse *mouse, DirectX::Mouse::State &ms, float dt)
{
    if (ms.positionMode == Mouse::MODE_RELATIVE)
    {
        Vector3 delta = Vector3(float(ms.x), float(ms.y), 0.f) * dt * 0.5f;
        m_cameraPos += delta * dt * 40000.0f;
    }
    else
    {
        m_length = m_initialLength - ms.scrollWheelValue;
    }

    if(m_length < 100.0f) m_length = 100.0f;
    if(m_length > 6000.0f) m_length = 6000.0f;

    m_cameraPos.Normalize();
    m_cameraPos *= m_length;

    if((ms.positionMode == Mouse::MODE_ABSOLUTE && ms.x > 260)
        || ms.positionMode == Mouse::MODE_RELATIVE)
    {
        mouse->SetMode(ms.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
    }
}

std::string Camera::to_string()
{
    std::ostringstream ss;
    ss << "(" << m_cameraPos.x << ", " << m_cameraPos.y << ", " << m_cameraPos.z << ")";
    return ss.str();
}

bool Camera::PixelFromWorldPoint(Vector3 worldPt, int& x, int& y)
{
    Matrix viewProj = m_view * m_proj;
    Vector3 viewportPt = Vector3::Transform(worldPt, viewProj);

    if(viewportPt.z < 0)
        return false;

    viewportPt.x /= viewportPt.z;
    viewportPt.y /= viewportPt.z;

    x = static_cast<int>((viewportPt.x + 1.0f) * m_width  * 0.5f);
    y = static_cast<int>((1.0f - viewportPt.y) * m_height * 0.5f);

    return true;
}

Vector3 Camera::WorldPointFromPixel(int x, int y)
{
    Matrix viewProj = m_view * m_proj;
    Vector4 Q;

    Q.x = static_cast<float>(x) / (static_cast<float>(m_width) / 2.0f) - 1.0f;
    Q.y = 1 - static_cast<float>(y) / (static_cast<float>(m_height) / 2.0f);
    Q.z = 0.0f;
    Q.w = m_near;

    Q.x *= Q.w;
    Q.y *= Q.w;
    Q.z *= Q.w;

    Matrix invViewProj = viewProj.Invert();

    Q = Vector4::Transform(Q, invViewProj);
    return Vector3(Q);
}