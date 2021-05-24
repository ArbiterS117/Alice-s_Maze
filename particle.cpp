//=============================================================================
//
// 粒子処理 [particle.cpp]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#pragma once

#include "particle.h"
#include "sprite.h"
#include "ground.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX					 (2) // テクスチャの数

#define ANIM_WAIT					 (4) // アニメーションの切り替わるWait値

#define SET_CONTINUE_PARTICLE_CDTIME (7) // 続ている粒子生成CD
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void ReInitparticle(particleStruct &particle);
void AnimUpdate(particleStruct &particle);

void SmokeUpdate(particleStruct &particle);
void CardUpdate(particleStruct &particle);
void AquaJumpUpdate(particleStruct &particle);
void StarUpdate(particleStruct &particle);
//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

//particleTypeの順番
static char *g_TexturName[] = {
	"data/TEXTURE/Dust Particle.png",  //16,16
	"data/TEXTURE/star.png"            //16,16
};

struct particleSpritesHWDataStruct {
	int width;
	int height;
	int TEXTURE_PATTERN_DIVIDE_X;
	int TEXTURE_PATTERN_DIVIDE_Y;
};

//g_TexturNameの順番
particleSpritesHWDataStruct particleSpritesHWDivData[particleTypesNum] = {    //データベース
	{16, 16, 1, 1},
	{16, 16, 1, 1}
};

static particleStruct g_Particle[PARTICLE_MAX];				// 粒子構造体

