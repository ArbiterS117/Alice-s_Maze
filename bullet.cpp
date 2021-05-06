//=============================================================================
//
// バレット処理 [bullet.cpp]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#include "game.h"
#include "bullet.h"
#include "sprite.h"
#include "ground.h"
#include "particle.h"

#include "sound.h"

#include "player.h"
#include "enemy.h"

#include <queue>// for bub max
using namespace std;
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX					  (bulletTypesNum)		// テクスチャの数

#define ANIM_WAIT					  (4)		// 普通アニメーションの切り替わるWait値
#define REMOTE_WATER_SHOWUP_ANIM_WAIT (7)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void ReInit(bulletStruct &bullet);
void AnimUpdate(bulletStruct &bullet);

void StoneUpdate(bulletStruct &bullet);
void CardUpdate(bulletStruct &bullet);
void swordAppearUpdate(bulletStruct &bullet);
void swordUpdate(bulletStruct &bullet);
void RemoteSwordUpdate(bulletStruct &bullet);
void RemoteWaterShowUpUpdate(bulletStruct &bullet);
void RemoteWaterBubUpdate(bulletStruct &bullet);
void RemoteWaterHitUpdate(bulletStruct &bullet);
void characterRebornUpdate(bulletStruct &bullet);
void characterDesappearingUpdate(bulletStruct &bullet);
void enemyRebornUpdate(bulletStruct &bullet);
void blinkEffectUpdate(bulletStruct &bullet);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

queue<bulletStruct> bubQ;//泡の数量WATER_BUB_MAXNUM以下

//bulletTypeの順番
static char *g_TexturName[] = {
	"data/TEXTURE/bullet00.png",                      // 50,50		  stone
	"data/TEXTURE/Card(32x32).png",                   // 32,32		  card
	"data/TEXTURE/swordAppear(128x128).png",          // 128 , 128	  swordAppear
	"data/TEXTURE/sword(128x128).png",                // 128 , 128	  sword
	"data/TEXTURE/ALICE_SP1.png",                     // 550 , 400	  water       nouse
	"data/TEXTURE/ALICE_SP1.png",                     // 550 , 400	  remoteSword nouse
	"data/TEXTURE/remoteWaterShowUp(36x36).png",      // 36 , 36	  remoteWaterShowUp
	"data/TEXTURE/remoteWaterBub(36x36).png",         // 36 , 36	  remoteWaterBub
	"data/TEXTURE/remoteWaterHit(36x36).png",         // 36 , 36	  remoteWaterHit
	"data/TEXTURE/CharacterAppearing (96x96).png",    // 96 , 96      characterReborn
	"data/TEXTURE/CharacterDesappearing (96x96).png", // 96 , 96      characterDesappearing
	"data/TEXTURE/CharacterAppearing (96x96).png",    // 96 , 96      enemyReborn
	"data/TEXTURE/blinkEffect(32x32).png",            // 32 , 32      blinkEffect


};												  

struct bulletSpritesHWDataStruct {
	int width;
	int height;
	int TEXTURE_PATTERN_DIVIDE_X;
	int TEXTURE_PATTERN_DIVIDE_Y;
};

//g_TexturNameの順番
bulletSpritesHWDataStruct bulletSpritesHWDivData[bulletTypesNum] = {
	{50, 50, 1, 1},		   //stone
	{32, 32, 12, 1},	   //card
	{128, 128, 5, 1},	   //swordAppear
	{128, 128, 8, 1},	   //sword
	{0, 0, 0, 0},		   //water
	{550, 400, 1, 4},	   //remoteSword
	{36, 36, 4, 1},		   //remoteWaterShowUp
	{36, 36, 10, 1},	   //remoteWaterBub
	{36, 36, 4, 1},		   //remoteWaterHit
    {96, 96, 7, 1},		   //characterReborn
    {96, 96, 7, 1},        //characterDesappearing
	{96, 96, 7, 1},		   //enemyReborn
	{32, 32, 6, 1}         //blinkEffect

};

