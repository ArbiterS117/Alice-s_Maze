//=============================================================================
//
// バレット処理 [bullet.h]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"



//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define BULLET_MAX		                    (30)		 // バレットのMax数
#define BULLET_SPEED	                    (7.0f)		 // バレットの移動スピード

//bullet live time
#define BULLET_STONE_LIVETIME               (60)        // as frame
#define BULLET_SWORD_APPEAR_LIVETIME        (20)
#define BULLET_SWORD_LIVETIME               (32)
#define BULLET_REMOTE_SWORD_LIVETIME        (10)
#define BULLET_REMOTE_WATER_SHOWUP_LIVETIME (20) 
#define BULLET_REMOTE_WATER_BUB_LIVETIME    (1800)
#define BULLET_REMOTE_WATER_HIT_LIVETIME    (20)
#define BULLET_REMOTE_C_REBORN_LIVETIME     (28)
#define BULLET_REMOTE_C_DIS_LIVETIME        (28)
#define BULLET_BLINK_EF_LIVETIME            (24)

#define BULLET_CARD_FORWARD_SPEED           (5.0f)
#define BULLET_CARD_DECELERATE              (0.2f)      // 阻力 
#define BULLET_CARD_FORWARD_TIME            (30)        

#define BULLET_WATER_BUB_MAXNUM             (3)         

									 

enum bulletType {
	stone,
	card,
	swordAppear,
	sword,
	water,
	remoteSword,
	remoteWaterShowUp,
	remoteWaterBub,
	remoteWaterHit,
	characterReborn,
	characterDesappearing,
	enemyReborn,
	blinkEffect,
	bulletTypesNum
};

enum cardAbility {
	none,
	wind,
	Aqua,

};

// バレット構造体
struct bulletStruct
{
	int                     ID;                     // バレットID
	bool					use;					// true:使っている  false:未使用
	float					w, h;					// 幅と高さ
	D3DXVECTOR3				pos;					// バレットの座標
	D3DXVECTOR3				rot;					// バレットの回転量
	
	D3DXVECTOR3				speed;					// バレットの移動量 // speed
	bool                    turnRight;

	//system
	bulletType              bullettype;
	D3DXCOLOR               cardColor;              // カードの色
	cardAbility             cardability;
	bool                    isSummonBack;           // 自動回収スイッチ
	bool                    abilityRemoteSwitch;     // アビリティスイッチ

	float                   timer;                  // Timer for (card forward, ....)
	float                   liveTime;               // liveTime

	//for enemy reborn
	int                     enemynum;               //for enemy reborn


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
HRESULT InitBullet(void);
void UninitBullet(void);
void UpdateBullet(void);
void DrawBullet(void);

bulletStruct *GetBullet(void);
void SetBullet(D3DXVECTOR3 pos, D3DXVECTOR3 dir, bulletType bType);
void SetBullet(D3DXVECTOR3 pos, D3DXVECTOR3 dir, bulletType bType, int enemynum);
void SetBullet(D3DXVECTOR3 pos, D3DXVECTOR3 dir, bulletType bType, cardAbility ability);
void SummonBackCard();
void UseCardAbility();
bool CheckCardHasAbility();

void setBubQ(int ID);