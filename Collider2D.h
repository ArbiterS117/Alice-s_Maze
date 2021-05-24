//=============================================================================
//
// Collider [Collider.h]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#pragma once

#include "main.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
//#define Physics_MAX		(1)		// プレイヤーのMax人数

//*****************************************************************************
// グローバル変数  "public"
//*****************************************************************************
enum ColliderType
{
	Rect,
	Circle
};

struct Collider2dStruct {           //class

	bool use;
	ColliderType Type;
	D3DXVECTOR2	pos;
	float       rotate;
	float       width;
	float       height;

};

//*****************************************************************************
// プロトタイプ宣言   "public"
//*****************************************************************************

void SetCollider(Collider2dStruct &collider2d, ColliderType type, D3DXVECTOR2	pos, float rotate, float width, float height, bool use);

void UpdateCollider(Collider2dStruct &collider2d, D3DXVECTOR2	pos, float rotate);

void SetColliderEnable(Collider2dStruct & collider2d, bool use);