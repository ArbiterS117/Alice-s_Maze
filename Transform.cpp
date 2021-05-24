#include "Transform.h"


// ==============================================================
// Position
// ==============================================================
void Transform::SetPositionX(float x)
{
}

void Transform::SetPositionY(float y)
{
}

void Transform::SetPositionZ(float z)
{
}

void Transform::SetPosition(float x, float y, float z)
{
}

void Transform::SetPosition(D3DVECTOR vec)
{
}

void Transform::SetLocalPositionX(float x)
{
	m_position.x = x;
}

void Transform::SetLocalPositionY(float y)
{
	m_position.y = y;
}

void Transform::SetLocalPositionZ(float z)
{
	m_position.z = z;
}

void Transform::SetLocalPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}

void Transform::SetLocalPosition(D3DVECTOR vec)
{
	m_position.x = vec.x;
	m_position.y = vec.y;
	m_position.z = vec.z;
}

void Transform::UpdatePositionX(float x)
{
	m_position.x += x;
}

void Transform::UpdatePositionY(float y)
{
	m_position.y += y;
}

void Transform::UpdatePositionZ(float z)
{
	m_position.z += z;
}

void Transform::UpdatePosition(float x, float y, float z)
{
	m_position.x += x;
	m_position.y += y;
	m_position.z += z;
}

void Transform::UpdatePosition(D3DVECTOR vec)
{
	m_position.x += vec.x;
	m_position.y += vec.y;
	m_position.z += vec.z;
}

D3DVECTOR Transform::GetPosition()
{
	return D3DVECTOR();
}

float Transform::GetPositionX()
{
	return 0.0f;
}

float Transform::GetPositionY()
{
	return 0.0f;
}

float Transform::GetPositionZ()
{
	return 0.0f;
}

D3DVECTOR Transform::GetLocalPosition()
{
	return m_position;
}

float Transform::GetLocalPositionX()
{
	return m_position.x;
}

float Transform::GetLocalPositionY()
{
	return m_position.y;
}

float Transform::GetLocalPositionZ()
{
	return m_position.z;
}

// ==============================================================
// Rotation
// ==============================================================
void Transform::SetRotationX(float x)
{
}

void Transform::SetRotationY(float y)
{
}

void Transform::SetRotationZ(float z)
{
}

void Transform::SetRotation(float x, float y, float z)
{
}

void Transform::SetRotation(D3DVECTOR vec)
{
}

void Transform::SetLocalRotationX(float x)
{
	m_rotation.x = x;
}

void Transform::SetLocalRotationY(float y)
{
	m_rotation.y = y;
}

void Transform::SetLocalRotationZ(float z)
{
	m_rotation.z = z;
}

void Transform::SetLocalRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}

void Transform::SetLocalRotation(D3DVECTOR vec)
{
	m_rotation.x = vec.x;
	m_rotation.y = vec.y;
	m_rotation.z = vec.z;
}

void Transform::UpdateRotationX(float x)
{
	m_rotation.x += x;
}

void Transform::UpdateRotationY(float y)
{
	m_rotation.y += y;
}

void Transform::UpdateRotationZ(float z)
{
	m_rotation.z += z;
}

void Transform::UpdateRotation(float x, float y, float z)
{
	m_rotation.x += x;
	m_rotation.y += y;
	m_rotation.z += z;
}

void Transform::UpdateRotation(D3DVECTOR vec)
{
	m_rotation.x += vec.x;
	m_rotation.y += vec.y;
	m_rotation.z += vec.z;
}

D3DVECTOR Transform::GetRotation()
{
	return D3DVECTOR();
}

float Transform::GetRotationX()
{
	return 0.0f;
}

float Transform::GetRotationY()
{
	return 0.0f;
}

float Transform::GetRotationZ()
{
	return 0.0f;
}

D3DVECTOR Transform::GetLocalRotation()
{
	return m_rotation;
}

float Transform::GetLocalRotationX()
{
	return m_rotation.x;
}

float Transform::GetLocalRotationY()
{
	return m_rotation.y;
}

float Transform::GetLocalRotationZ()
{
	return m_rotation.z;
}

// ==============================================================
// Scale
// ==============================================================
void Transform::SetScaleX(float x)
{
}

void Transform::SetScaleY(float y)
{
}

void Transform::SetScaleZ(float z)
{
}

void Transform::SetScale(float x, float y, float z)
{
}

void Transform::SetScale(D3DVECTOR vec)
{
}

void Transform::SetLocalScaleX(float x)
{
	m_scale.x = x;
}

void Transform::SetLocalScaleY(float y)
{
	m_scale.y = y;
}

void Transform::SetLocalScaleZ(float z)
{
	m_scale.z = z;
}

void Transform::SetLocalScale(float x, float y, float z)
{
	m_scale.x = x;
	m_scale.y = y;
	m_scale.z = z;
}

void Transform::SetLocalScale(D3DVECTOR vec)
{
	m_scale.x = vec.x;
	m_scale.y = vec.y;
	m_scale.z = vec.z;
}

void Transform::UpdateScaleX(float x)
{
	m_scale.x += x;
}

void Transform::UpdateScaleY(float y)
{
	m_scale.y += y;
}

void Transform::UpdateScaleZ(float z)
{
	m_scale.z += z;
}

void Transform::UpdateScale(float x, float y, float z)
{
	m_scale.x += x;
	m_scale.y += y;
	m_scale.z += z;
}

void Transform::UpdateScale(D3DVECTOR vec)
{
	m_scale.x += vec.x;
	m_scale.y += vec.y;
	m_scale.z += vec.z;
}

D3DVECTOR Transform::GetScale()
{
	return D3DVECTOR();
}

float Transform::GetScaleX()
{
	return 0.0f;
}

float Transform::GetScaleY()
{
	return 0.0f;
}

float Transform::GetScaleZ()
{
	return 0.0f;

}

D3DVECTOR Transform::GetLocalScale()
{
	return m_scale;
}

float Transform::GetLocalScaleX()
{
	return m_scale.x;
}

float Transform::GetLocalScaleY()
{
	return m_scale.y;
}

float Transform::GetLocalScaleZ()
{
	return m_scale.z;
}
