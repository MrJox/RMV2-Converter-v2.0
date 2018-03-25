#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera() :
	m_pos(0.0f, 0.0f, 3.0f),
	m_target(0.0f, 1.0f, 0.0f),

	m_width(800),
	m_height(600),

	m_fov(XM_PIDIV4),
	m_nearZ(0.1f),
	m_farZ(1000.0f),

	m_radius(3.5f),
	m_theta(XM_PIDIV2),
	m_phi(XM_PIDIV2)
{
	SetProj(m_fov, m_width, m_height - 20, m_nearZ, m_farZ);
}

Camera::~Camera()
{
}

void Camera::SetLookAt(FXMVECTOR& eyePosition, FXMVECTOR& target, FXMVECTOR& up)
{
	XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(eyePosition, target, up));
}

void Camera::SetProj(float fov, uint32_t width, uint32_t height, float nearZ, float farZ)
{
	m_fov = fov;
	m_width = width;
	m_height = height;
	m_nearZ = nearZ;
	m_farZ = farZ;

	XMStoreFloat4x4(&m_proj, XMMatrixPerspectiveFovLH(m_fov, static_cast<float>(m_width) / static_cast<float>(m_height), m_nearZ, m_farZ));
}

void Camera::SetPosition(float x, float y, float z)
{
	m_pos = XMFLOAT3(x, y, z);
}

void Camera::SetTarget(float x, float y, float z)
{
	m_target = XMFLOAT3(x, y, z);
}

void Camera::Update()
{
	SetPosition(m_target.x + m_radius * sinf(m_phi) * cosf(m_theta), m_target.y + m_radius * cosf(m_phi), m_target.z + m_radius * sinf(m_phi) * sinf(m_theta));
	SetLookAt(XMLoadFloat3(&m_pos), XMLoadFloat3(&m_target), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

void Camera::Move(float d)
{
	m_target.y += d;
}

void Camera::Zoom(float d)
{
	m_radius += d;
	m_radius > 50.0f ? m_radius = 50.0f : m_radius < m_nearZ + 0.5f ? m_radius = m_nearZ + 0.5f : m_radius;
}

void Camera::Rotate(float angleX, float angleY)
{
	m_theta += angleX;
	m_phi += angleY;

	m_phi < 0.01f ? m_phi = 0.01f : (m_phi > XM_PI - 0.01f) ? m_phi = XM_PI - 0.01f : m_phi;
}

void Camera::ResetPosition()
{
	m_radius = 3.5f;
	m_theta = XM_PIDIV2;
	m_phi = XM_PIDIV2;
	m_target = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

XMMATRIX Camera::GetView()const
{
	return XMLoadFloat4x4(&m_view);
}

XMMATRIX Camera::GetProj()const
{
	return XMLoadFloat4x4(&m_proj);
}

XMFLOAT3 Camera::GetPosition()const
{
	return m_pos;
}

XMVECTOR Camera::GetPositionV()const
{
	return XMLoadFloat3(&m_pos);
}

XMFLOAT3 Camera::GetTarget()const
{
	return m_target;
}

XMVECTOR Camera::GetTargetV()const
{
	return XMLoadFloat3(&m_target);
}
