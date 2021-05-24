//=============================================================================
//
// チュトリアル画面処理 [tutorial.cpp]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#include "tutorial.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX					(6)				// テクスチャの数
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 

#define TEXTURE_WIDTH_BLOCK			(64)	        // ブロック背景サイズ
#define TEXTURE_HEIGHT_BLOCK		(64)	        // 
#define TEXTURE_BLOCK_SCALE		    (96)	        // 


#define TEXTURE_WIDTH_LOGO			(400)			// ロゴサイズ
#define TEXTURE_HEIGHT_LOGO			(154)			// 

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
	"data/TEXTURE/Brown.png",	
	"data/TEXTURE/Title.png",
	"data/TEXTURE/pressbutton.png",
	"data/TEXTURE/tutorial/PCtrIntroC.png",
	"data/TEXTURE/tutorial/PCtrIntroK.png",
	"data/TEXTURE/tutorial/PCtrIntroK2.png"
};


static bool						g_Use;						// true:使っている  false:未使用
static float					g_w, g_h;					// 幅と高さ
static D3DXVECTOR3				g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

// other
int tu_blockMoveTimer = 0;
int tu_scalesize = TEXTURE_HEIGHT_BLOCK;

bool tu_canInput = true;

int tu_step = 0;
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTutorial(void)
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
	g_Pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_TexNo = 0;

	//other
	tu_canInput = true;

	tu_step = 0;

	// BGM再生
	//PlaySound(SOUND_LABEL_BGM_sample000);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTutorial(void)
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
void UpdateTutorial(void)
{
	tu_blockMoveTimer += 1;
	if (tu_blockMoveTimer >= BLOCK_MOVE_TIME) {
		tu_blockMoveTimer = 0;
		
	}

	// ボタン押したら、ステージを切り替える
	if (GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(0, BUTTON_START) ||  
		IsMouseLeftTriggered() || IsButtonTriggered(0, BUTTON_X) ||
		IsMouseRightTriggered() || IsButtonTriggered(0, BUTTON_B) ||
		GetKeyboardTrigger(DIK_SPACE) || IsButtonTriggered(0, BUTTON_A) ||
		GetKeyboardTrigger(DIK_Z) || GetKeyboardTrigger(DIK_X)
		)
	{
		if (tu_canInput == true) {
			tu_step += 1;
			if (tu_step > 2) {

				tu_canInput = false;
				SetFade(FADE_OUT, MODE_GAME);
				PlaySound(SOUND_LABEL_SE_start);

			}
		}
	}


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTutorial(void)
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

	// ブロックの背景を描画
	{
		for (int j = -1; j < SCREEN_HEIGHT / TEXTURE_HEIGHT_BLOCK + 2; j++) {
			for (int i = 0; i < SCREEN_WIDTH / TEXTURE_WIDTH_BLOCK + 1; i++) {
				// テクスチャ設定
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSprite(g_VertexBuffer, TEXTURE_WIDTH_BLOCK * i, TEXTURE_HEIGHT_BLOCK*j + tu_blockMoveTimer / 2, 64, 64, 0.0f, 0.0f, 1.0f, 1.0f);

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}

	int tuNum = 3;
	if (tu_step > 1) tuNum = 5;
	else if (tu_step > 0)tuNum = 4;
	// チュトリアルを描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[tuNum]); // 3 4 5

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 70, 412, 412, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// エンターキー/スタートボタンを描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		if (tu_blockMoveTimer <= BLOCK_MOVE_TIME / 2) {
			float a = (float)tu_blockMoveTimer / (float)(BLOCK_MOVE_TIME / 2.0f);
			SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2 + 130, 320, TEXTURE_WIDTH_LOGO, 90.0f, 0.0f, 0.0f, 1.0f, 1.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, a));
		}
		else {
			float a = (float)tu_blockMoveTimer / (float)(BLOCK_MOVE_TIME / 2.0f) - 1.0f;
			SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2 + 130, 320, TEXTURE_WIDTH_LOGO, 90.0f, 0.0f, 0.0f, 1.0f, 1.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1 - a));
		}

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

}

