//=============================================================================
//
// 物理碰撞処理 (Collision)[CollisionSystem.cpp]
// Author : GP11B132 14 ゴショウケン
//
//	First Declare "Collider2dStruct".
//  Then use "void SetCollider()". & Add in pool (AddColliderInPool()) //// Update POOL  GetColliderInPool ////
//  Finally use "bool IsCollision() or bool IsTrigger()..." in everywhere you want.
//=============================================================================
#include "CollisionSystem.h"

#include "player.h"
#include "enemy.h"
#include "bullet.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
//#define PLAYER_MAX		(1)		// プレイヤーのMax人数

//*****************************************************************************
// グローバル変数  "private"
//*****************************************************************************
Collider2dStruct colliderPool[2]; // Pool

struct OBB
{
	D3DXVECTOR2	Pos;        // 位置
	D3DXVECTOR2	Dir[2];		// XY各座標軸の傾きを表す方向ベクトル
	float		Length[2];  // 各軸方向の長さの半分
};


//*****************************************************************************
// プロトタイプ宣言   "private"
//*****************************************************************************
void CreateOBB(OBB * pOutOBB, const D3DXVECTOR2 & Pos, const D3DXMATRIX & RotateMtx, const D3DXVECTOR2 & Length);

float LenSegOnSeparateAxis(D3DXVECTOR2 * Sep, D3DXVECTOR2 * e1, D3DXVECTOR2 * e2);

bool TestOBBOBB(const OBB & obb1, const OBB & obb2);

//*****************************************************************************
//"public"
//*****************************************************************************
void InitCollisionSystem(void)
{
	
}

void UninitCollisionSystem(void)
{

}

void UpdateCollisionSystem(Transform transform[], int transformNumber)
{
	playerStruct *players = GetPlayer();
	enemyStruct *enemys = GetEnemy();
	bulletStruct *bullets = GetBullet();

	for (int i = 0; i < transformNumber; i++) {
		// 處理場上每個物理物件
	}

	for (int i = 0; i < Enemy_MAX; i++) {
		// 處理Enemy
	}
}

bool IsCollision(Collider2dStruct col1, Collider2dStruct col2)
{
	if (col1.use == true && col2.use == true) {
		OBB obb1;
		OBB obb2;
		D3DXMATRIX rotMtx1;
		D3DXMATRIX rotMtx2;

		D3DXMatrixRotationZ(&rotMtx1, col1.rotate);
		D3DXMatrixRotationZ(&rotMtx2, col2.rotate);
		CreateOBB(&obb1, col1.pos, rotMtx1, D3DXVECTOR2(col1.width / 2.0f, col1.height / 2.0f));
		CreateOBB(&obb2, col2.pos, rotMtx2, D3DXVECTOR2(col2.width / 2.0f, col2.height / 2.0f));
		return TestOBBOBB(obb1, obb2);
	}
	else
		return false;
}

bool IsTrigger(Collider2dStruct col1, Collider2dStruct col2)
{
	if (col1.use == true && col2.use == true) {
		OBB obb1;
		OBB obb2;
		D3DXMATRIX rotMtx1;
		D3DXMATRIX rotMtx2;

		D3DXMatrixRotationZ(&rotMtx1, col1.rotate);
		D3DXMatrixRotationZ(&rotMtx2, col2.rotate);
		CreateOBB(&obb1, col1.pos, rotMtx1, D3DXVECTOR2(col1.width / 2.0f, col1.height / 2.0f));
		CreateOBB(&obb2, col2.pos, rotMtx2, D3DXVECTOR2(col2.width / 2.0f, col2.height / 2.0f));
		return TestOBBOBB(obb1, obb2);
	}
	else
		return false;
}

void AddColliderInPool(Collider2dStruct col, int i)
{
	colliderPool[i] = col;
}

Collider2dStruct GetColliderInPool(int i)
{
	return colliderPool[i];
}

