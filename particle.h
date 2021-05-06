//=============================================================================
//
// バレット処理 [particle.h]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define PARTICLE_MAX		          (200)		  // 粒子のMax数   as frame
#define PARTICLE_SPEED	              (1.0f)	  // 粒子の移動スピード
#define SMOKE_PARTICLE_LIVETIME       (15)        // スモーク粒子
#define CARD_PARTICLE_LIVETIME        (60)        // カード後ろ星形粒子
#define AQUAJUMP_PARTICLE_LIVETIME    (200)       // アクアジャンプ粒子
#define STAR_PARTICLE_LIVETIME        (40)       // スター粒子（カードアビリティ発動）


enum particleType {
	smoke,                // ジャンプ, 移動, 衝突....
	cardParticle,         // カード飛び 後ろの光
	windParticle,		  // カード飛び 後ろの光
	AquaParticle,		  // カード飛び 後ろの光
	AquaJumpParticle,     // アクアジャンプ
	starParticle,         // カードアビリティ発動
	particleTypesNum
};

// バレット構造体
struct particleStruct
{
	bool					m_use;					// true:使っている  false:未使用
	float					m_w, m_h;					// 幅と高さ
	D3DXVECTOR3				m_pos;					// バレットの座標
	D3DXVECTOR3				m_rot;					// バレットの回転量

	D3DXVECTOR3				speed;				// バレットの移動量 // speed
	bool                    turnRight;

	float                   gravityScale;           // 重力
	//system
	particleType            particletype;
	D3DXCOLOR               Color;                  // 色

	float                   timer;                  // Timer 
	float                   liveTime;               // liveTime

	//anim
	int						countAnim;				// アニメーションカウント
	int						patternAnim;			// アニメーションパターンナンバー
	int						texNo;					// 何番目のテクスチャーを使用するのか

	int TEXTURE_PATTERN_DIVIDE_X, TEXTURE_PATTERN_DIVIDE_Y;
	int ANIM_PATTERN_NUM;

	int                     animDataNo;             //Anim計數

};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitParticle(void);
void UninitParticle(void);
void UpdateParticle(void);
void DrawParticle(void);

particleStruct *GetParticle(void);
void SetParticle(D3DXVECTOR3 pos, D3DXVECTOR3 dir, particleType bType, int particlenum);
