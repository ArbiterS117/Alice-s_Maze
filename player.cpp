//=============================================================================
//
// プレイヤー処理 [player.cpp]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#include "game.h"
#include "player.h"
#include "sprite.h"

#include "input.h"
#include "Collider2D.h"
#include "CollisionSystem.h"

#include "bullet.h"
#include "particle.h"
#include "ground.h"

#include "sound.h"
#include "score.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(32)//(256)//(SCREEN_WIDTH)	// (200)			// キャラサイズ
#define TEXTURE_HEIGHT				(32)//(256)//(SCREEN_HEIGHT)	// (200)			// 
#define TEXTURE_MAX					(1)		// テクスチャの数

#define ANIM_ONEACTION_MAX_SPRITES  (20)
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値

#define PLAYER_SETUNUSEDTIME        (100)
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void  ChangeStatus(playerStruct *player,int status);

void InputUpdate(playerStruct &player);
void PhysicsUpdate(playerStruct &player);
void CollisionUpdate(playerStruct &player);
void playerStatusUpdate(playerStruct &player);
void AnimUpdate(playerStruct &player);

//Actions
void ActionThrowCard(playerStruct & player);
void ActionAttack(playerStruct & player);

//Anim Trigger Actions
void AnimTActionFireCard(playerStruct & player, bulletType type);

//*****************************************************************************
// グローバル変数  "private"
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[] = {
	
"data/TEXTURE/ALICE.png"

};

playerStruct players[PLAYER_MAX];

Collider2dStruct playersCollider2d;

//=========Status
enum PlayerStatus
{
	A_Idle,
	A_InAir,
	A_Move,
	A_ThrowCard,
	A_Attack,
	A_DEAD,
	ANIM_ACTION_NUM   // アニメーション種類の数

};

//=========Anim
struct AnimSpritesData {
	int Data[ANIM_ONEACTION_MAX_SPRITES];
};

enum AnimPlayType{
	APT_LOOP  = -1,	 // loop
	APT_STOPF = -2,	 // stop in final
	APT_CWV   = -3,	 // Change with velocity
	APT_COS   = -4   // Change to Other States
};

