//=============================================================================
//
// プレイヤー処理 [enemy.h]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "bullet.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define Enemy_MAX		(36)		// プレイヤーのMax人数

//*****************************************************************************
// グローバル変数  "public"
//*****************************************************************************
enum enemyType {
	Trunk,
	Bunny,
	Slime,
	Turtle,
	Bird,
	Flag,
	EnemyTypesNum
};

struct enemyStruct {

	bool					m_Use;						// true:使っている  false:未使用
	float					m_w, m_h;					// 幅と高さ 
	D3DXVECTOR3				m_Pos;						// ポリゴンの座標
	D3DXVECTOR3				m_Rot;	

	bool                    turnRight;
	bool                    turnUp;
	D3DXVECTOR3             speed;
	float                   maxSpeedY;

	float                   moveSpeed;
	float                   gravityScale;

	//system
	int                     status;
	int                     statusPrev;

	enemyType               m_enemyType;

	int                     m_setUnUseTime;
	bool                    m_isElimated;

	//Slime                 
	int                     reborntime;
	D3DXVECTOR3             rebornPos;
	D3DXCOLOR               enemyColor;
	cardAbility             slimeType;

	//Anim
	int						m_CountAnim;				// アニメーションカウント
	int						m_PatternAnim;				// アニメーションパターンナンバー
	int						m_TexNo;					// テクスチャ番号

	int TEXTURE_PATTERN_DIVIDE_X, TEXTURE_PATTERN_DIVIDE_Y;
	int ANIM_PATTERN_NUM;

	int                     animDataNo;             //Anim計數
	bool                    b_changeAnimInSameState;

	//線形移動 tmep
	float LMtime;//Liner move time
};


//*****************************************************************************
// プロトタイプ宣言   "public"
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

enemyStruct *GetEnemy();
D3DXVECTOR3 GetEnemyPosition(void);

void getHit(enemyStruct &enemy);
void enemyGetElimated(enemyStruct &enemy);

void setEnemyReborn(enemyStruct &enemy);

