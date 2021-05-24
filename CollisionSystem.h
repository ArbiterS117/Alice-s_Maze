//=============================================================================
//
// 物理碰撞処理 (Collision)[CollisionSystem.cpp]
// Author : GP11B132 14 ゴショウケン
//
//	First Declare "Collider2dStruct".
//  Then use "void SetCollider()". & Add in pool (AddColliderInPool()) //// Update POOL  GetColliderInPool ////
//  Finally use "bool IsCollision() or bool IsTrigger()..." in everywhere you want.
//=============================================================================
#pragma once

#include "main.h"
#include "Transform.h"
#include "Collider2D.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
//#define Physics_MAX		(1)		// プレイヤーのMax人数

//*****************************************************************************
// グローバル変数  "public"
//*****************************************************************************
//float GlobalGraveity = 9.8f;


//*****************************************************************************
// プロトタイプ宣言   "public"
//*****************************************************************************
void InitCollisionSystem(void);
void UninitCollisionSystem(void);
void UpdateCollisionSystem(Transform transform[], int transformNumber);

bool IsCollision(Collider2dStruct col1, Collider2dStruct col2);
bool IsTrigger(Collider2dStruct col1, Collider2dStruct col2);

void AddColliderInPool(Collider2dStruct col, int i); // POOL
Collider2dStruct GetColliderInPool(int i);