// Same order as Player Status
// -1 : loop   -2: stop in final   -3: Change with velocity  -4: Change to Other States ......
AnimSpritesData m_SpritesData[ANIM_ACTION_NUM] = {
	{14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, APT_LOOP},     //A_Idle
	{25, 26, 6, 6, APT_CWV},                                    //A_InAir
	{26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, APT_LOOP}, //A_Move
	{0, 1, 2, 3, 4, 5, 5, APT_COS},                             //A_ThrowCard
	{0, 1, 2, 3, 4, 5, 5, APT_COS},                             //A_Attack
	{7, 8, 9, 10, 11, 12, 13, APT_STOPF},						//A_Dead
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(
			GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// プレイヤーの初期化
	for (int c = 0; c < PLAYER_MAX; c++)
	{

		players[c].m_Use = false;
		players[c].m_w = TEXTURE_WIDTH;
		players[c].m_h = TEXTURE_HEIGHT;
		players[c].m_Pos = D3DXVECTOR3(96.0f, SCREEN_HEIGHT - 176.0f, 0.0f);
		players[c].m_Rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		players[c].m_TexNo = 0; // = c & 1   // 奇数または偶数

		players[c].turnRight = true;
		players[c].turnUp    = false;
		players[c].speedX    = 0;
		players[c].speedY    = 0;

		players[c].accelerate   = 0.5f;
		players[c].decelerate   = 0.2f; // 空気阻力
		players[c].maxSpeedY    = 11.0f;
		players[c].maxMoveSpeed = 2.0f;
		players[c].jumpSpeed    = 9.0f;
		players[c].gravityScale = 0.4f;

		players[c].status     = 0;
		players[c].statusPrev = 0;

		players[c].m_setUnUseTime = PLAYER_SETUNUSEDTIME;
		players[c].m_isElimated = false;

		players[c].m_playerAbility  = p_none;
		players[c].aquaJumpSpeed    = 20.0f;
		players[c].isGround         = false;
		players[c].isGroundPrev     = false;
		players[c].doubleJumpSwitch = true;
		players[c].isThrowedCard    = false;

		players[c].TEXTURE_PATTERN_DIVIDE_X = 1;
		players[c].TEXTURE_PATTERN_DIVIDE_Y = 1;
		players[c].ANIM_PATTERN_NUM         = 1;

		players[c].m_CountAnim   = 0;
		players[c].m_PatternAnim = 0;

		players[c].animDataNo                   = 0;
		players[c].b_AnimTActionFireCardTrigger = true;
		players[c].b_changeAnimInSameState      = false;
		players[c].b_AnimStepParticleSwitch     = true;

	}

	//Anim Setting    int TEXTURE_PATTERN_DIVIDE_X, TEXTURE_PATTERN_DIVIDE_Y; int ANIM_PATTERN_NUM = TEXTURE_PATTERN_DIVIDE_X * TEXTURE_PATTERN_DIVIDE_Y;
	players[0].m_TexNo = 0;
	players[0].TEXTURE_PATTERN_DIVIDE_X = 38; //4;
	players[0].TEXTURE_PATTERN_DIVIDE_Y = 1;  // 7;
	players[0].ANIM_PATTERN_NUM = players[0].TEXTURE_PATTERN_DIVIDE_X * players[0].TEXTURE_PATTERN_DIVIDE_Y;
	players[0].status = A_Idle;
	//players[0].m_PatternAnim = m_SpritesData[players[0].status].Start;
	players[0].m_PatternAnim = m_SpritesData[players[0].status].Data[0];

	//Collider2d
	SetCollider(playersCollider2d, Rect, D3DXVECTOR2(players[0].m_Pos.x, players[0].m_Pos.y), 0.0f, players[0].m_w / 2.0f, players[0].m_h / 2.0f, players[0].m_Use); // 畫布太大長寬減半
	AddColliderInPool(playersCollider2d, 0);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
{
	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer(void)
{
	for (int c = 0; c < PLAYER_MAX; c++)
	{
		if (players[c].m_Use == true)		// プレイヤーが使われている？
		{						// Yes
			//================Input
			InputUpdate(players[c]);

			//================Physics
			PhysicsUpdate(players[c]);

			//================Collision
			CollisionUpdate(players[c]);

			//================PlayerStatus & other
			playerStatusUpdate(players[c]);

			// ================アニメーション 
			AnimUpdate(players[c]);

		}
	}

#ifdef _DEBUG	// デバッグ情報を表示する
	/*char *str = GetDebugStr();
	sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", m_Pos.x, m_Pos.y);*/
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PosX:%.2f PosY:%.2f Status:%.2f  AnimP:%d ", players[0].m_Pos.x, players[0].m_Pos.y, players[0].speedY, players[0].m_PatternAnim);
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	for (int c = 0; c < PLAYER_MAX; c++)
	{
		if (players[c].m_Use == true)		// プレイヤーが使われている？
		{						// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[players[c].m_TexNo]);

			// プレイヤーの位置やテクスチャー座標を反映
			float px = players[c].m_Pos.x - GetLevelX() * SCREEN_WIDTH;	// プレイヤーの表示位置X
			float py = players[c].m_Pos.y - GetLevelY() * SCREEN_HEIGHT;	// プレイヤーの表示位置Y
			if (px < SCREEN_WIDTH * 1.2f || px > 0 - SCREEN_WIDTH * 0.2f)  // no need to draw 
			{
				float pw = players[c].m_w;		// プレイヤーの表示幅
				float ph = players[c].m_h;		// プレイヤーの表示高さ

				float tw = 1.0f / players[c].TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
				float th = 1.0f / players[c].TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
				float tx = ((float)(players[c].m_PatternAnim % players[c].TEXTURE_PATTERN_DIVIDE_X)) * tw;	// テクスチャの左上X座標
				float ty = ((float)(players[c].m_PatternAnim / players[c].TEXTURE_PATTERN_DIVIDE_X)) * th;	// テクスチャの左上Y座標

				float r = players[c].m_Rot.z;

				D3DXCOLOR color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
				// １枚のポリゴンの頂点とテクスチャ座標を設定
				/*if (players[c].turnRight)SetSprite(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th);
				else SetSprite(g_VertexBuffer, px, py, pw, ph, tx + tw, ty, -tw, th);*/
				if (players[c].turnRight)SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th, color, r);
				else SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx + tw, ty, -tw, th, color, r);
				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}
}


//=============================================================================
// プレイヤーの座標を取得
//=============================================================================
playerStruct *GetPlayer() {
	return players;
}

D3DXVECTOR3 GetPlayerPosition(void)
{
	return players[0].m_Pos;
}


//=============================================================================
// Others
//=============================================================================

void ChangeStatus(playerStruct *player, int status) // change state and Anim , if can change state in same state : turn bool:b_changeAnimInSameState on
{
	player->status = status;
	if (player->statusPrev != player->status || player->b_changeAnimInSameState == true) {
		player->b_changeAnimInSameState = false;
		player->m_PatternAnim = m_SpritesData[player->status].Data[0];
		player->m_CountAnim = 0;
		player->animDataNo = 0;
	}
}

void InputUpdate(playerStruct & player)
{
	if (player.status == A_DEAD) return;
	// キー入力で移動 
	if (GetKeyboardPress(DIK_W) || GetKeyboardPress(DIK_UP) || IsButtonPressed(0, BUTTON_UP)) {
        #ifdef _DEBUG
		player.speedY -= player.accelerate;
        #endif // DEBUG	
	}
	if (GetKeyboardPress(DIK_A) || GetKeyboardPress(DIK_LEFT) || IsButtonPressed(0, BUTTON_LEFT)) {
		player.speedX -= player.accelerate;
		if (player.turnRight)player.turnRight = false;
	}
	if (GetKeyboardPress(DIK_S) || GetKeyboardPress(DIK_DOWN) || IsButtonPressed(0, BUTTON_DOWN)) {
		//player.speedY += player.accelerate;
	}
	if (GetKeyboardPress(DIK_D) || GetKeyboardPress(DIK_RIGHT) || IsButtonPressed(0, BUTTON_RIGHT)) {
		player.speedX += player.accelerate;
		if (!player.turnRight)player.turnRight = true;
	}

	if (IsMouseLeftTriggered() || IsButtonTriggered(0, BUTTON_X)|| GetKeyboardTrigger(DIK_Z)) {
		if (player.isThrowedCard == false) {
			ActionThrowCard(player);
		}
		else {
			SummonBackCard();
		}

	}
	if (IsMouseRightTriggered() || IsButtonTriggered(0, BUTTON_B) || GetKeyboardTrigger(DIK_X)) {
		if (!player.isThrowedCard)
		{
			if (player.m_playerAbility == p_wind) {
				ActionAttack(player);
				player.m_playerAbility = p_none;
			}
			if (player.m_playerAbility == p_Aqua) {
				player.speedY -= player.aquaJumpSpeed;
				player.m_playerAbility = p_none;
				SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y + player.m_h / 2.0f, 0.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), AquaJumpParticle, 20);
				SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y, 0.0f), D3DXVECTOR3(1.0f, 1.0f, 0.0f), starParticle, 10);
				PlaySound(SOUND_LABEL_SE_usecard);
				PlaySound(SOUND_LABEL_SE_aqua);
				PlaySound(SOUND_LABEL_SE_jump);
			}
		}
		else {
			if (CheckCardHasAbility() == true) {
				player.isThrowedCard = false;
				UseCardAbility();
			}
		}
	}


	//gravity
	if (GetKeyboardTrigger(DIK_SPACE) || IsButtonTriggered(0, BUTTON_A)) {
		if (player.isGround) {
			player.speedY = -player.jumpSpeed;
			SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y + player.m_h / 2.0f, 0.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), smoke, 4);
			PlaySound(SOUND_LABEL_SE_jump);
		}
		else {
			if (player.doubleJumpSwitch) {
				player.speedY = -player.jumpSpeed;
				SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y + player.m_h / 2.0f, 0.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), smoke, 4);
				player.doubleJumpSwitch = false;
				PlaySound(SOUND_LABEL_SE_jump);
			}
		}
	}


}

