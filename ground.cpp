//=============================================================================
//
// プレイヤー処理 [player.cpp]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#include "ground.h"
#include "sprite.h"

#include "Collider2D.h"
#include "CollisionSystem.h"

#include "score.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(16) //(SCREEN_WIDTH)	// (200)			// キャラサイズ
#define TEXTURE_HEIGHT				(16) //(SCREEN_HEIGHT)	// (200)			// 
#define TEXTURE_MAX					(2)	 // テクスチャの数
										 
#define TEXTURE_PATTERN_DIVIDE_X	(22)	 // アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(11)	 // アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数

#define ANIM_WAIT					(7)	 // アニメーションの切り替わるWait値

#define MAX_LEVELX                  (15) // 横レベルの数
#define MAX_LEVELY                  (1)  // 縦レベルの数

#define MAX_SCREEN_GROUND_WIDTH     (40) // 単一レベル床の数 横
#define MAX_SCREEN_GROUND_HEIGHT    (23)  // 単一レベル床の数 縦

#ifdef _DEBUG	// デバッグ
#define DEBUG_STARTLEVEL     (0) 
#endif

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
char GetTile(int x, int y);
bool checkGround(int x, int y);
bool checkSpike(int x, int y);

void SetLevelData();

//*****************************************************************************
// グローバル変数  "private"
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

int GroundWidth  = MAX_SCREEN_GROUND_WIDTH; //画面=40 MAX_SCREEN_GROUND_WIDTH  +1 tranport level
int GroundHeight = MAX_SCREEN_GROUND_HEIGHT;  //画面= 23 MAX_SCREEN_GROUND_HEIGHT

string sLevelData[MAX_LEVELX][MAX_LEVELY];

int Levelx = 0; //今のレベル
int Levely = 0; 

static char *g_TexturName[] = {

"data/TEXTURE/Grass.png",
"data/TEXTURE/Terrain.png"

};

// ground sprites data 床のスプライトデータ (PatternAnim No.)
enum groundSpritesData {
	//grass floor
	grassUL =  6, grassUM =  7, grassUR =  8,
	grassML = 28, grassMM = 29, grassMR = 30,
	grassDL = 50, grassDM = 51, grassDR = 52,

	//yellow grass floor + 88
	ygrassUL = 94, ygrassUM = 95, ygrassUR = 96,
	ygrassML = 116, ygrassMM = 117, ygrassMR = 118,
	ygrassDL = 138, ygrassDM = 139, ygrassDR = 140,

	//green grass floor + 88 * 2
	ggrassUL = 182, ggrassUM = 183, ggrassUR = 184,
	ggrassML = 204, ggrassMM = 205, ggrassMR = 206,
	ggrassDL = 226, ggrassDM = 227, ggrassDR = 228,

	//stone frontier 
	woodfUL = 0, woodfUM = 1, woodfUR = 2,
	woodfML = 22, woodfMM = 23, woodfMR = 24,
	woodfDL = 44, woodfDM = 45, woodfDR = 46,

	//wood frontier + 88
	swoodfUL = 88,  swoodfUM = 89,  swoodfUR = 90,
	swoodfML = 110, swoodfMM = 111, swoodfMR = 112,
	swoodfDL = 132, swoodfDM = 133, swoodfDR = 134,

	//grass frontier + 88 * 2
	gwoodfUL = 176, gwoodfUM = 177, gwoodfUR = 178,
	gwoodfML = 198, gwoodfMM = 199, gwoodfMR = 200,
	gwoodfDL = 220, gwoodfDM = 221, gwoodfDR = 222,

	//Spike
	spikeD = 5, spikeU = 11, spikeL = 27, spikeR = 33,

};

