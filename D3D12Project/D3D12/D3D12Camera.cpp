#include "D3D12Camera.h"

D3D12Camera::D3D12Camera() {

	//View
	m_cameraPos = {1.0f, 0.0f, -1.0f};
	m_focusPoint = {0.0f, 0.0f, 0.0f};
	m_cameraUp = {0.0f, 1.0f, 0.0f};
	m_viewMatrix = XMMatrixLookAtLH(m_cameraPos, m_focusPoint, m_cameraUp);

	//Projection
	m_fov = 75.0f;
	m_aspectRatio = 800.0f / 600.0f;
	m_nearPlane = 0.1f;
	m_farPlane = 100.0f;
	m_projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);

	m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;

	m_trigAngle = 0.0f;
}

D3D12Camera::~D3D12Camera() {

}

void D3D12Camera::setFocusPoint(XMVECTOR focusPoint) {
	m_focusPoint = focusPoint;
}

void D3D12Camera::update(float dt) {
	m_cameraPos = XMVECTOR{ cos(m_trigAngle) * 7.0f, 2.0f, sin(m_trigAngle) * 7.0f };

	m_trigAngle = fmod(m_trigAngle + dt, 6.28f);

	m_viewMatrix = XMMatrixLookAtLH(m_cameraPos, m_focusPoint, m_cameraUp);
	m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
}

XMMATRIX D3D12Camera::getViewProjection() {
	return m_viewProjectionMatrix;
}