void PhysicsUpdate(playerStruct & player)
{
	
	//x
	if (player.status != A_DEAD) {
		if (player.speedX > 0)player.speedX -= player.decelerate;
		else if (player.speedX < 0)player.speedX += player.decelerate;
	}
	else { // Don't cross level after death
		if (player.m_Pos.x + (player.m_w / 2) >= SCREEN_WIDTH + SCREEN_WIDTH * GetLevelX()) player.speedX = 0.0f;
		else if (player.m_Pos.x - (player.m_w / 2) <= SCREEN_WIDTH * GetLevelX())player.speedX = 0.0f;

	}

	if (fabsf(player.speedX) < player.decelerate)player.speedX = 0.0f;


	if (player.speedX >= player.maxMoveSpeed)player.speedX = player.maxMoveSpeed;
	else if (player.speedX <= -player.maxMoveSpeed)player.speedX = -player.maxMoveSpeed;

	//y
	if (player.speedY > 0)player.speedY -= player.decelerate;
	else if (player.speedY < 0)player.speedY += player.decelerate;

	if (fabsf(player.speedY) < player.decelerate)player.speedY = 0.0f;

	//Speed limited
	if (player.speedY >= player.maxSpeedY)player.speedY = player.maxSpeedY;
	else if (player.speedY <= -player.maxSpeedY)player.speedY = -player.maxSpeedY;


	//TOTAL
	//if (player.speedX*player.speedX + player.speedY*player.speedY > player.maxSpeedY*player.maxSpeedY) {
	//	player.speedX *= player.maxSpeedY - ABS(player.speedY) / player.maxSpeedY; //(ABS(player.maxSpeedY*player.maxSpeedY - player.speedY*player.speedY)) / player.maxSpeedY*player.maxSpeedY;
	//}

	//Gravity
	if (player.gravityScale > 0.0f) {
		if (player.m_Pos.y < SCREEN_HEIGHT - player.m_h / 2)player.speedY += player.gravityScale;

		if (player.status != A_DEAD) {
			if (player.m_Pos.y > SCREEN_HEIGHT - player.m_h / 2) { // make sure not keep falling down
				player.m_Pos.y = SCREEN_HEIGHT - player.m_h / 2;
				player.speedY = 0.0f;
			}
		}
	}

	//Transport Level Detect
	if (IsTransportLevel(player.m_Pos.x, player.m_Pos.y)) {

	}


	//Ground Detect
	if (player.status != A_DEAD) {
		player.isGroundPrev = player.isGround;
		if (IsGround(player.m_Pos.x, player.m_Pos.y + TEXTURE_HEIGHT / 2) || //mid
			IsGround(player.m_Pos.x + TEXTURE_WIDTH / 3, player.m_Pos.y + TEXTURE_HEIGHT / 2) || // right
			IsGround(player.m_Pos.x - TEXTURE_WIDTH / 3, player.m_Pos.y + TEXTURE_HEIGHT / 2))   // left
		{ // ground                                  ↑壁衝突判定の後
			if (player.speedY > 0.0f) {
				// which point left on the ground
				//mid
				if (IsGround(player.m_Pos.x, player.m_Pos.y + TEXTURE_HEIGHT / 2) && IsGround(player.m_Pos.x - TEXTURE_WIDTH / 2, player.m_Pos.y + TEXTURE_HEIGHT / 2) && IsGround(player.m_Pos.x + TEXTURE_WIDTH / 2, player.m_Pos.y + TEXTURE_HEIGHT / 2)) {
					player.m_Pos.y = GetGroundCenterPos(player.m_Pos.x, player.m_Pos.y + TEXTURE_HEIGHT / 2).y - GetGroundTextureWidth() / 2 - TEXTURE_HEIGHT / 2;
				}
				//only left
				if (!IsGround(player.m_Pos.x, player.m_Pos.y + TEXTURE_HEIGHT / 2) && IsGround(player.m_Pos.x - TEXTURE_WIDTH / 3, player.m_Pos.y + TEXTURE_HEIGHT / 2)) {
					player.m_Pos.y = GetGroundCenterPos(player.m_Pos.x - TEXTURE_WIDTH / 3, player.m_Pos.y + TEXTURE_HEIGHT / 2).y - GetGroundTextureWidth() / 2 - TEXTURE_HEIGHT / 2;
				}
				//only right
				if (!IsGround(player.m_Pos.x, player.m_Pos.y + TEXTURE_HEIGHT / 2) && IsGround(player.m_Pos.x + TEXTURE_WIDTH / 3, player.m_Pos.y + TEXTURE_HEIGHT / 2)) {
					player.m_Pos.y = GetGroundCenterPos(player.m_Pos.x + TEXTURE_WIDTH / 3, player.m_Pos.y + TEXTURE_HEIGHT / 2).y - GetGroundTextureWidth() / 2 - TEXTURE_HEIGHT / 2;
				}

				player.speedY = 0.0f;
				player.isGround = true;
			}
			else {
				player.isGround = false;
			}
		}
		else {
			player.isGround = false;
		}

		if (IsGround(player.m_Pos.x, player.m_Pos.y - TEXTURE_HEIGHT / 2)) { //floor
			//player.m_Rot.z -= D3DXToRadian(6.0f); // Radian : 弧度 = Degree * PI/180.0f
			if (player.speedY < 0.0f)player.speedY = 0.0f;
		}
		if (IsGround(player.m_Pos.x - TEXTURE_WIDTH / 2, player.m_Pos.y)) { //KABE
			//player.m_Rot.z -= D3DXToRadian(6.0f); // Radian : 弧度 = Degree * PI/180.0f
			if (player.speedX < 0.0f)player.speedX = 0.0f;
		}
		if (IsGround(player.m_Pos.x + TEXTURE_WIDTH / 2, player.m_Pos.y)) { //KABE
			//player.m_Rot.z -= D3DXToRadian(6.0f); // Radian : 弧度 = Degree * PI/180.0f
			if (player.speedX > 0.0f)player.speedX = 0.0f;
		}


		//Spike Detect
		if (IsSpike(player.m_Pos.x, player.m_Pos.y)) { // ground
			player.speedY += -10.0f; // 飛
			playerGetElimated(player);
		}

		//==========   地面に落ちった
		if (player.isGround == true && player.isGroundPrev == false && !player.m_isElimated) {
			SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y + player.m_h / 2.0f, 0.0f), D3DXVECTOR3(2.0f, -0.25f, 0.0f), smoke, 2);
			player.doubleJumpSwitch = true;
		}


	}

	//Update Speed
	player.m_Pos.x += player.speedX;
	player.m_Pos.y += player.speedY;

	if (player.status == A_DEAD) {
		player.m_Rot.z += D3DXToRadian(6.0f);
	}
}