int groundStyle = 0;
int frontierStyle = 0;
//============Reborn Spot
D3DXVECTOR3 playerRebornSpot[MAX_LEVELX]{
	{96.0f, SCREEN_HEIGHT - 176.0f, 0.0f},	  
	{TEXTURE_WIDTH * 5   + SCREEN_WIDTH * 1,  TEXTURE_WIDTH * 9.0f , 0.0f},
	{TEXTURE_WIDTH * 2   + SCREEN_WIDTH * 2,  TEXTURE_WIDTH * 4.0f , 0.0f},
	{TEXTURE_WIDTH * 2   + SCREEN_WIDTH * 3,  TEXTURE_WIDTH * 1.5f , 0.0f},
	{TEXTURE_WIDTH * 3   + SCREEN_WIDTH * 4,  TEXTURE_WIDTH * 15.0f, 0.0f},
	{TEXTURE_WIDTH * 3   + SCREEN_WIDTH * 5,  TEXTURE_WIDTH * 12.0f, 0.0f},
	{TEXTURE_WIDTH * 3   + SCREEN_WIDTH * 6,  TEXTURE_WIDTH * 2.0f , 0.0f},
	{TEXTURE_WIDTH * 4   + SCREEN_WIDTH * 7,  TEXTURE_WIDTH * 12.0f, 0.0f},
	{TEXTURE_WIDTH * 2   + SCREEN_WIDTH * 8,  TEXTURE_WIDTH * 2.0f , 0.0f},
	{TEXTURE_WIDTH * 2   + SCREEN_WIDTH * 9,  TEXTURE_WIDTH * 12.0f, 0.0f},
	{TEXTURE_WIDTH * 2   + SCREEN_WIDTH * 10, TEXTURE_WIDTH * 2.0f , 0.0f},
	{TEXTURE_WIDTH * 2   + SCREEN_WIDTH * 11, TEXTURE_WIDTH * 2.0f , 0.0f},
	{TEXTURE_WIDTH * 2   + SCREEN_WIDTH * 12, TEXTURE_WIDTH * 17.0f, 0.0f},
	{TEXTURE_WIDTH * 2   + SCREEN_WIDTH * 13, TEXTURE_WIDTH * 12.0f, 0.0f},
	{TEXTURE_WIDTH * 2   + SCREEN_WIDTH * 14, TEXTURE_WIDTH * 12.0f, 0.0f}
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGround(void)
{
	ID3D11Device *pDevice = GetDevice();

	groundStyle = rand() % 3;
	frontierStyle = rand()%3;

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

	//Init groundData			
	SetLevelData();

	//Init level
	Levelx = 0;
#ifdef _DEBUG
	Levelx = DEBUG_STARTLEVEL;
#endif
	

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGround(void)
{
	//Init level for result : levelx must be 0
	Levelx = 0;

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
void UpdateGround(void)
{


#ifdef _DEBUG	// デバッグ情報を表示する
	/*char *str = GetDebugStr();
	sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", m_Pos.x, m_Pos.y);*/
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGround(void)
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

	//bool US = true; // UpSwitch  (For Auto Change ground URDL) (too many Gettile ==> abandom)
	for (int i = 0; i < GroundWidth; i++) { //x

		for (int j = 0; j < GroundHeight; j++) { //y
			
			char Tile = GetTile(i, j);
			if (Tile !='G' && Tile !='.') {
				// テクスチャ設定
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

				float px = TEXTURE_WIDTH / 2 + i * TEXTURE_WIDTH;	// プレイヤーの表示位置X
				float py = TEXTURE_HEIGHT / 2 + j * TEXTURE_HEIGHT;	// プレイヤーの表示位置Y

				float pw = TEXTURE_WIDTH;		// プレイヤーの表示幅
				float ph = TEXTURE_HEIGHT;		// プレイヤーの表示高さ

				float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
				float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ

				float tx = 0.0f; // テクスチャの左上X座標
				float ty = 0.0f; // テクスチャの左上Y座標
				
				//================================== set Ground data
				int groundPattern = 0;
				switch (Tile)
				{
				case'Q':
					groundPattern = grassUL + 88 * groundStyle; break;
				case'W':
					groundPattern = grassUM + 88 * groundStyle; break;
				case'E':				   
					groundPattern = grassUR + 88 * groundStyle; break;
				case'A':				   
					groundPattern = grassML + 88 * groundStyle; break;
				case'S':				   
					groundPattern = grassMM + 88 * groundStyle; break;
				case'D':				   
					groundPattern = grassMR + 88 * groundStyle; break;
				case'Z':				   
					groundPattern = grassDL + 88 * groundStyle; break;
				case'X':				   
					groundPattern = grassDM + 88 * groundStyle; break;
				case'C':				  
					groundPattern = grassDR + 88 * groundStyle; break;
				case'q':
					groundPattern = woodfUL + 88 * frontierStyle; break;
				case'w':				   
					groundPattern = woodfUM + 88 * frontierStyle; break;
				case'e':				   
					groundPattern = woodfUR + 88 * frontierStyle; break;
				case'a':				    
					groundPattern = woodfML + 88 * frontierStyle; break;
				case's':				   
					groundPattern = woodfMM + 88 * frontierStyle; break;
				case'd':				   
					groundPattern = woodfMR + 88 * frontierStyle; break;
				case'z':				   
					groundPattern = woodfDL + 88 * frontierStyle; break;
				case'x':				   
					groundPattern = woodfDM + 88 * frontierStyle; break;
				case'c':				   
					groundPattern = woodfDR + 88 * frontierStyle; break;
				case'1':
					groundPattern = spikeD; break;
				case'!':
					groundPattern = spikeU; break;
				case'2':
					groundPattern = spikeR; break;
				case'@':
					groundPattern = spikeL; break;

				default:
					groundPattern = grassMM; break;
				}
				tx = ((float)(groundPattern % TEXTURE_PATTERN_DIVIDE_X)) * tw;
				ty = ((float)(groundPattern / TEXTURE_PATTERN_DIVIDE_X)) * th;
				 //==================================
				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSprite(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th); // draw call
				//SetSprite(g_VertexBuffer, TEXTURE_WIDTH/2 + i * TEXTURE_WIDTH, TEXTURE_HEIGHT/2 + j * TEXTURE_HEIGHT, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
			
		}
	}

}


char GetTile(int x, int y)
{
	if (x >= 0 && x < GroundWidth && y >= 0 && y < GroundHeight)// 出界
		return sLevelData[Levelx][Levely][y * GroundWidth + x];
	else
		return' ';
}


bool IsGround(D3DXVECTOR3 vec) {
	return IsGround(vec.x, vec.y);
}


bool IsGround(float posx, float posy)
{
	int x = (int)(posx / TEXTURE_WIDTH) - Levelx * MAX_SCREEN_GROUND_WIDTH;
	int y = (int)(posy / TEXTURE_HEIGHT) - Levely * MAX_SCREEN_GROUND_HEIGHT;
	if (x >= 0 && x < GroundWidth && y >= 0 && y < GroundHeight) { // 出界
		if (checkGround(x, y))
			return true;
		else
			return false;
	}
	return false;
}

bool IsSpike(float posx, float posy)
{
	int x = (int)(posx / TEXTURE_WIDTH) - Levelx * MAX_SCREEN_GROUND_WIDTH;
	int y = (int)(posy / TEXTURE_HEIGHT) - Levely * MAX_SCREEN_GROUND_HEIGHT;
	if (x >= 0 && x < GroundWidth && y >= 0 && y < GroundHeight) { // 出界
		if (checkSpike(x, y))
			return true;
		else
			return false;
	}
	return false;
}

bool IsTransportLevel(D3DXVECTOR3 vec) {
	return IsTransportLevel(vec.x, vec.y);
}


bool IsTransportLevel(float posx, float posy)
{
	
	//==== Transport by walk over screen (right , down)
	int x = (int)(posx / TEXTURE_WIDTH);
	int y = (int)(posy / TEXTURE_HEIGHT);
	if (x >= (Levelx + 1) * MAX_SCREEN_GROUND_WIDTH) {
		groundStyle = rand() % 3;
		frontierStyle = rand() % 3;
		AddScore(1);
		Levelx += 1;
		return true;
	}
	/*if (y >= (Levely + 1) * MAX_SCREEN_GROUND_HEIGHT) {
		Levely += 1;
		return true;
	}*/

	//==== Transport by touching 'T'
	x -= Levelx * MAX_SCREEN_GROUND_WIDTH;
	y -= Levely * MAX_SCREEN_GROUND_HEIGHT;

	if (x >= 0 && x < GroundWidth && y >= 0 && y < GroundHeight) { // 出界
		if (sLevelData[Levelx][Levely][y * GroundWidth + x] =='T') {
			groundStyle = rand() % 3;
			frontierStyle = rand() % 3;
			AddScore(1);
			Levelx += 1;
			return true;
		}
		else
			return false;
	}

	//==== Transport by going back
	if (x < 0) {
		groundStyle = rand() % 3;
		frontierStyle = rand() % 3;
		AddScore(-1);
		Levelx -= 1;
	}
	/*if (y < 0) {
		Levely -= 1;
	}*/

	return false;
}

void TransportLevel() {
	groundStyle = rand() % 3;
	frontierStyle = rand() % 3;
	AddScore(1);
	Levelx += 1;
}

int GetLevelX()
{
	return Levelx;
}

int GetLevelY()
{
	return Levely;
}

void SetLevelX(int level)
{
	Levelx = level;
}

D3DXVECTOR2  GetGroundCenterPos(float posx, float posy) {
	int x = (int)(posx / TEXTURE_WIDTH) - Levelx * MAX_SCREEN_GROUND_WIDTH;
	int y = (int)(posy / TEXTURE_HEIGHT) - Levely * MAX_SCREEN_GROUND_HEIGHT;
	if (x >= 0 && x < GroundWidth && y >= 0 && y < GroundHeight) { // 出界
		if (checkGround(x,y))
			return D3DXVECTOR2(x * TEXTURE_WIDTH + TEXTURE_WIDTH * 0.5f + Levelx * MAX_SCREEN_GROUND_WIDTH * TEXTURE_WIDTH, y * TEXTURE_HEIGHT + TEXTURE_HEIGHT * 0.5f + Levely * MAX_SCREEN_GROUND_HEIGHT * TEXTURE_HEIGHT);
		else
			return D3DXVECTOR2(0.0f, 0.0f);
	}
	return D3DXVECTOR2(0.0f, 0.0f);
}

float GetGroundTextureWidth()
{
	return TEXTURE_WIDTH;
}

D3DXVECTOR3 GetPlayerRebornSpot(int levelx)
{
	return playerRebornSpot[levelx];
}

bool checkGround(int x, int y) { // 衝突できる床
	if (sLevelData[Levelx][Levely][y * GroundWidth + x] =='Q' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='W' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='E' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='A' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='S' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='D' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='Z' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='X' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='C' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='q' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='w' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='e' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='a' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='s' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='d' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='z' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='x' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] =='c' 
		)
		return true;
		
	else return false;
}

bool checkSpike(int x, int y) {
	if (sLevelData[Levelx][Levely][y * GroundWidth + x] == '!' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] == '1' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] == '2' ||
		sLevelData[Levelx][Levely][y * GroundWidth + x] == '@' 
		)
		return true;

	else return false;
}

void SetLevelData()
{
	sLevelData[0][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[0][0] += "a......................................d";
	sLevelData[0][0] += "a......................................d";
	sLevelData[0][0] += "a......................................d";
	sLevelData[0][0] += "a......................................d";
	sLevelData[0][0] += "a......................................d";
	sLevelData[0][0] += "a......................................d";
	sLevelData[0][0] += "a......................................d";
	sLevelData[0][0] += "a......................................d";
	sLevelData[0][0] += "a.......................................";
	sLevelData[0][0] += "a.......................................";
	sLevelData[0][0] += "a.......................................";
	sLevelData[0][0] += "a.......................................";
	sLevelData[0][0] += "a.........................QWWWWWWWWWWWEd";
	sLevelData[0][0] += "a.........................ASSSSSSSSSSSDd";
	sLevelData[0][0] += "a.............QWWWWWWWWWWWSSSSSSSSSSSSDd";
	sLevelData[0][0] += "a.............ASSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[0][0] += "aQWWWWWWWWWWWWSSSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[0][0] += "aASSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[0][0] += "aASSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[0][0] += "aASSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[0][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[0][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";
	

	sLevelData[1][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[1][0] += "a......................................d";
	sLevelData[1][0] += "a......................................d";
	sLevelData[1][0] += "a.......................................";
	sLevelData[1][0] += "a.......................................";
	sLevelData[1][0] += "a.......................................";
	sLevelData[1][0] += "a.......................................";
	sLevelData[1][0] += "a.................................WWWWEd";
	sLevelData[1][0] += "a.................................ASSSDd";
	sLevelData[1][0] += "..................................ASSSDd";
	sLevelData[1][0] += "..................................ASSSDd";
	sLevelData[1][0] += "..................................ASSSDd";
	sLevelData[1][0] += "..................................ASSSDd";
	sLevelData[1][0] += "aQWWWWWWE.................QWWWWWWWSSSSDd";
	sLevelData[1][0] += "aASSSSSSD.................ASSSSSSSSSSSDd";
	sLevelData[1][0] += "aASSSSSSD.................ASSSSSSSSSSSDd";
	sLevelData[1][0] += "aASSSSSSD.................ASSSSSSXSSSSDd";
	sLevelData[1][0] += "aASSSSSSD.................ASSSSSD.ASSSDd";
	sLevelData[1][0] += "aASSSSSSD.................ASSSSSSWSSSSDd";
	sLevelData[1][0] += "aASSSSSSD.................ASSSSSSSSSSSDd";
	sLevelData[1][0] += "aASSSSSSSWWWWWWWWWWWWWWWWWSSSSSSSSSSSSDd";
	sLevelData[1][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[1][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";
	

	sLevelData[2][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[2][0] += "a.......................................";
	sLevelData[2][0] += "a.......................................";
	sLevelData[2][0] += "........................................";
	sLevelData[2][0] += "...............................QWWWWWWEd";
	sLevelData[2][0] += "...............................ASSSSSSDd";
	sLevelData[2][0] += "...............................ASSSSSSDd";
	sLevelData[2][0] += "aQWWE..........................ASSSSSSDd";
	sLevelData[2][0] += "aASSD..........................ASSSSSSDd";
	sLevelData[2][0] += "aASSD..........................ASSSSSSDd";
	sLevelData[2][0] += "aASSD....................QE....ASSSSSSDd";
	sLevelData[2][0] += "aASSD....................ZC....ASSSSSSDd";
	sLevelData[2][0] += "aASSD...QE.....QWWWWE..........ASSSSSSDd";
	sLevelData[2][0] += "aASSD...ZC.....ASSSSD..........ASSSSSSDd";
	sLevelData[2][0] += "aASSD..........ASSSSD..........ASSSSSSDd";
	sLevelData[2][0] += "aASSD..........ASSSSD..........ASSSSSSDd";
	sLevelData[2][0] += "aASSD..........ASSSSD..........ASSSSSSDd";
	sLevelData[2][0] += "aASSD..........ASSSSD..........ASSSSSSDd";
	sLevelData[2][0] += "aASSD..........ASSSSD..........ASSSSSSDd";
	sLevelData[2][0] += "aASSD..........ASSSSD..........ASSSSSSDd";
	sLevelData[2][0] += "aASSD1111111111ASSSSD1111111111ASSSSSSDd";
	sLevelData[2][0] += "aZXXXWWWWWWWWWWXXXXXXWWWWWWWWWWWWWWWWWWd";
	sLevelData[2][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";


	sLevelData[3][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[3][0] += ".........!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!d";
	sLevelData[3][0] += ".......................................d";
	sLevelData[3][0] += ".......................................d";
	sLevelData[3][0] += "aQWWWWWWE..............................d";
	sLevelData[3][0] += "aASSSSSSD..............................d";
	sLevelData[3][0] += "aASSSSSSD..............................d";
	sLevelData[3][0] += "aASSSSSSD..............................d";
	sLevelData[3][0] += "aASSSSSSD..............................d";
	sLevelData[3][0] += "aASSSSSSD..............................d";
	sLevelData[3][0] += "aASSSSSSD..............................d";
	sLevelData[3][0] += "aASSSSSSD..............................d";
	sLevelData[3][0] += "aASSSSSSD.......QWWWWE.................d";
	sLevelData[3][0] += "aASSSSSSD@.....2ASSSSD@.................";
	sLevelData[3][0] += "aASSSSSSD@.....2ASSSSD@.................";
	sLevelData[3][0] += "aASSSSSSD@.....2ASSSSD@.................";
	sLevelData[3][0] += "aASSSSSSD@.....2ASSSSD@.................";
	sLevelData[3][0] += "aASSSSSSD@.....2ASSSSD@.................";
	sLevelData[3][0] += "aASSSSSSD@.....2ASSSSD@.................";
	sLevelData[3][0] += "aASSSSSSD@.....2ASSSSD@1111111..........";
	sLevelData[3][0] += "aASSSSSSD1111111ASSSSSWWWWWWWWWWWWWWWWEd";
	sLevelData[3][0] += "aZXXXXXXXWWWWWWWXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[3][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";

	sLevelData[4][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[4][0] += "a.......................................";
	sLevelData[4][0] += "a.......................................";
	sLevelData[4][0] += "a.......................................";
	sLevelData[4][0] += "a.......................................";
	sLevelData[4][0] += "a................QWWWWWWWWWWWWWWWWWWWWEd";
	sLevelData[4][0] += "a................ASSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[4][0] += "a................ASSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[4][0] += "a................ASSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[4][0] += "a................ZXXXXXXXXXXXXXXXXXXXXXd";
	sLevelData[4][0] += "a................!!!!!!!!!!!!!!!!!!!!!!!";
	sLevelData[4][0] += "a.......................................";
	sLevelData[4][0] += "a.......................................";
	sLevelData[4][0] += "........................................";
	sLevelData[4][0] += "...........QWWWWWWWWWWWWWWWWWWWWWWWWWWWd";
	sLevelData[4][0] += "...........ASSSSSSSSSSSSSSSSSSSSSSSSSSSd";
	sLevelData[4][0] += ".......QWWWSSSSSSSSSSSSSSSSSSSSSSSSSSSSd";
	sLevelData[4][0] += ".......AXXXSSSSSSSSSSSSSSSSSSSSSSSSSSSSd";
	sLevelData[4][0] += ".......A...ASSSSSSSSSSSSSSSSSSSSSSSSSSSd";
	sLevelData[4][0] += ".......A...ASSSSSSSSSSSSSSSSSSSSSSSSSSSd";
	sLevelData[4][0] += "aWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWd";
	sLevelData[4][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[4][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";

	sLevelData[5][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[5][0] += "........................................";
	sLevelData[5][0] += "........................................";
	sLevelData[5][0] += "........................................";
	sLevelData[5][0] += "........................................";
	sLevelData[5][0] += "aQWWWWWWWWWWWWWWWWWWWWWWWWE....QWWWWWWEd";
	sLevelData[5][0] += "aASSSSSSSSSSXSSSSSSSSSSSSSD....ASSSSSSDd";
	sLevelData[5][0] += "aASSSSSSSSSD.ASSSSSSSSSSSSD....ASSSSSSDd";
	sLevelData[5][0] += "aASSSSSSSSSSWSSSSSSSSSSSSSD....ASSSSSSDd";
	sLevelData[5][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXC....ZXXXXXXXd";
	sLevelData[5][0] += "!!!!!!!!!!!!!!!................!!!!!!!!d";
	sLevelData[5][0] += ".......................................d";
	sLevelData[5][0] += ".......................................d";
	sLevelData[5][0] += ".......................................d";
	sLevelData[5][0] += "aQWWWWE................................d";
	sLevelData[5][0] += "aASSSSD................................d";
	sLevelData[5][0] += "aASSSSSWWWE............................d";
	sLevelData[5][0] += "aASSSSSSSSD.....QWWWWWWWWWWWWWWWWWWWWWWd";
	sLevelData[5][0] += "aASSSSSSSSD.....ASSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[5][0] += "aASSSSSSSSD.....ASSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[5][0] += "aWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWd";
	sLevelData[5][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[5][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";


	sLevelData[6][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[6][0] += ".......................................d";
	sLevelData[6][0] += ".......................................d";
	sLevelData[6][0] += ".......................................d";
	sLevelData[6][0] += ".......................................d";
	sLevelData[6][0] += "aQWWE..................................d";
	sLevelData[6][0] += "aASSD..................................d";
	sLevelData[6][0] += "aASSD..................................d";
	sLevelData[6][0] += "aASSD..................................d";
	sLevelData[6][0] += "aZXXC..................................d";
	sLevelData[6][0] += "a........QWWWE.........................d";
	sLevelData[6][0] += "a........AXXXD.........................d";
	sLevelData[6][0] += "a........A...D.........................d";
	sLevelData[6][0] += "a........A...D..........................";
	sLevelData[6][0] += "aQWWWWWWWWWWWD.....QE....QE.....QE......";
	sLevelData[6][0] += "aASSSSSSSSSSSD.....ZC....ZC.....ZC......";
	sLevelData[6][0] += "aASSSSSSSSXXXC.......................QEd";
	sLevelData[6][0] += "aASSSSSSSD...........................ZCd";
	sLevelData[6][0] += "aASSSSSXXC.............................d";
	sLevelData[6][0] += "aASSSSD................................d";
	sLevelData[6][0] += "aASSSSD11111111111111111111111111111111d";
	sLevelData[6][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[6][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";

	sLevelData[7][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[7][0] += "a..................ASSSD...............d";
	sLevelData[7][0] += "a..................ASSSD...............d";
	sLevelData[7][0] += "a..................ASSSD................";
	sLevelData[7][0] += "a......QWWWE.......ASSSD................";
	sLevelData[7][0] += "a......ASSSD.......ASSSD................";
	sLevelData[7][0] += "a....QWSSSSD.......ASSSD.........QWWWWWd";
	sLevelData[7][0] += "a....ZXSSSSD.......ZXXXC.........ZXXXXXd";
	sLevelData[7][0] += "a......ASSSD.......!!!!!...............d";
	sLevelData[7][0] += "a......ASSSD...........................d";
	sLevelData[7][0] += "aWE....ASSSD...........................d";
	sLevelData[7][0] += "aXC....ASSSD................QE.........d";
	sLevelData[7][0] += "a......ASSSD................ZC.........d";
	sLevelData[7][0] += ".....QWSXXXD...........................d";
	sLevelData[7][0] += ".....ASD...D...........................d";
	sLevelData[7][0] += ".....ASD...D...........................d";
	sLevelData[7][0] += "aQWWWWWWWWWWWWWWWE.....................d";
	sLevelData[7][0] += "aASSSSSSSSSSSSSSSD.....................d";
	sLevelData[7][0] += "aASSSSSSXSSSSSSSSD.....................d";
	sLevelData[7][0] += "aASSSSSD.ASSSSSSSD.....................d";
	sLevelData[7][0] += "aASSSSSSWSSSSSSSSD111111111111111111111d";
	sLevelData[7][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[7][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";

	sLevelData[8][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[8][0] += "a.............ASSSD....................d";
	sLevelData[8][0] += "a.............ASSSD....................d";
	sLevelData[8][0] += "..............AXXXD....................d";
	sLevelData[8][0] += "..............A...D....................d";
	sLevelData[8][0] += "..............A...D....................d";
	sLevelData[8][0] += "aQWWWE........AWWWD........QWE.........d";
	sLevelData[8][0] += "aASSSD........ZXXXC........ZXC.........d";
	sLevelData[8][0] += "aASSSD........!!!!!........!!!QWWE.....d";
	sLevelData[8][0] += "aASSSD.......................2ASSD.....d";
	sLevelData[8][0] += "aASSSSWE.....................2ASSD.....d";
	sLevelData[8][0] += "aASSSSSD.....................2ASSD......";
	sLevelData[8][0] += "aAXXXSXC.....................2ASSD......";
	sLevelData[8][0] += "aA...D.......................2ASSD......";
	sLevelData[8][0] += "aA...D.......................2ASSD......";
	sLevelData[8][0] += "aQWWWWWWWWWE.................2ASSSWWWWEd";
	sLevelData[8][0] += "aASSSSSSSSSD.................2ASSSSSSSDd";
	sLevelData[8][0] += "aASSSSSSSSSD.................2ASSSSSSSDd";
	sLevelData[8][0] += "aASSSSSSSSSD.................2ASSSSSSSDd";
	sLevelData[8][0] += "aASSSSSSSSSD.................2ASSSSSSSDd";
	sLevelData[8][0] += "aASSSSSSSSSD111111111111111112SSSSSSSSDd";
	sLevelData[8][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[8][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";

	sLevelData[9][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[9][0] += "aASSSSSSD...........ASSSSSSSSSSSSSSSSSDd";
	sLevelData[9][0] += "aASSSSSSD...........ASSSSSSSSSSSSXXXXXCd";
	sLevelData[9][0] += "aASSSSSSD...........ASSSSSXXXSSSD.......";
	sLevelData[9][0] += "aASSSXSSD...........ASSSSD...ASSD.......";
	sLevelData[9][0] += "aASSD.ASD...........ASSSSD...ASSD.......";
	sLevelData[9][0] += "aASSSWSSD...........ASSSSSWWWSSSD...QWEd";
	sLevelData[9][0] += "aASSSSSSD...........ASSSSSSSSSSSD...ASDd";
	sLevelData[9][0] += "aASSSSSSD...........ASSSSSSSSSSSD...ASDd";
	sLevelData[9][0] += "aASSSSSSD...........ASSSSSSSSSSSD...ASDd";
	sLevelData[9][0] += "aZXXXXXXC...........ZXXXXXXXXXXXC...ZXCd";
	sLevelData[9][0] += ".......................................d";
	sLevelData[9][0] += ".......................................d";
	sLevelData[9][0] += ".......................................d";
	sLevelData[9][0] += ".......................................d";
	sLevelData[9][0] += "aQWWWE.................................d";
	sLevelData[9][0] += "aASSSD.................................d";
	sLevelData[9][0] += "aASSSSWWWWWWWWWE.......................d";
	sLevelData[9][0] += "aASD...ASSSSSSSD.......................d";
	sLevelData[9][0] += "aASD...ASSSSSSSD.......................d";
	sLevelData[9][0] += "aQWWWWWWWWWWWWWWWWWWWWWWWWWWE1111111111d";
	sLevelData[9][0] += "axxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxd";
	sLevelData[9][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxz";

	sLevelData[10][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[10][0] += "a.........................ASD...........";
	sLevelData[10][0] += "a.........................ASD...........";
	sLevelData[10][0] += "..........................ASD...........";
	sLevelData[10][0] += "..........................ASD...........";
	sLevelData[10][0] += "..........................ASD......QWWEd";
	sLevelData[10][0] += "aQWE......................ASD......ZXXCd";
	sLevelData[10][0] += "aZXC......................ASD..........d";
	sLevelData[10][0] += "a.........................ASD..........d";
	sLevelData[10][0] += "a............QWE..........ASD..........d";
	sLevelData[10][0] += "a............ZXC..........ASD..........d";
	sLevelData[10][0] += "a.........................ASD..........d";
	sLevelData[10][0] += "a.........................ASD..........d";
	sLevelData[10][0] += "a.........................ZXC..........d";
	sLevelData[10][0] += "a......................................d";
	sLevelData[10][0] += "aQWE....QWE.......QWE..................d";
	sLevelData[10][0] += "aZXC....ZXC.......ZXC..................d";
	sLevelData[10][0] += "a......................................d";
	sLevelData[10][0] += "a......................................d";
	sLevelData[10][0] += "a......................................d";
	sLevelData[10][0] += "a11111111111111111111111111111111111111d";
	sLevelData[10][0] += "axxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxd";
	sLevelData[10][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxz";

	sLevelData[11][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[11][0] += ".......................................d";
	sLevelData[11][0] += ".......................................d";
	sLevelData[11][0] += ".......................................d";
	sLevelData[11][0] += ".......................................d";
	sLevelData[11][0] += "aQWWWWWWWWWWWWWWWWE...QWWWWWWWWWWWWWWWEd";
	sLevelData[11][0] += "aZXXXXXXXXXXXXXXXXC...ZXXXXXXXXXXXXXXXCd";
	sLevelData[11][0] += "a......................................d";
	sLevelData[11][0] += "a......................................d";
	sLevelData[11][0] += "a......................................d";
	sLevelData[11][0] += "aQWWWWWWWWWWWWWWWWWWWWWWWWWWWWWE...QWWEd";
	sLevelData[11][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXC...ZXXCd";
	sLevelData[11][0] += "a......................................d";
	sLevelData[11][0] += "a......................................d";
	sLevelData[11][0] += "a......................................d";
	sLevelData[11][0] += "aQWWE...QWWWWWWWWWWWWWWWWWWWWWWWWWWWWWEd";
	sLevelData[11][0] += "aZXXC...ZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[11][0] += "a.......................................";
	sLevelData[11][0] += "a.......................................";
	sLevelData[11][0] += "a.......................................";
	sLevelData[11][0] += "aQWWWWWWWWE...QWWWWWWWWWWWE...QWWWWWWWEd";
	sLevelData[11][0] += "aZXXXXXXXXC...ZXXXXXXXXXXXC...ZXXXXXXXCd";
	sLevelData[11][0] += "zxxxxxxxxxxWWWxxxxxxxxxxxxxWWWxxxxxxxxxz";

	sLevelData[12][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "aQWE...................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "a......................................d";
	sLevelData[12][0] += "aQWE...................................d";
	sLevelData[12][0] += "a.......................................";
	sLevelData[12][0] += "........................................";
	sLevelData[12][0] += "........................................";
	sLevelData[12][0] += "........................................";
	sLevelData[12][0] += "aQWWWWWWE......................QWWWWWWEd";
	sLevelData[12][0] += "aZXXXXXXC1111111111111111111111ZXXXXXXCd";
	sLevelData[12][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxz";

	sLevelData[13][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[13][0] += "a......................ASSSSSSSSSSSSSSDd";
	sLevelData[13][0] += "a......................ASSSSSSSSSSSSSSDd";
	sLevelData[13][0] += "a......................ASSSSSSSSSSSSSSDd";
	sLevelData[13][0] += "a......................AXXXSSSSSSSSSSSDd";
	sLevelData[13][0] += "a...............QWWWWWWD...ASSSSSSSSSSDd";
	sLevelData[13][0] += "a...............ASSSSSSD...ASSSSSSSSSSDd";
	sLevelData[13][0] += "a...............ASSSSSSSWWWSSSSSSSSSSSDd";
	sLevelData[13][0] += "a...............ZXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[13][0] += "a.......................................";
	sLevelData[13][0] += "a.......................................";
	sLevelData[13][0] += "a.......................................";
	sLevelData[13][0] += "a..........QWWWWWWWWWWWWWWWWWWWWWWWWWWEd";
	sLevelData[13][0] += "a..........ASSSSSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[13][0] += "a..........ASSSSSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[13][0] += "a.......QWWSSSSSSSSSSSSSSSSSSD...ASSSSDd";
	sLevelData[13][0] += "........ZXXXXXXXXSSSSSSSSSSSSD...ASSSSDd";
	sLevelData[13][0] += ".................ASSSSSSSSSSSSWWWSSSSSDd";
	sLevelData[13][0] += ".................ASSSSSSSSSSSD...AD...Dd";
	sLevelData[13][0] += ".................ASSSSSSSSSSSD...AD...Dd";
	sLevelData[13][0] += "aWWWWWWWWWWWWWWWWSSSSSSSSSSSSSWWWSSWWWDd";
	sLevelData[13][0] += "aZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[13][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxz";


	sLevelData[14][0] += "qwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwe";
	sLevelData[14][0] += "a......................................d";
	sLevelData[14][0] += "a......................................d";
	sLevelData[14][0] += "a......................................d";
	sLevelData[14][0] += "a......................................d";
	sLevelData[14][0] += "a......................................d";
	sLevelData[14][0] += "a......................................d";
	sLevelData[14][0] += "a......................................d";
	sLevelData[14][0] += "a......................................d";
	sLevelData[14][0] += ".......................................d";
	sLevelData[14][0] += ".......................................d";
	sLevelData[14][0] += ".......................................d";
	sLevelData[14][0] += "aQWWWWWWWWWWWE.........................d";
	sLevelData[14][0] += "aASSSSSSSSSSSD.........................d";
	sLevelData[14][0] += "aSSSSSSSSSSSSSWWWWWWWWWWWE.............d";
	sLevelData[14][0] += "aSSSSSSSSSSSSSSSSSSSSSSSSD.............d";
	sLevelData[14][0] += "aSSSSSSSSSSSSSSSSSSSSSSSSD.............d";
	sLevelData[14][0] += "aSSSSSSSSSSSSSSSSSSSSSSSSSWWWWWWWWWWWWEd";
	sLevelData[14][0] += "aSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[14][0] += "aSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[14][0] += "aSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSDd";
	sLevelData[14][0] += "aXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXCd";
	sLevelData[14][0] += "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxc";


}