// スプライトの当たり判定（２次元のOBB vs OBB） // 
bool IsHitTest(const Collider2dStruct/*Sprite&*/ sprite1, const Collider2dStruct/*Sprite&*/ sprite2)
{
	OBB obb1;
	OBB obb2;
	D3DXMATRIX rotMtx1;
	D3DXMATRIX rotMtx2;

	D3DXMatrixRotationZ(&rotMtx1, sprite1.rotate);
	D3DXMatrixRotationZ(&rotMtx2, sprite2.rotate);
	CreateOBB(&obb1, sprite1.pos, rotMtx1, D3DXVECTOR2(sprite1.width / 2.0f, sprite1.height / 2.0f));
	CreateOBB(&obb2, sprite2.pos, rotMtx2, D3DXVECTOR2(sprite2.width / 2.0f, sprite2.height / 2.0f));
	return TestOBBOBB(obb1, obb2);
}

//*****************************************************************************
// "private"
//*****************************************************************************

// OBB作成
void CreateOBB(OBB* pOutOBB, const D3DXVECTOR2& Pos, const D3DXMATRIX& RotateMtx, const D3DXVECTOR2& Length)
{
	pOutOBB->Pos = Pos;
	pOutOBB->Length[0] = Length.x;
	pOutOBB->Length[1] = Length.y;
	pOutOBB->Dir[0] = D3DXVECTOR2(RotateMtx._11, RotateMtx._12);
	pOutOBB->Dir[1] = D3DXVECTOR2(RotateMtx._21, RotateMtx._22);
}

// 分離軸に投影された軸成分から投影線分長を算出
float LenSegOnSeparateAxis(D3DXVECTOR2 *Sep, D3DXVECTOR2 *e1, D3DXVECTOR2 *e2)
{
	// 3つの内積の絶対値の和で投影線分長を計算
	// 分離軸Sepは標準化されていること
	float r1 = fabs(D3DXVec2Dot(Sep, e1));
	float r2 = fabs(D3DXVec2Dot(Sep, e2));
	return r1 + r2;
}

// OBB vs OBB
bool TestOBBOBB(const OBB &obb1, const OBB &obb2)
{
	// 各方向ベクトルの確保
	// （N***:標準化方向ベクトル）
	D3DXVECTOR2 NAe1 = obb1.Dir[0], Ae1 = NAe1 * obb1.Length[0];
	D3DXVECTOR2 NAe2 = obb1.Dir[1], Ae2 = NAe2 * obb1.Length[1];
	D3DXVECTOR2 NBe1 = obb2.Dir[0], Be1 = NBe1 * obb2.Length[0];
	D3DXVECTOR2 NBe2 = obb2.Dir[1], Be2 = NBe2 * obb2.Length[1];
	D3DXVECTOR2 Interval = obb1.Pos - obb2.Pos;

	// 分離軸 : Ae1
	float rA = D3DXVec2Length(&Ae1);
	float rB = LenSegOnSeparateAxis(&NAe1, &Be1, &Be2);
	float L = fabs(D3DXVec2Dot(&Interval, &NAe1));
	if (L > rA + rB)
		return false; // 衝突していない

	// 分離軸 : Ae2
	rA = D3DXVec2Length(&Ae2);
	rB = LenSegOnSeparateAxis(&NAe2, &Be1, &Be2);
	L = fabs(D3DXVec2Dot(&Interval, &NAe2));
	if (L > rA + rB)
		return false;

	// 分離軸 : Be1
	rA = LenSegOnSeparateAxis(&NBe1, &Ae1, &Ae2);
	rB = D3DXVec2Length(&Be1);
	L = fabs(D3DXVec2Dot(&Interval, &NBe1));
	if (L > rA + rB)
		return false;

	// 分離軸 : Be2
	rA = LenSegOnSeparateAxis(&NBe2, &Ae1, &Ae2);
	rB = D3DXVec2Length(&Be2);
	L = fabs(D3DXVec2Dot(&Interval, &NBe2));
	if (L > rA + rB)
		return false;

	// 分離平面が存在しないので「衝突している」
	return true;
}