void CollisionUpdate(playerStruct & player)
{
	UpdateCollider(playersCollider2d, D3DXVECTOR2(player.m_Pos.x, player.m_Pos.y), 0.0f);
	AddColliderInPool(playersCollider2d, 0); // updatePool
	if (IsCollision(playersCollider2d, GetColliderInPool(1))) {
		//player.m_Rot.z += D3DXToRadian(6.0f); // Radian : 弧度 = Degree * PI/180.0f
	}

	
}

void playerStatusUpdate(playerStruct & player) // & other
{
	player.statusPrev = player.status;

	if (player.status == A_Idle || player.status == A_Move || player.status == A_InAir) {
		if (fabsf(player.speedX) > 1.0f && fabsf(player.speedY) == 0.0f) {
			ChangeStatus(&player, A_Move);
		}
		if (fabsf(player.speedX) <= 1.0f && fabsf(player.speedY) == 0.0f) {
			ChangeStatus(&player, A_Idle);
		}

		if (fabsf(player.speedY) > 0.0f) {
			ChangeStatus(&player, A_InAir);
		}
	}

	if (player.status == A_ThrowCard && m_SpritesData[player.status].Data[player.animDataNo] == APT_COS) {
		if (fabsf(player.speedX) > 1.0f && fabsf(player.speedY) == 0.0f) {
			ChangeStatus(&player, A_Move);
		}
		if (fabsf(player.speedX) <= 1.0f && fabsf(player.speedY) == 0.0f) {
			ChangeStatus(&player, A_Idle);
		}

		if (fabsf(player.speedY) > 0.0f) {
			ChangeStatus(&player, A_InAir);
		}
	}

	if (player.status == A_Attack && m_SpritesData[player.status].Data[player.animDataNo] == APT_COS) {
		if (fabsf(player.speedX) > 1.0f && fabsf(player.speedY) == 0.0f) {
			ChangeStatus(&player, A_Move);
		}
		if (fabsf(player.speedX) <= 1.0f && fabsf(player.speedY) == 0.0f) {
			ChangeStatus(&player, A_Idle);
		}

		if (fabsf(player.speedY) > 0.0f) {
			ChangeStatus(&player, A_InAir);
		}
	}

	if (player.status == A_DEAD) {
		player.m_setUnUseTime -= 1;
		if (player.m_setUnUseTime <= 0) {
			player.m_Use = false;
			player.status = A_Idle;
		}
	}

	//==========================Other
	if (player.m_playerAbility == p_wind) {
		SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y, 0.0f), D3DXVECTOR3(0.0f , 0.0f, 0.0f), windParticle, 2);
	}
	if (player.m_playerAbility == p_Aqua) {
		SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), AquaParticle, 2);
	}

}

