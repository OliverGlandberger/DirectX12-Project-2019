#pragma once

#include <DirectXMath.h>

#include "D3D12ConstantBuffer.h"

using namespace DirectX;

class D3D12Camera {
private:
	//View
	XMVECTOR m_cameraPos;
	XMVECTOR m_focusPoint;
	XMVECTOR m_cameraUp;
	XMMATRIX m_viewMatrix;

	//Projection
	float m_fov;
	float m_aspectRatio;
	float m_nearPlane;
	float m_farPlane;
	XMMATRIX m_projectionMatrix;

	//View-Proj
	XMMATRIX m_viewProjectionMatrix;

	//Update
	float m_trigAngle;

public:
	D3D12Camera();
	~D3D12Camera();

	void setFocusPoint(XMVECTOR focusPoint);

	void update(float dt);

	XMMATRIX getViewProjection();

};