static bulletStruct g_Bullet[BULLET_MAX];							// バレット構造体


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBullet(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
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


	// バレット構造体の初期化
	for (int i = 0; i < BULLET_MAX; i++)
	{
		g_Bullet[i].ID = i;
		g_Bullet[i].use = false;	// 未使用（発射されていない弾）
		g_Bullet[i].w = bulletSpritesHWDivData[0].width;
		g_Bullet[i].h = bulletSpritesHWDivData[0].height;
		g_Bullet[i].pos = D3DXVECTOR3(300, 300.0f, 0.0f);
		g_Bullet[i].rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		
		g_Bullet[i].speed = D3DXVECTOR3(0.0f, BULLET_SPEED, 0.0f);	// 移動量を初期化
		g_Bullet[i].turnRight = true;

		g_Bullet[i].bullettype = stone;
		g_Bullet[i].cardColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		g_Bullet[i].cardability = none;
		g_Bullet[i].isSummonBack = false;
		g_Bullet[i].abilityRemoteSwitch = false;
		g_Bullet[i].liveTime = 0.0f;
		g_Bullet[i].timer = 0.0f;
	}

	// バレットアニメーション 可以做成腳色版本 : AnimNo. 
	for (int i = 0; i < BULLET_MAX; i++)
	{
		g_Bullet[i].countAnim = 0;
		g_Bullet[i].patternAnim = 0;
		g_Bullet[i].texNo = 1;
		g_Bullet[i].TEXTURE_PATTERN_DIVIDE_X = 1;
		g_Bullet[i].TEXTURE_PATTERN_DIVIDE_Y = 4;
		g_Bullet[i].ANIM_PATTERN_NUM = g_Bullet[i].TEXTURE_PATTERN_DIVIDE_X * g_Bullet[i].TEXTURE_PATTERN_DIVIDE_Y;
		
		g_Bullet[i].animDataNo = 0;
		
	}

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBullet(void)
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
void UpdateBullet(void)
{
	//泡の数量WATER_BUB_MAXNUM以下
	if (bubQ.size() > BULLET_WATER_BUB_MAXNUM) {
		g_Bullet[bubQ.front().ID].use = false;
		SetBullet(D3DXVECTOR3(bubQ.front().pos.x, bubQ.front().pos.y, bubQ.front().pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), remoteWaterHit);
		bubQ.pop();
	}

	
	int bulletCount = 0;				// 処理したバレットの数 debug

	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].use == true)	// このバレットが使われている？
		{								// Yes
			if (IsStoppingFrame()) break;
			AnimUpdate(g_Bullet[i]);

			// バレットの移動処理
			switch (g_Bullet[i].bullettype)
			{
			case stone:	StoneUpdate(g_Bullet[i]); break;
			case card: CardUpdate(g_Bullet[i]);	break;
			case swordAppear: swordAppearUpdate(g_Bullet[i]); break;
			case sword: swordUpdate(g_Bullet[i]);	break;
			case water:	break;
			case remoteSword: RemoteSwordUpdate(g_Bullet[i]); break;
			case remoteWaterShowUp:	RemoteWaterShowUpUpdate(g_Bullet[i]); break;
			case remoteWaterBub: RemoteWaterBubUpdate(g_Bullet[i]);	break;
			case remoteWaterHit: RemoteWaterHitUpdate(g_Bullet[i]);	break;
			case characterReborn: characterRebornUpdate(g_Bullet[i]); break;
			case characterDesappearing: characterDesappearingUpdate(g_Bullet[i]); break;
			case enemyReborn: enemyRebornUpdate(g_Bullet[i]); break;
			case blinkEffect: blinkEffectUpdate(g_Bullet[i]); break;

			default: break;
			}
			

			// 画面外まで進んだ？
			//if (g_Bullet[i].pos.y < -TEXTURE_HEIGHT / 2)	// 自分の大きさを考慮して画面外か判定している
			//{
			//	g_Bullet[i].use = false;
			//}

			//if (g_Bullet[i].pos.x < -TEXTURE_WIDTH / 2 || g_Bullet[i].pos.x > TEXTURE_WIDTH / 2 + SCREEN_WIDTH)	// 自分の大きさを考慮して画面外か判定している
			//{
			//	g_Bullet[i].use = false;
			//}

			
			#ifdef _DEBUG
			bulletCount++; // debug
			#endif
		}
		
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " BX:%.2f BY:%.2f", g_Bullet[0].Pos.x, g_Bullet[0].Pos.y);
	char *str = GetDebugStr();
	sprintf(&str[strlen(str)], " BC:%d", bulletCount);