void AnimUpdate(playerStruct &player)
{			
	//アニメーション (SpritesSheets)
	player.m_CountAnim++;
	if ((player.m_CountAnim % ANIM_WAIT) == 0)
	{
		// パターンの切り替え
		player.animDataNo += 1;
		player.m_PatternAnim = m_SpritesData[player.status].Data[player.animDataNo];
		
		if (m_SpritesData[player.status].Data[player.animDataNo] == APT_LOOP) {
			player.animDataNo = 0;
			player.m_PatternAnim = m_SpritesData[player.status].Data[player.animDataNo];
		}
		else if (m_SpritesData[player.status].Data[player.animDataNo] == APT_STOPF) {
			player.animDataNo -= 1;
			player.m_PatternAnim = m_SpritesData[player.status].Data[player.animDataNo];
		}
		else if(m_SpritesData[player.status].Data[player.animDataNo] == APT_CWV) {
			player.animDataNo = 0;
		}
		else if (m_SpritesData[player.status].Data[player.animDataNo] == APT_COS) {
			player.m_PatternAnim = m_SpritesData[player.status].Data[player.animDataNo - 1]; // -1 in oder to not display the APT_COS(-4) anim to cause error (frame + 1)
		}

		player.m_CountAnim = 0;

	}

	// アニメーション (スピードによって変わる) ex:InAir
	if (player.status == A_InAir) {
		int InAirSpritesData = m_SpritesData[player.status].Data[player.animDataNo];
		if (player.speedY < 2.0f)player.m_PatternAnim = m_SpritesData[player.status].Data[0];
		if (player.speedY > 1.0f)player.m_PatternAnim = m_SpritesData[player.status].Data[1];
		if (player.speedY > 2.0f)player.m_PatternAnim = m_SpritesData[player.status].Data[2];
		if (player.speedY > 6.0f)player.m_PatternAnim = m_SpritesData[player.status].Data[3];
	}

	// アニメーションアクション （アニメトリガー） ex: Action Fire
	// Action Fire
	if (player.status == A_ThrowCard && m_SpritesData[player.status].Data[player.animDataNo] == 1) {
		if (player.b_AnimTActionFireCardTrigger) {
			AnimTActionFireCard(player, card);
			player.b_AnimTActionFireCardTrigger = false;
		}
	}
	else if (player.status == A_ThrowCard && m_SpritesData[player.status].Data[player.animDataNo] != 1) { // resume trigger
		player.b_AnimTActionFireCardTrigger = true;
	}

	if (player.status == A_Attack && m_SpritesData[player.status].Data[player.animDataNo] == 1) {
		if (player.b_AnimTActionFireCardTrigger) {
			SetBullet(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y, player.m_Pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), swordAppear, none);
			SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y, 0.0f), D3DXVECTOR3(1.0f, 1.0f, 0.0f), starParticle, 15);
			player.b_AnimTActionFireCardTrigger = false;
		}
	}
	else if (player.status == A_Attack && m_SpritesData[player.status].Data[player.animDataNo] != 1) { // resume trigger
		player.b_AnimTActionFireCardTrigger = true;
	}

	// ステップ粒子
	if (player.status == A_Move && m_SpritesData[player.status].Data[player.animDataNo] == 30 && player.b_AnimStepParticleSwitch) {
		SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y + player.m_h / 2.0f, 0.0f), D3DXVECTOR3(0.0f, -0.25f, 0.0f), smoke, 2);
		player.b_AnimStepParticleSwitch = false;
	}
	if (player.status == A_Move && m_SpritesData[player.status].Data[player.animDataNo] == 31 && !player.b_AnimStepParticleSwitch) {
		player.b_AnimStepParticleSwitch = true;
	}
	if (player.status == A_Move && m_SpritesData[player.status].Data[player.animDataNo] == 36 && player.b_AnimStepParticleSwitch) {
		SetParticle(D3DXVECTOR3(player.m_Pos.x, player.m_Pos.y + player.m_h / 2.0f, 0.0f), D3DXVECTOR3(0.0f, -0.25f, 0.0f), smoke, 2);
		player.b_AnimStepParticleSwitch = false;
	}
	if (player.status == A_Move && m_SpritesData[player.status].Data[player.animDataNo] == 37 && !player.b_AnimStepParticleSwitch) {
		player.b_AnimStepParticleSwitch = true;
	}

	// アニメーションで変数変える  ex: b_changeAnimInSameState -> after X frame , you can change to same Anim again
	if (player.status == A_ThrowCard && player.animDataNo == 3) {		// can change to same Anim again at frame "4 (3+1)"
		player.b_changeAnimInSameState = true;		
	}
	

}

