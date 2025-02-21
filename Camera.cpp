#include "Camera.h"
#include "Input.h"

using namespace DirectX;

Camera::Camera(const float aspectRatio, const XMFLOAT3& position) :
	m_fov(XM_PIDIV4),
	m_nearDist(0.1f),
	m_farDist(1000.0f),
	m_moveSpeed(0.5f),
	m_mouseLookSpeed(0.01f),
	m_isPerspective(true)
{
	m_transform = std::make_shared<Transform>();
	m_transform->SetPosition(position);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return m_viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return m_projectionMatrix;
}

void Camera::SetFOV(const float fov)
{
	m_fov = fov;
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 position = m_transform->GetPosition();
	XMFLOAT3 forward = m_transform->GetForward();
	XMFLOAT3 up = m_transform->GetUp();

	XMMATRIX viewMat = XMMatrixLookToLH(XMLoadFloat3(&position), 
										XMLoadFloat3(&forward),
										XMLoadFloat3(&up));

	XMStoreFloat4x4(&m_viewMatrix, viewMat);
}

void Camera::UpdateProjectionMatrix(const float aspectRatio)
{
	XMMATRIX projMat{};
	
	if (m_isPerspective)
	{
		projMat = XMMatrixPerspectiveFovLH(m_fov, aspectRatio, m_nearDist, m_farDist);
	}
	else
	{
		float viewHeight = static_cast<float>(tan(m_fov / 2.0f) * m_nearDist * 2.0f);
		float viewWidth = viewHeight / aspectRatio;
		projMat = XMMatrixOrthographicLH(viewWidth, viewHeight, m_nearDist, m_farDist);
	}
	
	XMStoreFloat4x4(&m_projectionMatrix, projMat);
}

void Camera::Update(const float dt)
{
	float moveSpeed = m_moveSpeed * dt;

	// move forward
	if (Input::KeyDown('W'))
	{
		m_transform->MoveRelative(0.0f, 0.0f, moveSpeed);
	}

	// move backwards
	if (Input::KeyDown('S'))
	{
		m_transform->MoveRelative(0.0f, 0.0f, -moveSpeed);
	}

	// move left
	if (Input::KeyDown('A'))
	{
		m_transform->MoveRelative(-moveSpeed, 0.0f, 0.0f);
	}

	// move right
	if (Input::KeyDown('D'))
	{
		m_transform->MoveRelative(moveSpeed, 0.0f, 0.0f);
	}

	// move up
	if (Input::KeyDown(VK_SPACE))
	{
		m_transform->MoveRelative(0.0f, moveSpeed, 0.0f);
	}

	// move down
	if (Input::KeyDown('X'))
	{
		m_transform->MoveRelative(0.0f, -moveSpeed, 0.0f);
	}

	// Mouse input
	if (Input::MouseLeftDown())
	{
		float rotateY = Input::GetMouseXDelta() * m_mouseLookSpeed;
		float rotateX = Input::GetMouseYDelta() * m_mouseLookSpeed;

		//float currRotateX = m_transform->GetPitchYawRoll().x;

		//XMVECTOR finalRotateX = XMLoadFloat(&rotateX);

		//float min = -XM_PIDIV2 - currRotateX;
		//float max = XM_PIDIV2 - currRotateX;
		//finalRotateX = XMVectorClamp(finalRotateX, XMLoadFloat(&min), XMLoadFloat(&max));

		//XMStoreFloat(&rotateX, finalRotateX);
		//
		m_transform->Rotate(rotateX, rotateY, 0.0f);
	}

	UpdateViewMatrix();
}
