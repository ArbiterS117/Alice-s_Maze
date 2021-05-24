//=============================================================================
//
// カメラの処理 [camera.h]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#pragma once



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void InitCamera(void);
void UninitCamera(void);
void UpdateCamera(void);
void SetCamera(void);

D3DXMATRIX GetCameraViewMatrix();
D3DXMATRIX GetCameraInvViewMatrix();
D3DXMATRIX GetCameraProjectionMatrix();
D3DXVECTOR3 GetCameraPosition();