void ActionThrowCard(playerStruct & player) { // only change states, the other action will start in Anim so write in "AnimUpdate"( アニメーションアクション)
	ChangeStatus(&player, A_ThrowCard);
}

void ActionAttack(playerStruct & player) { // only change states, the other action will start in Anim so write in "AnimUpdate"( アニメーションアクション)
	ChangeStatus(&player, A_Attack);
}

void AnimTActionFireCard(playerStruct & player, bulletType type) { // Fire Bullet
	float bd;
	if (player.turnRight)bd = 1.0f; else bd = -1.0f;
	if (player.m_playerAbility == p_wind) {
		SetBullet(D3DXVECTOR3(player.m_Pos.x + bd * TEXTURE_WIDTH / 3.5, player.m_Pos.y, player.m_Pos.z), D3DXVECTOR3(bd, 0.0f, 0.0f), type, wind);
	}
	if (player.m_playerAbility == p_Aqua) {
		SetBullet(D3DXVECTOR3(player.m_Pos.x + bd * TEXTURE_WIDTH / 3.5, player.m_Pos.y, player.m_Pos.z), D3DXVECTOR3(bd, 0.0f, 0.0f), type, Aqua);
	}
	if (player.m_playerAbility == p_none) {
	    SetBullet(D3DXVECTOR3(player.m_Pos.x + bd * TEXTURE_WIDTH / 3.5, player.m_Pos.y, player.m_Pos.z), D3DXVECTOR3(bd, 0.0f, 0.0f), type, none);
	}
	player.isThrowedCard = true;
    player.m_playerAbility = p_none;

}

void playerGetElimated(playerStruct &player)
{
	ChangeStatus(&player, A_DEAD);
	player.m_isElimated = true;
	// デス数
	//AddScore(1);
	AddDeathCount(1);
	PlaySound(SOUND_LABEL_SE_hit2);
}

void setPlayerReborn(playerStruct & player)
{
	player.m_Use = true;
	player.m_Pos = GetPlayerRebornSpot(GetLevelX());
	player.m_Rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	player.turnRight = true;
	player.turnUp = false;
	player.speedX = 0;
	player.speedY = 0;

	player.status = 0;
	player.statusPrev = 0;

	player.m_setUnUseTime = PLAYER_SETUNUSEDTIME;
	player.m_isElimated = false;

	player.m_playerAbility = p_none;
	player.isGround = false;
	player.isGroundPrev = false;
	player.doubleJumpSwitch = true;

	player.m_CountAnim = 0;
	player.m_PatternAnim = 0;

	player.animDataNo = 0;
	player.b_AnimTActionFireCardTrigger = true;
	player.b_changeAnimInSameState = false;
	player.b_AnimStepParticleSwitch = true;

	player.m_TexNo = 0;
	player.status = A_Idle;
	player.m_PatternAnim = m_SpritesData[player.status].Data[0];

}