#endif

	
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawBullet(void)
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

	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].use == true)	// このバレットが使われている？
		{								// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Bullet[i].texNo]);

			//バレットの位置やテクスチャー座標を反映
			float px = g_Bullet[i].pos.x - GetLevelX() * SCREEN_WIDTH;	// バレットの表示位置X
			float py = g_Bullet[i].pos.y - GetLevelY() * SCREEN_HEIGHT;	// バレットの表示位置Y
			if (px < SCREEN_WIDTH * 1.2f || px > 0 - SCREEN_WIDTH * 0.2f)  // no need to draw 
			{
				float pw = g_Bullet[i].w;		// バレットの表示幅
				float ph = g_Bullet[i].h;		// バレットの表示高さ

				float tw = 1.0f / g_Bullet[i].TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
				float th = 1.0f / g_Bullet[i].TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
				float tx = (float)(g_Bullet[i].patternAnim % g_Bullet[i].TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
				float ty = (float)(g_Bullet[i].patternAnim / g_Bullet[i].TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				if (g_Bullet[i].turnRight) {
				SetSpriteColorRotation(g_VertexBuffer,
					px, py, pw, ph,
					tx, ty, tw, th,
					g_Bullet[i].cardColor,
					g_Bullet[i].rot.z);
				}
				else {
					SetSpriteColorRotation(g_VertexBuffer,
						px, py, pw, ph,
						tx, ty, -tw, th,
						g_Bullet[i].cardColor,
						g_Bullet[i].rot.z);
				}
				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}

}

//=============================================================================
// バレット構造体の先頭アドレスを取得
//=============================================================================
bulletStruct *GetBullet(void)
{
	return &g_Bullet[0];
}

//=============================================================================
// バレットの発射と自動回収設定
//=============================================================================
void SetBullet(D3DXVECTOR3 pos, D3DXVECTOR3 dir, bulletType bType)
{
	// もし未使用の弾が無かったら発射しない( =これ以上撃てないって事 )
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].use == false)		// 未使用状態のバレットを見つける
		{
			g_Bullet[i].use = true;			// 使用状態へ変更する
			g_Bullet[i].pos = pos;			// 座標をセット
			g_Bullet[i].bullettype = bType;

			if(dir.x > 0)g_Bullet[i].turnRight = true;
			else g_Bullet[i].turnRight = false;

			// ReInit
			ReInit(g_Bullet[i]);
		
			switch (g_Bullet[i].bullettype)
			{
			case stone:
				g_Bullet[i].speed = D3DXVECTOR3(dir.x * BULLET_SPEED, dir.y * BULLET_SPEED, 0.0f);
				PlaySound(SOUND_LABEL_SE_hit);
				break;

			case card:
				g_Bullet[i].speed = D3DXVECTOR3(dir.x * BULLET_CARD_FORWARD_SPEED, dir.y * BULLET_CARD_FORWARD_SPEED, 0.0f);
				PlaySound(SOUND_LABEL_SE_throwcard);
				break;
			case swordAppear: PlaySound(SOUND_LABEL_SE_usecard); break;
			case sword: PlaySound(SOUND_LABEL_SE_sword); break;
			case water: break;
			case remoteSword: break;
			case remoteWaterShowUp: PlaySound(SOUND_LABEL_SE_usecard); break;
			case remoteWaterBub: bubQ.push(g_Bullet[i]);  break;
			case remoteWaterHit: break;
			case characterReborn: PlaySound(SOUND_LABEL_SE_usecard); break;
			case characterDesappearing: break;
			case enemyReborn: break;
			case blinkEffect: break;
			default: break;
			}
			
			return;							// 1発セットしたので終了する
		}
	}
}

void SetBullet(D3DXVECTOR3 pos, D3DXVECTOR3 dir, bulletType bType, int enemynum)
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].use == false)		// 未使用状態のバレットを見つける
		{
			g_Bullet[i].enemynum = enemynum;
			break;
		}
	}
	SetBullet(pos, dir, bType);
	
}

void SetBullet(D3DXVECTOR3 pos, D3DXVECTOR3 dir, bulletType bType, cardAbility ability)
{
	int num = -1; // save the num of bullet
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].use == false)		// 未使用状態のバレットを見つける
		{
			num = i;
			break;
		}
	}
	if (num != -1) {
		SetBullet(pos, dir, bType);
		g_Bullet[num].cardability = ability;
		if (g_Bullet[num].cardability == wind)g_Bullet[num].cardColor = D3DXCOLOR(0.3f, 1.0f, 0.3f, 1.0f);
		if (g_Bullet[num].cardability == Aqua)g_Bullet[num].cardColor = D3DXCOLOR(0.3f, 0.3f, 1.0f, 1.0f);
	}
}


void SummonBackCard()
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		g_Bullet[i].isSummonBack = true;
	}
}

bool CheckCardHasAbility() {
	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].cardability != none) return true;
	}
	return false;
}

