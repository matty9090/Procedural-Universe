#include "Render/Camera.hpp"
#include "Services/Log.hpp"

#include <algorithm>
#include <sstream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera(size_t width, size_t height)
    : m_width(width),
      m_height(height),
      m_cameraPos(0.0f, 0.0f, -1000.0f),
      m_yaw(0.0f),
      m_pitch(0.0f)
{
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(width) / float(height), 1.0f, 10000.f);
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

    mouse->SetMode(ms.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
}

DirectX::SimpleMath::Quaternion Camera::GetQuaternion() const
{
    return Quaternion::CreateFromYawPitchRoll(m_yaw, -m_pitch, 0.0f);
}

std::string Camera::to_string()
{
    std::ostringstream ss;
    ss << "(" << m_cameraPos.x << ", " << m_cameraPos.y << ", " << m_cameraPos.z << ")";
    return ss.str();
}
