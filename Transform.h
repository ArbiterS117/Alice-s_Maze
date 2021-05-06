////////////////////////////////////////////////////////////////////////////////
// Filename: Transform.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>

#include <fstream>
using namespace std;

class Transform
{
public:
	Transform();
	~Transform();

	// ==============================================================
	// Position
	// ==============================================================
	void SetPositionX(float x);
	void SetPositionY(float y);
	void SetPositionZ(float z);
	void SetPosition(float x, float y, float z);
	void SetPosition(D3DVECTOR vec);
	
	void SetLocalPositionX(float x);
	void SetLocalPositionY(float y);
	void SetLocalPositionZ(float z);
	void SetLocalPosition(float x, float y, float z);
	void SetLocalPosition(D3DVECTOR vec);

	void UpdatePositionX(float x);
	void UpdatePositionY(float y);
	void UpdatePositionZ(float z);
	void UpdatePosition(float x, float y, float z);
	void UpdatePosition(D3DVECTOR vec);

	D3DVECTOR GetPosition(); 
	float GetPositionX();
	float GetPositionY();
	float GetPositionZ();

	D3DVECTOR GetLocalPosition();
	float GetLocalPositionX();
	float GetLocalPositionY();
	float GetLocalPositionZ();

	// ==============================================================
	// Rotation
	// ==============================================================
	void SetRotationX(float x);
	void SetRotationY(float y);
	void SetRotationZ(float z);
	void SetRotation(float x, float y, float z);
	void SetRotation(D3DVECTOR vec);

	void SetLocalRotationX(float x);
	void SetLocalRotationY(float y);
	void SetLocalRotationZ(float z);
	void SetLocalRotation(float x, float y, float z);
	void SetLocalRotation(D3DVECTOR vec);

	void UpdateRotationX(float x);
	void UpdateRotationY(float y);
	void UpdateRotationZ(float z);
	void UpdateRotation(float x, float y, float z);
	void UpdateRotation(D3DVECTOR vec);

	D3DVECTOR GetRotation();
	float GetRotationX();
	float GetRotationY();
	float GetRotationZ();

	D3DVECTOR GetLocalRotation();
	float GetLocalRotationX();
	float GetLocalRotationY();
	float GetLocalRotationZ();

	// ==============================================================
	// Scale
	// ==============================================================
	void SetScaleX(float x);
	void SetScaleY(float y);
	void SetScaleZ(float z);
	void SetScale(float x, float y, float z);
	void SetScale(D3DVECTOR vec);

	void SetLocalScaleX(float x);
	void SetLocalScaleY(float y);
	void SetLocalScaleZ(float z);
	void SetLocalScale(float x, float y, float z);
	void SetLocalScale(D3DVECTOR vec);

	void UpdateScaleX(float x);
	void UpdateScaleY(float y);
	void UpdateScaleZ(float z);
	void UpdateScale(float x, float y, float z);
	void UpdateScale(D3DVECTOR vec);

	D3DVECTOR GetScale();
	float GetScaleX();
	float GetScaleY();
	float GetScaleZ();

	D3DVECTOR GetLocalScale();
	float GetLocalScaleX();
	float GetLocalScaleY();
	float GetLocalScaleZ();

	D3DMATRIX pM;

private:
	D3DVECTOR m_position, m_rotation, m_scale;

	bool isGravity;
	
};

