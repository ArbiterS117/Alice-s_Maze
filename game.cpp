//=============================================================================
//
// ゲーム画面処理 [game.cpp]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#include "game.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"

#include "player.h"
#include "enemy.h"
#include "ground.h"
#include "bullet.h"
#include "particle.h"
#include "score.h"
#include "bg.h"
#include "fade.h"
#include "sound.h"

#include "file.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define STOPFRAMECDTIME		 (10)		
#define FINISHFADETIME		 (120)		
#define BLINKCDTIMER         (10)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void CheckHit(void);
bool CheckHitBB(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2, D3DXVECTOR2 size1, D3DXVECTOR2 size2);
bool CheckHitBC(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2, float size1, float size2);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
bool bIsStoppingFrame = false;
int  StoppingFrameLeft = 0;
int  StopFrameCD = 0;

int BlinkCDTimer = 0; // not keep blinking

bool CanSponePlayer = true;
bool IsFinish = false;
int FinishFadeTimer = FINISHFADETIME;

//Game system
int gameTime = 0;
int gameTimeSec = 0;
int gameTimeMin = 0;
int deathCount = 0;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGame(void)
{
	bIsStoppingFrame = false;
	StoppingFrameLeft = 0;
	StopFrameCD = 0;
    BlinkCDTimer = 0; 
	CanSponePlayer = true;
	IsFinish = false;
	FinishFadeTimer = FINISHFADETIME;

	gameTime = 0;
	gameTimeSec = 0;
	gameTimeMin = 0;
	deathCount = 0;

	// プレイヤーの初期化処理
	InitPlayer();

	// エネミーの初期化処理
	InitEnemy();

	// バレットの初期化処理
	InitBullet();

	// 粒子の初期化処理
	InitParticle();

	// Ground
	InitGround();

	// スコアの初期化処理
	InitScore();

	// BGの初期化処理
	InitBg();

	// BGM再生
	//PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGame(void)
{
	// BGの終了処理
	UninitBg();
	
	// スコアの終了処理
	UninitScore();

	// バレットの終了処理
	UninitBullet();

	// 粒子の終了処理
	UninitParticle();

	// Ground
	UninitGround();

	// エネミーの終了処理
	UninitEnemy();

	// プレイヤーの終了処理
	UninitPlayer();
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateGame(void)
{
	playerStruct *player = GetPlayer();
	//gameTimer
	gameTime += 1;
	if (gameTime >= 60) {
		gameTimeSec += 1;
		gameTime = 0;
	}
	if (gameTimeSec >= 60) {
		gameTimeMin += 1;
		gameTimeSec = 0;
	}

	//Timer
	if (StoppingFrameLeft > 0 ) {
		bIsStoppingFrame = true;
		StoppingFrameLeft -= 1;
	}
	else {
		bIsStoppingFrame = false;
	}
	
	if (StopFrameCD > 0)StopFrameCD -=1;

	if (BlinkCDTimer > 0) {
		BlinkCDTimer -= 1;
	}

	// Spone Player
	if (player[0].m_Use == false && CanSponePlayer == true && !IsFinish) {
		SetBullet(GetPlayerRebornSpot(GetLevelX()), D3DXVECTOR3(1.0f, 0.0f, 0.0f), characterReborn);
		CanSponePlayer = false;
	}
	if (player[0].m_Use == true) {
		CanSponePlayer = true;
	}

	if (IsFinish) {
		FinishFadeTimer -= 1;
		if (FinishFadeTimer <= 0) {
			SetFade(FADE_OUT, MODE_RESULT);
		}
	}

#ifdef _DEBUG
	if (GetKeyboardTrigger(DIK_RETURN)) {
		SetFade(FADE_OUT, MODE_RESULT);
	}
#endif

	// プレイヤーの更新処理
	UpdatePlayer();
	
	// エネミーの更新処理
	UpdateEnemy();

	// バレットの更新処理
	UpdateBullet();

	// 当たり判定
	CheckHit();

	// 粒子の更新処理
	UpdateParticle();

	// Ground
	UpdateGround();

	// スコアの更新処理
	UpdateScore();

	// BGの更新処理
	UpdateBg();

	// セーブ＆ロードのテスト
	if (GetKeyboardTrigger(DIK_S))
	{
		SaveData();
	}

	if (GetKeyboardTrigger(DIK_L))
	{
		LoadData();
	}

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGame(void)
{
	// BGの描画処理
	DrawBg();

	// Ground
	DrawGround();

	// エネミーの描画処理
	DrawEnemy();

	// バレットの描画処理
	DrawBullet();
	
	// 粒子の描画処理
	DrawParticle();

	// プレイヤーの描画処理
	DrawPlayer();

	// スコアの描画処理
	DrawScore();

}


//=============================================================================
// BBによる当たり判定処理
// 回転は考慮しない
// 戻り値：当たってたらtrue
//=============================================================================
bool CheckHitBB(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2, D3DXVECTOR2 size1, D3DXVECTOR2 size2)
{
	//当たり判定AABB        ↓辺　
	if ((pos1.x + size1.x * 0.5f > pos2.x - size2.x * 0.5f) &&
		(pos1.x - size1.x * 0.5f < pos2.x + size2.x * 0.5f) &&
		(pos1.y + size1.y * 0.5f > pos2.y - size2.y * 0.5f) &&
		(pos1.y - size1.y * 0.5f < pos2.y + size2.y * 0.5f))
	{
		return true;
	}

	return false;
}

//=============================================================================
// BCによる当たり判定処理
// サイズは半径
// 戻り値：当たってたらtrue
//=============================================================================
bool CheckHitBC(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2, float r1, float r2) // 半径
{
	float len = (r1 + r2) * (r1 + r2);	// 半径を2乗した物
	D3DXVECTOR3 temp = pos1 - pos2;
	float fLengthSq = D3DXVec3LengthSq(&temp);		// 2点間の距離（2乗した物） 三平方の定理含むは D3DXVec3Length()

	if (len > fLengthSq)
	{
		return true;
	}

	return false;
}


//=============================================================================
// 当たり判定処理
//=============================================================================
void CheckHit(void)
{
	playerStruct *player = GetPlayer();		// プレイヤーのポインターを初期化
	enemyStruct  *enemy = GetEnemy();		// エネミーのポインターを初期化
	bulletStruct *bullet = GetBullet();		// バレットのポインターを初期化

	// 敵と操作キャラ(BB)
	if (player[0].m_Use == true)
	{
		for (int i = 0; i < Enemy_MAX; i++)
		{
			if (enemy[i].m_Use == false || enemy[i].m_isElimated == true)	continue;

			//Step on Enemys
			if (CheckHitBB(player[0].m_Pos, enemy[i].m_Pos, D3DXVECTOR2(player[0].m_w, player[0].m_h ), D3DXVECTOR2(enemy[i].m_w * 0.75f , enemy[i].m_h * 0.75f)))
			{
				if (!player[0].m_isElimated) {
					// step on enemy
					if (enemy[i].m_enemyType != Turtle) {
						if (player[0].m_Pos.y + player[0].m_h / 2 < enemy[i].m_Pos.y && player[0].speedY > 0.0f) {
							player[0].speedY = -player[0].jumpSpeed * 1.2f;
							enemy[i].speed += D3DXVECTOR3(0.0f, -10.0f, 0.0f); // 飛
							enemyGetElimated(enemy[i]);
							PlaySound(SOUND_LABEL_SE_hit2);
						}
					}
					// hit enemy
					if (!enemy[i].m_isElimated && enemy[i].m_enemyType != Flag) {
						player[0].speedY = -10.0f; // 飛
						playerGetElimated(player[0]);
						
					}
					//Hit flag
					if (enemy[i].m_enemyType == Flag) {
						enemy[i].speed += D3DXVECTOR3(0.0f, -10.0f, 0.0f); // 飛
						player[0].speedY = -10.0f; // 飛
						SetBullet(D3DXVECTOR3(player[0].m_Pos.x, player[0].m_Pos.y, player[0].m_Pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), characterDesappearing);
						getHit(enemy[i]);
						player[0].m_Use = false;
						IsFinish = true;
					}
				}

			}
		}
	}

	// 弾と敵(BC)
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (bullet[i].use == false) continue;

		for (int j = 0; j < Enemy_MAX; j++)
		{
			if (enemy[j].m_Use == false || enemy[j].m_isElimated == true) continue;

			if (CheckHitBC(bullet[i].pos, enemy[j].m_Pos, bullet[i].w*0.5f, enemy[j].m_w*0.5f))
			{
				//Stone hit enemy
				if (bullet[i].bullettype == stone) {
					bullet[i].use = false;		// 弾の消滅処理を行い

					enemy[j].speed += D3DXVECTOR3((enemy[j].m_Pos.x - bullet[i].pos.x) * 5.0f, -10.0f, 0.0f); // 飛
					enemyGetElimated(enemy[j]);

					//break;					// １回当たって終わりの時はここでbreakする。１フレーム内の間有効ならbreakをコメントにする。
				}

				//card hit enemy
				if (bullet[i].bullettype == card) {
					//モンスターを突き抜ける
					if (bullet[i].turnRight)bullet[i].speed.x += 0.1f;
					else bullet[i].speed.x -= 0.1f;

					if (enemy[j].m_enemyType == Trunk) {
						if (BlinkCDTimer == 0) {
							EffectStopFrame(5);
							SetBullet(D3DXVECTOR3(enemy[j].m_Pos.x + enemy[j].m_w / 2, enemy[j].m_Pos.y - enemy[j].m_h / 2, enemy[j].m_Pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), blinkEffect);
							BlinkCDTimer = BLINKCDTIMER;
						}
					}

					if (enemy[j].m_enemyType == Bunny) {
						if (BlinkCDTimer == 0) {
							EffectStopFrame(5);
							SetBullet(D3DXVECTOR3(enemy[j].m_Pos.x + enemy[j].m_w / 2, enemy[j].m_Pos.y - enemy[j].m_h / 2, enemy[j].m_Pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), blinkEffect);
							BlinkCDTimer = BLINKCDTIMER;
						}
					}
					if (enemy[j].m_enemyType == Slime) {
						EffectStopFrame(5);
						if (enemy[j].slimeType == wind) {
							bullet[i].cardColor = D3DXCOLOR(0.3f, 1.0f, 0.3f, 1.0f);
							bullet[i].cardability = wind;
							getHit(enemy[j]);
						}
						if (enemy[j].slimeType == Aqua) {
							bullet[i].cardColor = D3DXCOLOR(0.3f, 0.3f, 1.0f, 1.0f);
							bullet[i].cardability = Aqua;
							getHit(enemy[j]);
						}
					}
					if (enemy[j].m_enemyType == Turtle) {
						if (BlinkCDTimer == 0) {
							EffectStopFrame(5);
							SetBullet(D3DXVECTOR3(enemy[j].m_Pos.x + enemy[j].m_w / 2, enemy[j].m_Pos.y - enemy[j].m_h / 2, enemy[j].m_Pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), blinkEffect);
							BlinkCDTimer = BLINKCDTIMER;
						}
					}
					if (enemy[j].m_enemyType == Bird) {
						if (BlinkCDTimer == 0) {
							EffectStopFrame(5);
							SetBullet(D3DXVECTOR3(enemy[j].m_Pos.x + enemy[j].m_w / 2, enemy[j].m_Pos.y - enemy[j].m_h / 2, enemy[j].m_Pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), blinkEffect);
							BlinkCDTimer = BLINKCDTIMER;
						}

					}
				}

				//sword hit enemy
				if (bullet[i].bullettype == sword) {
					EffectStopFrame(7);

					enemy[j].speed += D3DXVECTOR3((enemy[j].m_Pos.x - bullet[i].pos.x) * 5.0f, -10.0f, 0.0f); // 飛
					enemyGetElimated(enemy[j]);

				}


			}
		}
	}

	// ボスと弾(BC)


	// 自分と敵の弾(BC)

	// 自分の弾と自分(BC)
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (bullet[i].use == false) continue;
		if (player[0].m_Use == false) continue;

		if (CheckHitBC(bullet[i].pos, player[0].m_Pos, bullet[i].w*0.5f, player[0].m_w*0.5f))
		{
			//Forward Stop and pickup enable
			if (fabs(bullet[i].speed.x) < BULLET_CARD_FORWARD_SPEED / 2 && bullet[i].bullettype == card) {
				player[0].isThrowedCard = false;
				switch (bullet[i].cardability)
				{
				case wind:
					player[0].m_playerAbility = p_wind;
					break;
				case Aqua:
					player[0].m_playerAbility = p_Aqua;
					break;
				default:
					break;
				}
				bullet[i].use = false;		// 弾の消滅処理を行い
				//break;					// １回当たって終わりの時はここでbreakする。１フレーム内の間有効ならbreakをコメントにする。
			}

			// AquaBub for AquaJump
			if (bullet[i].bullettype == remoteWaterBub) {
				SetBullet(D3DXVECTOR3(bullet[i].pos.x, bullet[i].pos.y, bullet[i].pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), remoteWaterHit);
				bullet[i].use = false;
				setBubQ(i);
				player[0].speedY -= player[0].aquaJumpSpeed;
				PlaySound(SOUND_LABEL_SE_aqua);
				PlaySound(SOUND_LABEL_SE_jump);
				SetParticle(D3DXVECTOR3(player[0].m_Pos.x, player[0].m_Pos.y + player[0].m_h / 2.0f, 0.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), AquaJumpParticle, 20);
			}

		}
		
	}


	// エネミーが全部死亡したら状態遷移
	int enemy_count = 0;
	for (int i = 0; i < Enemy_MAX; i++)
	{
		if (enemy[i].m_Use == false) continue;
		enemy_count++;
	}

	// エネミーが０匹？
	if (enemy_count == 0)
	{
		//SetFade(FADE_OUT, MODE_RESULT);
	}

}

//================ Stop frame

void EffectStopFrame(int frame) {
	if (!bIsStoppingFrame && StopFrameCD == 0) {
		StoppingFrameLeft = frame;
		StopFrameCD = STOPFRAMECDTIME;
	}
}

bool IsStoppingFrame()
{
	return bIsStoppingFrame;
}

int GetGameTimeFrame()
{
	return gameTime;
}

int GetGameTimeSec()
{
	return gameTimeSec;
}

int GetGameTimeMin()
{
	return gameTimeMin;
}

int GetDeathCount()
{
	return deathCount;
}

void AddDeathCount(int i)
{
	deathCount += 1;
}