void UseCardAbility()
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		g_Bullet[i].abilityRemoteSwitch = true;
	}
}

void setBubQ(int id)
{
	bulletStruct Temp;
	int sizeTemp = bubQ.size(); // bubQ.size() 変える可能性がある

	for (int i = 0; i < sizeTemp; i++) {
		if (bubQ.size() != 0) {
			if (bubQ.front().ID == id) {
				bubQ.pop();
			}
			else {
				Temp = bubQ.front();
				bubQ.pop();
				bubQ.push(Temp);
			}
		}
	}
	

}


//=============================================================================
// Others
//=============================================================================
void ReInit(bulletStruct &bullet) {
	int hwDivData = bullet.bullettype;
	bullet.w = bulletSpritesHWDivData[hwDivData].width;
	bullet.h = bulletSpritesHWDivData[hwDivData].height;
	bullet.rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	bullet.texNo = hwDivData;
	bullet.TEXTURE_PATTERN_DIVIDE_X = bulletSpritesHWDivData[hwDivData].TEXTURE_PATTERN_DIVIDE_X;
	bullet.TEXTURE_PATTERN_DIVIDE_Y = bulletSpritesHWDivData[hwDivData].TEXTURE_PATTERN_DIVIDE_Y;
	bullet.ANIM_PATTERN_NUM = bullet.TEXTURE_PATTERN_DIVIDE_X * bullet.TEXTURE_PATTERN_DIVIDE_Y;

	bullet.timer = 0;
	bullet.liveTime = 0;

	bullet.countAnim = 0;
	bullet.patternAnim = 0;
	bullet.cardColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	bullet.isSummonBack = false;
	bullet.abilityRemoteSwitch = false;
	bullet.cardability = none;

	bullet.speed = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}


void AnimUpdate(bulletStruct &bullet) {

	// アニメーション  
	bullet.countAnim++;
	if (bullet.bullettype == remoteWaterShowUp || bullet.bullettype == remoteWaterBub || bullet.bullettype == remoteWaterHit) {
		if ((bullet.countAnim % REMOTE_WATER_SHOWUP_ANIM_WAIT) == 0) {
			bullet.patternAnim = (bullet.patternAnim + 1) % bullet.ANIM_PATTERN_NUM;
		}
		return;
	}

	if ((bullet.countAnim % ANIM_WAIT) == 0)
	{
		// パターンの切り替え
		bullet.patternAnim = (bullet.patternAnim + 1) % bullet.ANIM_PATTERN_NUM;
	}

}

void StoneUpdate(bulletStruct & bullet)
{
	bullet.pos += bullet.speed;

	//liveTime
	bullet.liveTime += 1;
	if (bullet.liveTime >= BULLET_STONE_LIVETIME) {
		bullet.use = false;
	}

}

void CardUpdate(bulletStruct & bullet)
{
	bullet.timer += 1;
	bullet.pos += bullet.speed;

	//turnRight or not
	if (bullet.speed.x > 0.0f)bullet.turnRight = true;
	if (bullet.speed.x < 0.0f)bullet.turnRight = false;


	// 前にダッシュ
	if (bullet.timer > BULLET_CARD_FORWARD_TIME) {

		// now for axis X only
		if (bullet.speed.x > 0.0f) bullet.speed.x -= BULLET_CARD_DECELERATE;
		else if (bullet.speed.x < 0.0f) bullet.speed.x += BULLET_CARD_DECELERATE;

		if (fabsf(bullet.speed.x) < BULLET_CARD_DECELERATE) {
			bullet.speed = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			bullet.rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		}
	}

	// 粒子&イフェクト
	if (fabsf(bullet.speed.x) > BULLET_CARD_DECELERATE) {
		if(bullet.cardability == none)SetParticle(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, 0.0f), D3DXVECTOR3(-bullet.speed.x, -bullet.speed.y, 0.0f), cardParticle, 3);
		if (bullet.cardability == wind)SetParticle(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, 0.0f), D3DXVECTOR3(-bullet.speed.x, -bullet.speed.y, 0.0f), windParticle, 3);
		if (bullet.cardability == Aqua)SetParticle(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, 0.0f), D3DXVECTOR3(-bullet.speed.x, -bullet.speed.y, 0.0f), AquaParticle, 3);
		bullet.rot = D3DXVECTOR3(0.0f, 0.0f, D3DXToRadian(90.0f));
	}

	//自動回収
	if (bullet.isSummonBack == true) {
		playerStruct *players = GetPlayer();
		D3DXVECTOR3 backDir = players[0].m_Pos - bullet.pos;
		//D3DXVec3Normalize(&backDir, &backDir);

		bullet.speed = backDir / 25.0f;

		//Limit speed


	}

	//アビリティ
	if (bullet.cardability == wind && bullet.abilityRemoteSwitch) {
		if(bullet.turnRight)SetBullet(D3DXVECTOR3(bullet.pos.x , bullet.pos.y, bullet.pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), swordAppear);
		else SetBullet(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, bullet.pos.z), D3DXVECTOR3(-1.0f, 0.0f, 0.0f), swordAppear);
		SetParticle(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, bullet.pos.z), D3DXVECTOR3(1.0f, 1.0f, 0.0f), starParticle, 15);
		bullet.abilityRemoteSwitch = false;
		bullet.use = false;
	}
	if (bullet.cardability == Aqua && bullet.abilityRemoteSwitch) {
		SetBullet(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, bullet.pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), remoteWaterShowUp);
		SetParticle(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, bullet.pos.z), D3DXVECTOR3(1.0f, 1.0f, 0.0f), starParticle, 15);
		bullet.abilityRemoteSwitch = false;
		bullet.use = false;
	}

}

