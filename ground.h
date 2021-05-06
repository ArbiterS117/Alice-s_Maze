//=============================================================================
//
// プレイヤー処理 [player.h]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
//#define PLAYER_MAX		(1)		// プレイヤーのMax人数

//*****************************************************************************
// グローバル変数  "public"
//*****************************************************************************
//struct groundStruct {
//
//	bool					m_Use;						// true:使っている  false:未使用
//	float					m_w, m_h;					// 幅と高さ
//	D3DXVECTOR3				m_Pos;						// ポリゴンの座標
//	D3DXVECTOR3				m_Rot;						// ポリゴンの回転量
//	//anim
//	int						m_CountAnim;				// アニメーションカウント
//	int						m_PatternAnim;				// アニメーションパターンナンバー
//	int						m_TexNo;					// テクスチャ番号
//
//	bool                    turnRight;
//	bool                    turnUp;
//	float                   speedX;
//	float                   speedY;
//
//	float                   maxSpeed;
//	float                   accelerate;
//	float                   decelerate;
//	float                   maxMoveSpeed;
//
//	bool                    gravityScale;
//
//	int TEXTURE_PATTERN_DIVIDE_X, TEXTURE_PATTERN_DIVIDE_Y;
//	int ANIM_PATTERN_NUM;
//
//	int                     status;
//	int                     statusPrev;
//};

//*****************************************************************************
// プロトタイプ宣言   "public"
//*****************************************************************************
HRESULT InitGround(void);
void UninitGround(void);
void UpdateGround(void);
void DrawGround(void);

bool   IsGround(D3DXVECTOR3 vec);
bool   IsGround(float x, float y);
bool   IsSpike(float x, float y);

D3DXVECTOR2  GetGroundCenterPos(float x, float y);
float GetGroundTextureWidth();

int GetLevelX();
int GetLevelY();
void SetLevelX(int level);

bool IsTransportLevel(D3DXVECTOR3 vec);
bool IsTransportLevel(float posx, float posy);
void TransportLevel();

D3DXVECTOR3 GetPlayerRebornSpot(int levelx);
	