int setContinueParticleCDTimer = 0;
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitParticle(void)
{
	srand(time(NULL));

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


	// 粒子構造体の初期化
	for (int i = 0; i < PARTICLE_MAX; i++)
	{
		g_Particle[i].m_use = false;	// 未使用（発射されていない弾）
		g_Particle[i].m_w = particleSpritesHWDivData[0].width;
		g_Particle[i].m_h = particleSpritesHWDivData[0].height;
		g_Particle[i].m_pos = D3DXVECTOR3(0.0f , 0.0f, 0.0f);
		g_Particle[i].m_rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		g_Particle[i].speed = D3DXVECTOR3(0.0f, PARTICLE_SPEED, 0.0f);	// 移動量を初期化
		g_Particle[i].turnRight = true;
		g_Particle[i].gravityScale = 0.5f;
		g_Particle[i].particletype = smoke;
		g_Particle[i].Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		
		g_Particle[i].timer = 0.0f;
		g_Particle[i].liveTime = 0.0f;
	}

	// 粒子アニメーション 
	for (int i = 0; i < PARTICLE_MAX; i++)
	{
		g_Particle[i].countAnim = 0;
		g_Particle[i].patternAnim = 0;
		g_Particle[i].texNo = 0;
		g_Particle[i].TEXTURE_PATTERN_DIVIDE_X = particleSpritesHWDivData[0].TEXTURE_PATTERN_DIVIDE_X;
		g_Particle[i].TEXTURE_PATTERN_DIVIDE_Y = particleSpritesHWDivData[0].TEXTURE_PATTERN_DIVIDE_Y;
		g_Particle[i].ANIM_PATTERN_NUM = g_Particle[i].TEXTURE_PATTERN_DIVIDE_X * g_Particle[i].TEXTURE_PATTERN_DIVIDE_Y;

		g_Particle[i].animDataNo = 0;

	}

	// other
	setContinueParticleCDTimer = 0;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitParticle(void)
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
void UpdateParticle(void)
{
	if (setContinueParticleCDTimer > 0) {
		setContinueParticleCDTimer -= 1;
	}

	int particleCount = 0;				// 処理した粒子の数 debug

	for (int i = 0; i < PARTICLE_MAX; i++)
	{
		if (g_Particle[i].m_use == true)	// この粒子が使われている？
		{								// Yes
			AnimUpdate(g_Particle[i]);

			// 粒子の移動処理
			switch (g_Particle[i].particletype)
			{
			case smoke:
				SmokeUpdate(g_Particle[i]);
				break;
			case cardParticle:
				CardUpdate(g_Particle[i]);
				break;
			case windParticle:
				CardUpdate(g_Particle[i]);
				break;
			case AquaParticle:
				CardUpdate(g_Particle[i]);
				break;
			case AquaJumpParticle:
				AquaJumpUpdate(g_Particle[i]);
				break;
			case starParticle:
				StarUpdate(g_Particle[i]);
				break;
			default:
				break;
			}


			// 画面外まで進んだ？
			//if (g_Particle[i].pos.y < -TEXTURE_HEIGHT / 2)	// 自分の大きさを考慮して画面外か判定している
			//{
			//	g_Particle[i].m_use = false;
			//}

			//if (g_Particle[i].pos.x < -TEXTURE_WIDTH / 2 || g_Particle[i].pos.x > TEXTURE_WIDTH / 2 + SCREEN_WIDTH)	// 自分の大きさを考慮して画面外か判定している
			//{
			//	g_Particle[i].m_use = false;
			//}



			particleCount++; // debug
		}
		else {
			//g_Particle[i].patternAnim = 0;
		}
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " BX:%.2f BY:%.2f", g_Particle[0].Pos.x, g_Particle[0].Pos.y);
	char *str = GetDebugStr();
	sprintf(&str[strlen(str)], " BC:%d", particleCount);

#endif


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawParticle(void)
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

	for (int i = 0; i < PARTICLE_MAX; i++)
	{
		if (g_Particle[i].m_use == true)	// この粒子が使われている？
		{								// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Particle[i].texNo]);

			//粒子の位置やテクスチャー座標を反映
			float px = g_Particle[i].m_pos.x - GetLevelX() * SCREEN_WIDTH;	// 粒子の表示位置X
			float py = g_Particle[i].m_pos.y - GetLevelY() * SCREEN_HEIGHT;	// 粒子の表示位置Y
			if (px < SCREEN_WIDTH * 1.2f || px > 0 - SCREEN_WIDTH * 0.2f)  // no need to draw 
			{
				float pw = g_Particle[i].m_w;		// 粒子の表示幅
				float ph = g_Particle[i].m_h;		// 粒子の表示高さ

				float tw = 1.0f / g_Particle[i].TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
				float th = 1.0f / g_Particle[i].TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
				float tx = (float)(g_Particle[i].patternAnim % g_Particle[i].TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
				float ty = (float)(g_Particle[i].patternAnim / g_Particle[i].TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				if (g_Particle[i].turnRight) {
					SetSpriteColorRotation(g_VertexBuffer,
						px, py, pw, ph,
						tx, ty, tw, th,
						g_Particle[i].Color,
						g_Particle[i].m_rot.z);
				}
				else {
					SetSpriteColorRotation(g_VertexBuffer,
						px, py, pw, ph,
						tx, ty, -tw, th,
						g_Particle[i].Color,
						g_Particle[i].m_rot.z);
				}
				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}

}

//=============================================================================
// 粒子構造体の先頭アドレスを取得
//=============================================================================
particleStruct *GetParticle(void)
{
	return &g_Particle[0];
}

//=============================================================================
// 粒子の発射と自動回収設定
//=============================================================================
void SetParticle(D3DXVECTOR3 pos, D3DXVECTOR3 dir, particleType pType, int particlenum)
{
	bool isreturn = false;
	if (setContinueParticleCDTimer != 0 && pType == cardParticle) isreturn = true;
	if (setContinueParticleCDTimer == 0 && pType == cardParticle) setContinueParticleCDTimer = SET_CONTINUE_PARTICLE_CDTIME;
	if (setContinueParticleCDTimer != 0 && pType == windParticle) isreturn = true;
	if (setContinueParticleCDTimer == 0 && pType == windParticle) setContinueParticleCDTimer = SET_CONTINUE_PARTICLE_CDTIME;
	if (setContinueParticleCDTimer != 0 && pType == AquaParticle) isreturn = true;
	if (setContinueParticleCDTimer == 0 && pType == AquaParticle) setContinueParticleCDTimer = SET_CONTINUE_PARTICLE_CDTIME;
	if (isreturn) return;

	for (int n = 0; n < particlenum; n++) {         // 生成したい粒子の数
		// もし未使用の粒子が無かったら生成しない( =これ以上生成てないって事 )
		for (int i = 0; i < PARTICLE_MAX; i++)
		{
			if (g_Particle[i].m_use == false)		// 未使用状態の粒子を見つける
			{
				g_Particle[i].m_use = true;			// 使用状態へ変更する
				g_Particle[i].m_pos = pos;			// 座標をセット
				g_Particle[i].particletype = pType;

				if (dir.x > 0)g_Particle[i].turnRight = true;
				else g_Particle[i].turnRight = false;

				// ReInit
				ReInitparticle(g_Particle[i]);

				// type
				int randDirx = rand() % 25;
				int randDiry = rand() % 25;
				switch (g_Particle[i].particletype)
				{
				case smoke:
					g_Particle[i].m_w = particleSpritesHWDivData[smoke].width;
					g_Particle[i].m_h = particleSpritesHWDivData[smoke].height;
					g_Particle[i].texNo = smoke;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_X = particleSpritesHWDivData[smoke].TEXTURE_PATTERN_DIVIDE_X;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_Y = particleSpritesHWDivData[smoke].TEXTURE_PATTERN_DIVIDE_Y;
					g_Particle[i].gravityScale = 0.5f;

					randDirx -= 12;                    //           Swift
					g_Particle[i].speed = D3DXVECTOR3(((float)randDirx / 5.0f) * PARTICLE_SPEED + (randDirx / 12.0f) * dir.x * PARTICLE_SPEED, dir.y * ((float)randDiry/6.0f) * PARTICLE_SPEED , 0.0f);
					break;

				case cardParticle:
					g_Particle[i].m_w = particleSpritesHWDivData[cardParticle].width;
					g_Particle[i].m_h = particleSpritesHWDivData[cardParticle].height;
					g_Particle[i].texNo = cardParticle;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_X = particleSpritesHWDivData[cardParticle].TEXTURE_PATTERN_DIVIDE_X;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_Y = particleSpritesHWDivData[cardParticle].TEXTURE_PATTERN_DIVIDE_Y;
					g_Particle[i].gravityScale = 0.0f;

					g_Particle[i].Color = D3DXCOLOR(0.95f,0.95f,0.7f,0.7f);
					g_Particle[i].speed = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
					break;
				case windParticle:
					g_Particle[i].m_w = particleSpritesHWDivData[cardParticle].width;
					g_Particle[i].m_h = particleSpritesHWDivData[cardParticle].height;
					g_Particle[i].texNo = cardParticle;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_X = particleSpritesHWDivData[cardParticle].TEXTURE_PATTERN_DIVIDE_X;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_Y = particleSpritesHWDivData[cardParticle].TEXTURE_PATTERN_DIVIDE_Y;
					g_Particle[i].gravityScale = 0.0f;

					g_Particle[i].Color = D3DXCOLOR(0.3f, 0.95f, 0.3f, 0.7f);
					g_Particle[i].speed = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
					break;
				case AquaParticle:
					g_Particle[i].m_w = particleSpritesHWDivData[smoke].width;
					g_Particle[i].m_h = particleSpritesHWDivData[smoke].height;
					g_Particle[i].texNo = smoke;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_X = particleSpritesHWDivData[smoke].TEXTURE_PATTERN_DIVIDE_X;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_Y = particleSpritesHWDivData[smoke].TEXTURE_PATTERN_DIVIDE_Y;
					g_Particle[i].gravityScale = 0.0f;

					g_Particle[i].Color = D3DXCOLOR(0.3f, 0.3f, 0.95f, 0.7f);
					g_Particle[i].speed = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
					break;

				case AquaJumpParticle:
					g_Particle[i].m_w = particleSpritesHWDivData[smoke].width;
					g_Particle[i].m_h = particleSpritesHWDivData[smoke].height;
					g_Particle[i].texNo = smoke;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_X = particleSpritesHWDivData[smoke].TEXTURE_PATTERN_DIVIDE_X;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_Y = particleSpritesHWDivData[smoke].TEXTURE_PATTERN_DIVIDE_Y;
					g_Particle[i].gravityScale = 0.2f;
					g_Particle[i].Color = D3DXCOLOR(0.3f, 0.3f, 0.95f, 0.8f);

					randDirx -= 12;                    //           Swift
					g_Particle[i].speed = D3DXVECTOR3(((float)randDirx / 5.0f) * PARTICLE_SPEED + (randDirx / 12.0f) * dir.x * PARTICLE_SPEED, ((float)randDirx / 5.0f) * PARTICLE_SPEED + dir.y * ((float)randDiry / 6.0f) * PARTICLE_SPEED, 0.0f);
					break;

				case starParticle:
					g_Particle[i].m_w = particleSpritesHWDivData[cardParticle].width;
					g_Particle[i].m_h = particleSpritesHWDivData[cardParticle].height;
					g_Particle[i].texNo = cardParticle;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_X = particleSpritesHWDivData[cardParticle].TEXTURE_PATTERN_DIVIDE_X;
					g_Particle[i].TEXTURE_PATTERN_DIVIDE_Y = particleSpritesHWDivData[cardParticle].TEXTURE_PATTERN_DIVIDE_Y;
					g_Particle[i].gravityScale = 0.5f;
					g_Particle[i].Color = D3DXCOLOR(0.95f, 0.95f, 0.7f, 0.7f);

					randDiry -= 12;
					randDirx -= 12;                    //           Swift
					g_Particle[i].speed = D3DXVECTOR3(((float)randDirx / 5.0f) * PARTICLE_SPEED * 1.5f , ((float)randDiry / 5.0f) * PARTICLE_SPEED * 1.5f, 0.0f);
					break;

				default:
					break;
				}
				break;
			}
		}
	}
	return;							// 1セットしたので終了する
}

//=============================================================================
// Others
//=============================================================================
void ReInitparticle(particleStruct &particle) {
	particle.timer = 0;
	particle.liveTime = 0;

	particle.countAnim = 0;
	particle.patternAnim = 0;
	particle.Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	particle.m_rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

}


void AnimUpdate(particleStruct &particle) {
	// アニメーション  
	particle.countAnim++;
	if ((particle.countAnim % ANIM_WAIT) == 0)
	{
		// パターンの切り替え
		particle.patternAnim = (particle.patternAnim + 1) % particle.ANIM_PATTERN_NUM;
	}

}

void SmokeUpdate(particleStruct & particle)
{
	particle.m_pos += particle.speed;
	particle.Color.a = (SMOKE_PARTICLE_LIVETIME - particle.liveTime) / SMOKE_PARTICLE_LIVETIME + 0.3f;
	//liveTime
	particle.liveTime += 1;
	if (particle.liveTime >= SMOKE_PARTICLE_LIVETIME) {
		particle.liveTime = 0.0f;
		particle.m_use = false;
	}

}

void CardUpdate(particleStruct & particle)
{
	particle.m_pos += particle.speed;
	particle.Color.a = (CARD_PARTICLE_LIVETIME - particle.liveTime) / CARD_PARTICLE_LIVETIME - 0.3f;
	particle.m_rot.z += D3DXToRadian(6.0f);
	//liveTime
	particle.liveTime += 1;
	if (particle.liveTime >= CARD_PARTICLE_LIVETIME) {
		particle.liveTime = 0.0f;
		particle.m_use = false;
	}
}

void AquaJumpUpdate(particleStruct & particle)
{
	particle.speed.y += particle.gravityScale;
	particle.m_pos += particle.speed;
	particle.Color.a = (AQUAJUMP_PARTICLE_LIVETIME - particle.liveTime) / AQUAJUMP_PARTICLE_LIVETIME - 0.3f;
	particle.m_rot.z += D3DXToRadian(6.0f);
	//liveTime
	particle.liveTime += 1;
	if (particle.liveTime >= AQUAJUMP_PARTICLE_LIVETIME) {
		particle.liveTime = 0.0f;
		particle.m_use = false;
	}
}

void StarUpdate(particleStruct & particle)
{
	particle.m_pos += particle.speed;
	particle.Color.a = (STAR_PARTICLE_LIVETIME - particle.liveTime) / STAR_PARTICLE_LIVETIME + 0.2f;
	particle.m_rot.z += D3DXToRadian(6.0f);
	//liveTime
	particle.liveTime += 1;
	if (particle.liveTime >= STAR_PARTICLE_LIVETIME) {
		particle.liveTime = 0.0f;
		particle.m_use = false;
	}

}