void swordAppearUpdate(bulletStruct &bullet) {
	if (!IsStoppingFrame()) {
		//liveTime
		bullet.liveTime += 1;
		if (bullet.liveTime >= BULLET_SWORD_APPEAR_LIVETIME) {
			bullet.use = false;
			SetBullet(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, bullet.pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), sword);
		}
	}
}

void swordUpdate(bulletStruct &bullet) {
	if (!IsStoppingFrame()) {
		//liveTime
		bullet.liveTime += 1;
		if (bullet.liveTime >= BULLET_SWORD_LIVETIME) {
			bullet.use = false;
		}
	}
}

void RemoteSwordUpdate(bulletStruct & bullet)
{
	if (!IsStoppingFrame()) {
		//liveTime
		bullet.liveTime += 1;
		if (bullet.liveTime >= BULLET_REMOTE_SWORD_LIVETIME) {
			bullet.use = false;
		}
	}
}

void RemoteWaterShowUpUpdate(bulletStruct &bullet)
{
	if (!IsStoppingFrame()) {
		bullet.liveTime += 1;
		if (bullet.liveTime >= BULLET_REMOTE_WATER_SHOWUP_LIVETIME) {
			bullet.use = false;
			SetBullet(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, bullet.pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), remoteWaterBub);
		}
	}
}

void RemoteWaterBubUpdate(bulletStruct & bullet)
{
	if (!IsStoppingFrame()) {
		bullet.liveTime += 1;
		if (bullet.liveTime >= BULLET_REMOTE_WATER_BUB_LIVETIME) {
			bullet.use = false;
			bubQ.pop();
			SetBullet(D3DXVECTOR3(bullet.pos.x, bullet.pos.y, bullet.pos.z), D3DXVECTOR3(1.0f, 0.0f, 0.0f), remoteWaterHit);
		}
	}
}

void RemoteWaterHitUpdate(bulletStruct & bullet)
{
	if (!IsStoppingFrame()) {
		bullet.liveTime += 1;
		if (bullet.liveTime >= BULLET_REMOTE_WATER_HIT_LIVETIME) {
			bullet.use = false;
		}
	}
}

void characterRebornUpdate(bulletStruct &bullet)
{
	bullet.liveTime += 1;
	if (bullet.liveTime >= BULLET_REMOTE_C_REBORN_LIVETIME) {
		bullet.use = false;
	    playerStruct *player = GetPlayer();
		setPlayerReborn(player[0]);
	}
	
}

void characterDesappearingUpdate(bulletStruct &bullet)
{
	
	bullet.liveTime += 1;
	if (bullet.liveTime >= BULLET_REMOTE_C_DIS_LIVETIME) {
		bullet.use = false;
	}
	
}

void enemyRebornUpdate(bulletStruct &bullet)
{
	bullet.liveTime += 1;
	if (bullet.liveTime >= BULLET_REMOTE_C_REBORN_LIVETIME) {
		bullet.use = false;
		enemyStruct *enemy = GetEnemy();
		setEnemyReborn(enemy[bullet.enemynum]);
	}

}

void blinkEffectUpdate(bulletStruct &bullet)
{
	bullet.liveTime += 1;
	if (bullet.liveTime >= BULLET_BLINK_EF_LIVETIME) {
		bullet.use = false;
	}
}