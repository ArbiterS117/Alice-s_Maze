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
#define PLAYER_MAX		(1)		// プレイヤーのMax人数

//*****************************************************************************
// グローバル変数  "public"
//*****************************************************************************
enum playerAbility {
	p_none,
	p_wind,
	p_Aqua,
};

struct playerStruct {

	bool					m_Use;						// true:使っている  false:未使用
	float					m_w, m_h;					// 幅と高さ
	D3DXVECTOR3				m_Pos;						// ポリゴンの座標
	D3DXVECTOR3				m_Rot;						// ポリゴンの回転量
	
	bool                    turnRight;
	bool                    turnUp;
	float                   speedX;
	float                   speedY;

	float                   maxSpeedY;
	float                   accelerate;
	float                   decelerate;
	float                   maxMoveSpeed;
	float                   jumpSpeed;

	float                   gravityScale;

	int                     status;
	int                     statusPrev;

	int                     m_setUnUseTime;
	bool                    m_isElimated;

	//system
	playerAbility           m_playerAbility;
	float                   aquaJumpSpeed;

	bool                    isGround;
	bool                    isGroundPrev;
	bool                    doubleJumpSwitch;

	bool                    isThrowedCard;

	//Anim
	int						m_CountAnim;				// アニメーションカウント
	int						m_PatternAnim;				// アニメーションパターンナンバー
	int						m_TexNo;					// テクスチャ番号

	int TEXTURE_PATTERN_DIVIDE_X, TEXTURE_PATTERN_DIVIDE_Y;
	int ANIM_PATTERN_NUM;

	int                     animDataNo;             //Anim計數
	bool                    b_AnimTActionFireCardTrigger;
	bool                    b_changeAnimInSameState;

	bool                    b_AnimStepParticleSwitch; // ステップ生成の粒子

};



//*****************************************************************************
// プロトタイプ宣言   "public"
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

playerStruct *GetPlayer();
D3DXVECTOR3 GetPlayerPosition(void);

void playerGetElimated(playerStruct &player);
void setPlayerReborn(playerStruct &player);

