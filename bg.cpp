//=============================================================================
//
// 背景処理 [bg.cpp]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#include "bg.h"
#include "player.h"
#include "sprite.h"
#include "ground.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// キャラサイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)			// 
#define TEXTURE_MAX					(7)				// テクスチャの数

#define TEXTURE_WIDTH_BLOCK			(64)	        // ブロック背景サイズ
#define TEXTURE_HEIGHT_BLOCK		(64)	        // 
#define BLOCK_MOVE_TIME             (TEXTURE_HEIGHT_BLOCK * 2)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[] = {
	"data/TEXTURE/Blue.png", // blocktype 0
	"data/TEXTURE/Gray.png", //           1
	"data/TEXTURE/Green.png",//           2
	"data/TEXTURE/Brown.png",//           3...
	"data/TEXTURE/Pink.png",
	"data/TEXTURE/Purple.png",
	"data/TEXTURE/Yellow.png",
};


static bool						g_Use;						// true:使っている  false:未使用
static float					g_w, g_h;					// 幅と高さ
static D3DXVECTOR3				g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

// other
int bgblockMoveTimer = 0;
int bg_blocktype = 3;
int bg_nowlevelx = 0;
int bg_levelxPrev = 0;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBg(void)
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


	// プレイヤーの初期化
	g_Use = true;
	g_w = TEXTURE_WIDTH;
	g_h = TEXTURE_HEIGHT;
	g_Pos = D3DXVECTOR3(SCREEN_CENTER_X , SCREEN_CENTER_Y, 0.0f);
	g_TexNo = 0;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBg(void)
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
void UpdateBg(void)
{
	bgblockMoveTimer += 1;
	if (bgblockMoveTimer >= BLOCK_MOVE_TIME)bgblockMoveTimer = 0;

	//ブロックの種類変える
	bg_nowlevelx = GetLevelX();
	if (bg_nowlevelx != bg_levelxPrev) {
		bg_blocktype += 1;
		if (bg_blocktype > 6)bg_blocktype = 0;
	}

	bg_levelxPrev = GetLevelX();

	// スクロール処理
	//g_Pos.x = -GetPlayer()->m_Pos.x / 4.0f + SCREEN_CENTER_X * 1.25f;


#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawBg(void)
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

	for (int i = 0; i < 3; i++) {
		g_TexNo = i;
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

		// スコアの位置やテクスチャー座標を反映
		float px = g_Pos.x;		// プレイヤーの表示位置X
		float py = g_Pos.y;		// プレイヤーの表示位置Y
		float pw = g_w;			// プレイヤーの表示幅
		float ph = g_h;			// プレイヤーの表示高さ

		float tw = 1.0f;		// テクスチャの幅
		float th = 1.0f;		// テクスチャの高さ
		float tx = 0.0f;		// テクスチャの左上X座標
		float ty = 0.0f;		// テクスチャの左上Y座標

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// ブロックの背景を描画
	{
		for (int j = -1; j < SCREEN_HEIGHT / TEXTURE_HEIGHT_BLOCK + 2; j++) {
			for (int i = 0; i < SCREEN_WIDTH / TEXTURE_WIDTH_BLOCK + 1; i++) {
				// テクスチャ設定
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[bg_blocktype]);

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSprite(g_VertexBuffer, TEXTURE_WIDTH_BLOCK * i, TEXTURE_HEIGHT_BLOCK*j + bgblockMoveTimer / 2, TEXTURE_WIDTH_BLOCK, TEXTURE_HEIGHT_BLOCK, 0.0f, 0.0f, 1.0f, 1.0f);

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}

}


