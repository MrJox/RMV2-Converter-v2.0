#pragma once
#ifndef CAMERA_H
#define CAMERA_H
using namespace DirectX;

class Camera
{
public:

	Camera();
	~Camera();

	void SetLookAt(FXMVECTOR& eyePosition, FXMVECTOR& target, FXMVECTOR& up);
	void SetProj(float fov, uint32_t width, uint32_t height, float nearZ, float farZ);
	void SetPosition(float x, float y, float z);
	void SetTarget(float x, float y, float z);

	void Update();
	void Move(float d);
	void Zoom(float d);
	void Rotate(float angleX, float angleY);
	void ResetPosition();

	XMMATRIX GetView()const;
	XMMATRIX GetProj()const;
	
	XMFLOAT3 GetPosition()const;
	XMVECTOR GetPositionV()const;
	XMFLOAT3 GetTarget()const;
	XMVECTOR GetTargetV()const;

private:

	XMFLOAT4X4	m_view;
	XMFLOAT4X4	m_proj;

	XMFLOAT3	m_pos;
	XMFLOAT3	m_target;

	uint32_t	m_width;
	uint32_t	m_height;

	float		m_fov;
	float		m_nearZ;
	float		m_farZ;
	float		m_radius;
	float		m_theta;
	float		m_phi;

};

#endif // !CAMERA_H