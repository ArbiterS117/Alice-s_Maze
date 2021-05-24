//=============================================================================
//
// プレイヤー処理 [enemy.cpp]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#include "game.h"
#include "enemy.h"
#include "sprite.h"
#include "ground.h"
#include "bullet.h"
#include "sound.h"

#include "Collider2D.h"
#include "CollisionSystem.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX					(EnemyTypesNum)		// テクスチャの数

#define ANIM_WAIT					(3)		// アニメーションの切り替わるWait値

#define ENEMY_SPEED                 (2.0f)
#define ANIM_ONEACTION_MAX_SPRITES  (30)

#define ENEMY_SETUNUSEDTIME         (100)

#define SLIME_REBORN_TIME           (30)
#define MONSTER_REBORN_TIME         (360)
#define TURTLE_REBORN_TIME          (1200)

#define HIT_SE_TIME                 (30)
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void ChangeStatus(enemyStruct *enemy, int status);

void InputUpdate(enemyStruct &enemy);
void PhysicsUpdate(enemyStruct &enemy);
void CollisionUpdate(enemyStruct &enemy);
void enemyStatusUpdate(enemyStruct &enemy);
void AnimUpdate(enemyStruct &enemy);

void TrunkStatusUpdate(enemyStruct &enemy);
void TrunkAnimUpdate(enemyStruct &enemy);
void BunnyStatusUpdate(enemyStruct &enemy);
void BunnyAnimUpdate(enemyStruct &enemy);
void SlimeStatusUpdate(enemyStruct &enemy);
void SlimeAnimUpdate(enemyStruct &enemy);
void TurtleStatusUpdate(enemyStruct &enemy);
void TurtleAnimUpdate(enemyStruct &enemy);
void BirdStatusUpdate(enemyStruct &enemy);
void BirdAnimUpdate(enemyStruct &enemy);
void FlagStatusUpdate(enemyStruct &enemy);
void FlagAnimUpdate(enemyStruct &enemy);

void linearMove();
//*****************************************************************************
// グローバル変数  "private"
//*****************************************************************************
int HitSETimer = 0; // ヒットSE

static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[] = {

"data/TEXTURE/Trunk (64x32).png",
"data/TEXTURE/Bunny (34x44).png",
"data/TEXTURE/SLIME(44x30).png",
"data/TEXTURE/turtle(44x26).png",
"data/TEXTURE/blueBird(32x32).png",
"data/TEXTURE/Flag(64x64).png"

};

struct enemySpritesHWDivDataStruct {
	int width;
	int height;
	int TEXTURE_PATTERN_DIVIDE_X;
	int TEXTURE_PATTERN_DIVIDE_Y;
};

//g_TexturNameの順番
enemySpritesHWDivDataStruct enemySpritesHWDivData[EnemyTypesNum] = {
	{64, 32, 30, 2},             // Trunk
	{34, 44, 27 ,1},			 // Bunny
	{44, 30, 15, 1},             // Slime
	{44, 26, 19, 1},             // Turtle
	{32, 32, 14, 1},             // Bird
	{64, 64, 26, 1}              // Flag
};

enemyStruct enemys[Enemy_MAX];

//=========Status

enum EnemyStatus
{
	A_Idle,
	A_InAir,
	A_Move,
	A_Hit,
	A_Attack,
	A_DEAD,
	ANIM_ACTION_NUM   // アニメーション種類の数

};

//=========Anim
struct EnemyAnimSpritesData {
	int Data[ANIM_ONEACTION_MAX_SPRITES];
};

enum EnemyAnimPlayType {
	APT_LOOP = -1,	 // loop
	APT_STOPF = -2,	 // stop in final
	APT_CWV = -3,	 // Change with velocity
	APT_COS = -4   // Change to Other States
};

// Same order as Enemy Status
// -1 : loop   -2: stop in final   -3: Change with velocity  -4: Change to Other States ......
EnemyAnimSpritesData TrunkSpritesData[ANIM_ACTION_NUM] = {
	{30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, APT_LOOP},     //A_Idle
	{11, 12, 12, 12,APT_CWV},									                            //A_InAir
	{16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, APT_LOOP},                     //A_Move
	{0, 1, 2, 3, 4, APT_COS},                                                               //A_Hit
	{5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, APT_COS},								        //A_Attack
	{0, 1, 2, 3, 4, APT_STOPF},                                                             //A_DEAD

};

EnemyAnimSpritesData BunnySpritesData[ANIM_ACTION_NUM] = {
	{6, 7, 8, 9, 10, 11, 12, 13, APT_LOOP},                                                 //A_Idle
	{0, 14, 14, 14,APT_CWV},									                            //A_InAir
	{15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, APT_LOOP},                             //A_Move
	{1, 2, 3, 4, 5, APT_COS},                                                               //A_Hit
	{0, 0, 0, 0, 0, 0, 0, 0, APT_COS},								                        //A_Attack
	{1, 2, 3, 4, 5, APT_STOPF},                                                             //A_DEAD

};

EnemyAnimSpritesData SlimeSpritesData[ANIM_ACTION_NUM] = {
	{5, 6, 7, 8, 9, 10, 11, 12, 13, 14, APT_LOOP},                                          //A_Idle
	{11, 12, 13, 14,APT_CWV},									                            //A_InAir
	{5, 6, 7, 8, 9, 10, 11, 12, 13, 14, APT_LOOP},                                          //A_Move
	{0, 1, 2, 3, 4, APT_COS},                                                               //A_Hit
	{0, 0, 0, 0, 0, 0, 0, 0, APT_COS},								                        //A_Attack
	{0, 1, 2, 3, 4, APT_STOPF},                                                             //A_DEAD

};

EnemyAnimSpritesData TurtleSpritesData[ANIM_ACTION_NUM] = {
	{5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, APT_LOOP},                          //A_Idle
	{11, 12, 13, 14,APT_CWV},									                            //A_InAir
	{5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, APT_LOOP},                          //A_Move
	{0, 1, 2, 3, 4, APT_COS},                                                               //A_Hit
	{0, 0, 0, 0, 0, 0, 0, 0, APT_COS},								                        //A_Attack
	{0, 1, 2, 3, 4, APT_STOPF},                                                             //A_DEAD

};

EnemyAnimSpritesData BirdSpritesData[ANIM_ACTION_NUM] = {
	{0, 1, 2, 3, 4, 5, 6, 7, 8, APT_LOOP},                                                  //A_Idle
	{0, 1, 2, 3, 4, 5, 6, 7, 8, APT_LOOP}, 						                            //A_InAir
	{0, 1, 2, 3, 4, 5, 6, 7, 8, APT_LOOP},                                                  //A_Move
	{9, 10, 11, 12, 13, APT_COS},                                                           //A_Hit
	{0, 0, 0, 0, 0, 0, 0, 0, APT_COS},								                        //A_Attack
	{9, 10, 11, 12, 13, APT_STOPF},                                                         //A_DEAD

};

EnemyAnimSpritesData FlagSpritesData[ANIM_ACTION_NUM] = {
	{7, 7, 7, 7, APT_LOOP},                                                                 //A_Idle
	{7, 7, 7, 7, APT_LOOP}, 						                                        //A_InAir
	{7, 7, 7, 7, APT_LOOP},                                                                 //A_Move
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25,APT_STOPF},                                      //A_Hit
	{0, 0, 0, 0, 0, 0, 0, 0, APT_COS},								                        //A_Attack
	{25, 24, 23, 22, APT_LOOP},                                                             //A_DEAD

};

//collider
Collider2dStruct enemysCollider2d;

//線形補間のプログラミング
//エネミーの線形移動用の移動座標テーブル
static D3DXVECTOR3 g_MoveTbl[] = { //    example                                                    ↓調整
	D3DXVECTOR3(GetGroundTextureWidth() * 16,SCREEN_HEIGHT - GetGroundTextureWidth() * 12 - enemySpritesHWDivData[0].height / 4.0f,0.0f),
	D3DXVECTOR3(GetGroundTextureWidth() * 28,SCREEN_HEIGHT - GetGroundTextureWidth() * 12 - enemySpritesHWDivData[0].height / 4.0f,0.0f),
	
};
static D3DXVECTOR3 g_MoveTbl2[2]; // level2 --> enemy[1]
static D3DXVECTOR3 g_MoveTbl3[2]; // level4 --> enemy[3]
static D3DXVECTOR3 g_MoveTbl4[2]; // level5 --> enemy[5]
static D3DXVECTOR3 g_MoveTbl5[2]; // level6 --> enemy[6]
static D3DXVECTOR3 g_MoveTbl6[2]; // level6 --> enemy[7]
static D3DXVECTOR3 g_MoveTbl7[2]; // level6 --> enemy[8]
static D3DXVECTOR3 g_MoveTbl8[2]; // level6 --> enemy[9]
static D3DXVECTOR3 g_MoveTbl9[2]; // level7 --> enemy[11]
static D3DXVECTOR3 g_MoveTbl10[2]; //level8 --> enemy[14]
static D3DXVECTOR3 g_MoveTbl11[2]; //level8 --> enemy[15]
static D3DXVECTOR3 g_MoveTbl12[2]; //level9 --> enemy[21]
static D3DXVECTOR3 g_MoveTbl13[2]; //level11 --> enemy[23]
static D3DXVECTOR3 g_MoveTbl14[2]; //level11 --> enemy[24]
static D3DXVECTOR3 g_MoveTbl15[2]; //level11 --> enemy[25]
static D3DXVECTOR3 g_MoveTbl16[2]; //level11 --> enemy[26]


//エネミーの線形移動用の移動スピードテーブル
//1.0 = arrived  ==> 1 frame %
static float g_MoveSpd[] = {
	0.003f,
	0.003f,
};
static float g_MoveSpd2[2]; // level2
static float g_MoveSpd3[2]; // level4
static float g_MoveSpd4[2]; // level5
static float g_MoveSpd5[2]; // level6
static float g_MoveSpd6[2]; // level6
static float g_MoveSpd7[2]; // level6
static float g_MoveSpd8[2]; // level6
static float g_MoveSpd9[2]; // level7
static float g_MoveSpd10[2]; //level8
static float g_MoveSpd11[2]; //level8
static float g_MoveSpd12[2]; //level9
static float g_MoveSpd13[2]; //level11
static float g_MoveSpd14[2]; //level11
static float g_MoveSpd15[2]; //level11
static float g_MoveSpd16[2]; //level11

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
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

	// エネミーの初期化
	for (int c = 0; c < Enemy_MAX; c++)
	{

		enemys[c].m_Use = true;
		enemys[c].m_w = enemySpritesHWDivData[0].width;
		enemys[c].m_h = enemySpritesHWDivData[0].height;
		enemys[c].m_Pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		enemys[c].m_Rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		enemys[c].m_TexNo = 0;

		enemys[c].m_CountAnim = 0;
		enemys[c].m_PatternAnim = 0;

		enemys[c].turnRight      = true;
		enemys[c].turnUp         = true;
		enemys[c].speed.x        = 0.0f;
		enemys[c].speed.y        = 0.0f;
		enemys[c].moveSpeed      = c * 0.5;
		enemys[c].maxSpeedY      = 5.0f;
		enemys[c].gravityScale   = 0.3f;
							     
		enemys[c].status         = A_Idle;
		enemys[c].statusPrev     = 0;
							     
		enemys[c].m_setUnUseTime = ENEMY_SETUNUSEDTIME;
		enemys[c].m_isElimated   = false;

		enemys[c].reborntime     = SLIME_REBORN_TIME;
		enemys[c].rebornPos      = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		enemys[c].enemyColor     = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		enemys[c].slimeType      = none;

		enemys[c].TEXTURE_PATTERN_DIVIDE_X = 1;
		enemys[c].TEXTURE_PATTERN_DIVIDE_Y = 1;
		enemys[c].ANIM_PATTERN_NUM         = 1;

		enemys[c].animDataNo               = 0;
		enemys[c].b_changeAnimInSameState  = false;
		//線形移動 temp
		enemys[c].LMtime        = 0.0f;

	}

	//Anim Setting    int TEXTURE_PATTERN_DIVIDE_X, TEXTURE_PATTERN_DIVIDE_Y; int ANIM_PATTERN_NUM = TEXTURE_PATTERN_DIVIDE_X * TEXTURE_PATTERN_DIVIDE_Y;
	
	// Set Level Enemy here
	// Level 1
	enemys[0].m_TexNo = Bunny;
	enemys[0].m_enemyType = Bunny;
	enemys[0].m_Pos = D3DXVECTOR3(25.0f * GetGroundTextureWidth(), 18.0f * GetGroundTextureWidth(), 0.0f);
	enemys[0].m_Pos.x += 1 * SCREEN_WIDTH;
	enemys[0].moveSpeed = 0.5f;
	enemys[0].m_w = enemySpritesHWDivData[Bunny].width;
	enemys[0].m_h = enemySpritesHWDivData[Bunny].height;
	enemys[0].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bunny].TEXTURE_PATTERN_DIVIDE_X;
	enemys[0].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bunny].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[0].ANIM_PATTERN_NUM = enemys[0].TEXTURE_PATTERN_DIVIDE_X * enemys[0].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[0].rebornPos = enemys[0].m_Pos;
	enemys[0].reborntime = MONSTER_REBORN_TIME;

	// Level 2
	enemys[1].m_TexNo = Slime;
	enemys[1].m_enemyType = Slime;
	enemys[1].m_Pos = D3DXVECTOR3(15.0f * GetGroundTextureWidth(), 12.0f * GetGroundTextureWidth(), 0.0f);
	enemys[1].m_Pos.x += 2 * SCREEN_WIDTH;
	g_MoveTbl2[0] = D3DXVECTOR3(2 * SCREEN_WIDTH + GetGroundTextureWidth() * 15.0f, GetGroundTextureWidth() * 12.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl2[1] = D3DXVECTOR3(2 * SCREEN_WIDTH + GetGroundTextureWidth() * 20.0f, GetGroundTextureWidth() * 12.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd2[0] = 0.01f;
	g_MoveSpd2[1] = 0.01f;
	enemys[1].m_w = enemySpritesHWDivData[Slime].width;
	enemys[1].m_h = enemySpritesHWDivData[Slime].height;
	enemys[1].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[1].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[1].ANIM_PATTERN_NUM = enemys[1].TEXTURE_PATTERN_DIVIDE_X * enemys[1].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[1].rebornPos = enemys[1].m_Pos;
	enemys[1].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[1].slimeType = Aqua;

	// Level 3
	enemys[2].m_TexNo = Slime;
	enemys[2].m_enemyType = Slime;
	enemys[2].m_Pos = D3DXVECTOR3(25.0f * GetGroundTextureWidth(), 18.0f * GetGroundTextureWidth(), 0.0f);
	enemys[2].m_Pos.x += 3 * SCREEN_WIDTH;
	enemys[2].m_w = enemySpritesHWDivData[Slime].width;
	enemys[2].m_h = enemySpritesHWDivData[Slime].height;
	enemys[2].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[2].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[2].ANIM_PATTERN_NUM = enemys[2].TEXTURE_PATTERN_DIVIDE_X * enemys[2].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[2].rebornPos = enemys[2].m_Pos;
	enemys[2].enemyColor = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	enemys[2].slimeType = wind;

	// Level 4
	enemys[3].m_TexNo = Turtle;
	enemys[3].m_enemyType = Turtle;
	enemys[3].m_Pos = D3DXVECTOR3(17.0f * GetGroundTextureWidth(), 14.0f * GetGroundTextureWidth(), 0.0f);
	enemys[3].m_Pos.x += 4 * SCREEN_WIDTH;
	enemys[3].moveSpeed = 0.5f;
	g_MoveTbl3[0] = D3DXVECTOR3(4 * SCREEN_WIDTH + GetGroundTextureWidth() * 17.0f, GetGroundTextureWidth() * 14.0f - enemySpritesHWDivData[0].height / 2.65f, 0.0f);
	g_MoveTbl3[1] = D3DXVECTOR3(4 * SCREEN_WIDTH + GetGroundTextureWidth() * 36.0f, GetGroundTextureWidth() * 14.0f - enemySpritesHWDivData[0].height / 2.65f, 0.0f);
	g_MoveSpd3[0] = 0.002f;
	g_MoveSpd3[1] = 0.002f;
	enemys[3].m_w = enemySpritesHWDivData[Turtle].width;
	enemys[3].m_h = enemySpritesHWDivData[Turtle].height;
	enemys[3].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_X;
	enemys[3].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[3].ANIM_PATTERN_NUM = enemys[3].TEXTURE_PATTERN_DIVIDE_X * enemys[3].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[3].rebornPos = enemys[3].m_Pos;
	enemys[3].reborntime = TURTLE_REBORN_TIME;

	enemys[4].m_TexNo = Slime;
	enemys[4].m_enemyType = Slime;
	enemys[4].m_Pos = D3DXVECTOR3(9.5f * GetGroundTextureWidth(), 19.0f * GetGroundTextureWidth(), 0.0f);
	enemys[4].m_Pos.x += 4 * SCREEN_WIDTH;
	enemys[4].moveSpeed = 0.0f;
	enemys[4].turnRight = false;
	enemys[4].m_w = enemySpritesHWDivData[Slime].width;
	enemys[4].m_h = enemySpritesHWDivData[Slime].height;
	enemys[4].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[4].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[4].ANIM_PATTERN_NUM = enemys[4].TEXTURE_PATTERN_DIVIDE_X * enemys[4].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[4].rebornPos = enemys[4].m_Pos;
	enemys[4].enemyColor = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	enemys[4].slimeType = wind;

	// Level 5
	enemys[5].m_TexNo = Slime;
	enemys[5].m_enemyType = Slime;
	enemys[5].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 37.0f, GetGroundTextureWidth() * 17.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[5].m_Pos.x += 5 * SCREEN_WIDTH;
	enemys[5].moveSpeed = 0.5f;
	g_MoveTbl4[0] = D3DXVECTOR3(5 * SCREEN_WIDTH + GetGroundTextureWidth() * 37.0f, GetGroundTextureWidth() * 17.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl4[1] = D3DXVECTOR3(5 * SCREEN_WIDTH + GetGroundTextureWidth() * 19.0f, GetGroundTextureWidth() * 17.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd4[0] = 0.002f;
	g_MoveSpd4[1] = 0.002f;
	enemys[5].m_w = enemySpritesHWDivData[Slime].width;
	enemys[5].m_h = enemySpritesHWDivData[Slime].height;
	enemys[5].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[5].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[5].ANIM_PATTERN_NUM = enemys[5].TEXTURE_PATTERN_DIVIDE_X * enemys[5].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[5].rebornPos = enemys[5].m_Pos;
	enemys[5].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[5].slimeType = Aqua;

	//Level 6
	enemys[6].m_TexNo = Bird;
	enemys[6].m_enemyType = Bird;
	enemys[6].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 16, GetGroundTextureWidth() *  4 - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[6].m_Pos.x += 6 * SCREEN_WIDTH;
	enemys[6].moveSpeed = 0.5f;
	g_MoveTbl5[0] = D3DXVECTOR3(6 * SCREEN_WIDTH + GetGroundTextureWidth() * 16.0f, GetGroundTextureWidth() *  4.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl5[1] = D3DXVECTOR3(6 * SCREEN_WIDTH + GetGroundTextureWidth() * 16.0f, GetGroundTextureWidth() * 18.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd5[0] = 0.0035f;
	g_MoveSpd5[1] = 0.0035f;
	enemys[6].m_w = enemySpritesHWDivData[Bird].width;
	enemys[6].m_h = enemySpritesHWDivData[Bird].height;
	enemys[6].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_X;
	enemys[6].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[6].ANIM_PATTERN_NUM = enemys[6].TEXTURE_PATTERN_DIVIDE_X * enemys[6].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[6].rebornPos = enemys[6].m_Pos;
	enemys[6].reborntime = MONSTER_REBORN_TIME;
	enemys[6].turnRight = false;

	enemys[7].m_TexNo = Bird;
	enemys[7].m_enemyType = Bird;
	enemys[7].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 22.0f, GetGroundTextureWidth() * 18.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[7].m_Pos.x += 6 * SCREEN_WIDTH;
	enemys[7].moveSpeed = 0.5f;
	g_MoveTbl6[0] = D3DXVECTOR3(6 * SCREEN_WIDTH + GetGroundTextureWidth() * 22.0f, GetGroundTextureWidth() * 18.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl6[1] = D3DXVECTOR3(6 * SCREEN_WIDTH + GetGroundTextureWidth() * 22.0f, GetGroundTextureWidth() * 4.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd6[0] = 0.005f;
	g_MoveSpd6[1] = 0.005f;
	enemys[7].m_w = enemySpritesHWDivData[Bird].width;
	enemys[7].m_h = enemySpritesHWDivData[Bird].height;
	enemys[7].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_X;
	enemys[7].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[7].ANIM_PATTERN_NUM = enemys[7].TEXTURE_PATTERN_DIVIDE_X * enemys[7].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[7].rebornPos = enemys[7].m_Pos;
	enemys[7].reborntime = MONSTER_REBORN_TIME;
	enemys[7].turnRight = false;

	enemys[8].m_TexNo = Bird;
	enemys[8].m_enemyType = Bird;
	enemys[8].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 29.0f, GetGroundTextureWidth() * 4.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[8].m_Pos.x += 6 * SCREEN_WIDTH;
	enemys[8].moveSpeed = 0.5f;
	g_MoveTbl7[0] = D3DXVECTOR3(6 * SCREEN_WIDTH + GetGroundTextureWidth() * 29.0f, GetGroundTextureWidth() * 4.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl7[1] = D3DXVECTOR3(6 * SCREEN_WIDTH + GetGroundTextureWidth() * 29.0f, GetGroundTextureWidth() * 18.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd7[0] = 0.006f;
	g_MoveSpd7[1] = 0.006f;
	enemys[8].m_w = enemySpritesHWDivData[Bird].width;
	enemys[8].m_h = enemySpritesHWDivData[Bird].height;
	enemys[8].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_X;
	enemys[8].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[8].ANIM_PATTERN_NUM = enemys[8].TEXTURE_PATTERN_DIVIDE_X * enemys[8].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[8].rebornPos = enemys[8].m_Pos;
	enemys[8].reborntime = MONSTER_REBORN_TIME;
	enemys[8].turnRight = false;

	enemys[9].m_TexNo = Bird;
	enemys[9].m_enemyType = Bird;
	enemys[9].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 35.0f, GetGroundTextureWidth() * 18.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[9].m_Pos.x += 6 * SCREEN_WIDTH;
	enemys[9].moveSpeed = 0.5f;
	g_MoveTbl8[0] = D3DXVECTOR3(6 * SCREEN_WIDTH + GetGroundTextureWidth() * 35.0f, GetGroundTextureWidth() * 18.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl8[1] = D3DXVECTOR3(6 * SCREEN_WIDTH + GetGroundTextureWidth() * 35.0f, GetGroundTextureWidth() * 4.0f  - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd8[0] = 0.0055f;
	g_MoveSpd8[1] = 0.0055f;
	enemys[9].m_w = enemySpritesHWDivData[Bird].width;
	enemys[9].m_h = enemySpritesHWDivData[Bird].height;
	enemys[9].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_X;
	enemys[9].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[9].ANIM_PATTERN_NUM = enemys[9].TEXTURE_PATTERN_DIVIDE_X * enemys[9].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[9].rebornPos = enemys[9].m_Pos;
	enemys[9].reborntime = MONSTER_REBORN_TIME;
	enemys[9].turnRight = false;

	enemys[10].m_TexNo = Slime;
	enemys[10].m_enemyType = Slime;
	enemys[10].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 11.5f, GetGroundTextureWidth() * 14.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[10].m_Pos.x += 6 * SCREEN_WIDTH;
	enemys[10].moveSpeed = 0.0f;
	enemys[10].m_w = enemySpritesHWDivData[Slime].width;
	enemys[10].m_h = enemySpritesHWDivData[Slime].height;
	enemys[10].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[10].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[10].ANIM_PATTERN_NUM = enemys[10].TEXTURE_PATTERN_DIVIDE_X * enemys[10].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[10].rebornPos = enemys[10].m_Pos;
	enemys[10].turnRight = false;
	enemys[10].enemyColor = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	enemys[10].slimeType = wind;

	//Level 7
	enemys[11].m_TexNo = Bird;
	enemys[11].m_enemyType = Bird;
	enemys[11].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 22.0f, GetGroundTextureWidth() * 11.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[11].m_Pos.x += 7 * SCREEN_WIDTH;
	enemys[11].moveSpeed = 0.5f;
	g_MoveTbl9[0] = D3DXVECTOR3(7 * SCREEN_WIDTH + GetGroundTextureWidth() * 22.0f, GetGroundTextureWidth() * 10.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl9[1] = D3DXVECTOR3(7 * SCREEN_WIDTH + GetGroundTextureWidth() * 22.0f, GetGroundTextureWidth() * 16.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd9[0] = 0.0045f;
	g_MoveSpd9[1] = 0.0045f;
	enemys[11].m_w = enemySpritesHWDivData[Bird].width;
	enemys[11].m_h = enemySpritesHWDivData[Bird].height;
	enemys[11].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_X;
	enemys[11].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[11].ANIM_PATTERN_NUM = enemys[11].TEXTURE_PATTERN_DIVIDE_X * enemys[11].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[11].rebornPos = enemys[11].m_Pos;
	enemys[11].reborntime = MONSTER_REBORN_TIME;
	enemys[11].turnRight = false;

	enemys[12].m_TexNo = Slime;
	enemys[12].m_enemyType = Slime;
	enemys[12].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() *  9.5f, GetGroundTextureWidth() * 15.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[12].m_Pos.x += 7 * SCREEN_WIDTH;
	enemys[12].moveSpeed = 0.0f;
	enemys[12].m_w = enemySpritesHWDivData[Slime].width;
	enemys[12].m_h = enemySpritesHWDivData[Slime].height;
	enemys[12].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[12].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[12].ANIM_PATTERN_NUM = enemys[12].TEXTURE_PATTERN_DIVIDE_X * enemys[12].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[12].rebornPos = enemys[12].m_Pos;
	enemys[12].turnRight = false;
	enemys[12].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[12].slimeType = Aqua;

	//Level 8
	enemys[13].m_TexNo = Slime;
	enemys[13].m_enemyType = Slime;
	enemys[13].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() *  16.5f, GetGroundTextureWidth() * 5.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[13].m_Pos.x += 8 * SCREEN_WIDTH;
	enemys[13].moveSpeed = 0.0f;
	enemys[13].m_w = enemySpritesHWDivData[Slime].width;
	enemys[13].m_h = enemySpritesHWDivData[Slime].height;
	enemys[13].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[13].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[13].ANIM_PATTERN_NUM = enemys[12].TEXTURE_PATTERN_DIVIDE_X * enemys[12].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[13].rebornPos = enemys[13].m_Pos;
	enemys[13].turnRight = false;
	enemys[13].enemyColor = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	enemys[13].slimeType = wind;

	enemys[14].m_TexNo = Bird;
	enemys[14].m_enemyType = Bird;
	enemys[14].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 25.0f, GetGroundTextureWidth() * 10.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[14].m_Pos.x += 8 * SCREEN_WIDTH;
	enemys[14].moveSpeed = 0.5f;
	g_MoveTbl10[0] = D3DXVECTOR3(8 * SCREEN_WIDTH + GetGroundTextureWidth() * 25.0f, GetGroundTextureWidth() * 10.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl10[1] = D3DXVECTOR3(8 * SCREEN_WIDTH + GetGroundTextureWidth() * 25.0f, GetGroundTextureWidth() * 15.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd10[0] = 0.007f;
	g_MoveSpd10[1] = 0.007f;
	enemys[14].m_w = enemySpritesHWDivData[Bird].width;
	enemys[14].m_h = enemySpritesHWDivData[Bird].height;
	enemys[14].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_X;
	enemys[14].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[14].ANIM_PATTERN_NUM = enemys[14].TEXTURE_PATTERN_DIVIDE_X * enemys[14].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[14].rebornPos = enemys[14].m_Pos;
	enemys[14].reborntime = MONSTER_REBORN_TIME;
	enemys[14].turnRight = false;

	enemys[15].m_TexNo = Bird;
	enemys[15].m_enemyType = Bird;
	enemys[15].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 16.0f, GetGroundTextureWidth() * 20.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[15].m_Pos.x += 8 * SCREEN_WIDTH;
	enemys[15].moveSpeed = 0.5f;
	g_MoveTbl11[0] = D3DXVECTOR3(8 * SCREEN_WIDTH + GetGroundTextureWidth() * 16.0f, GetGroundTextureWidth() * 20.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl11[1] = D3DXVECTOR3(8 * SCREEN_WIDTH + GetGroundTextureWidth() * 25.0f, GetGroundTextureWidth() * 20.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd11[0] = 0.008f;
	g_MoveSpd11[1] = 0.008f;
	enemys[15].m_w = enemySpritesHWDivData[Bird].width;
	enemys[15].m_h = enemySpritesHWDivData[Bird].height;
	enemys[15].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_X;
	enemys[15].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[15].ANIM_PATTERN_NUM = enemys[15].TEXTURE_PATTERN_DIVIDE_X * enemys[15].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[15].rebornPos = enemys[15].m_Pos;
	enemys[15].reborntime = MONSTER_REBORN_TIME;
	enemys[15].turnRight = false;

	enemys[16].m_TexNo = Slime;
	enemys[16].m_enemyType = Slime;
	enemys[16].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 3.5f, GetGroundTextureWidth() * 13.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[16].m_Pos.x += 8 * SCREEN_WIDTH;
	enemys[16].moveSpeed = 0.0f;
	enemys[16].m_w = enemySpritesHWDivData[Slime].width;
	enemys[16].m_h = enemySpritesHWDivData[Slime].height;
	enemys[16].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[16].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[16].ANIM_PATTERN_NUM = enemys[16].TEXTURE_PATTERN_DIVIDE_X * enemys[16].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[16].rebornPos = enemys[16].m_Pos;
	enemys[16].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[16].slimeType = Aqua;

	enemys[17].m_TexNo = Turtle;
	enemys[17].m_enemyType = Turtle;
	enemys[17].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() *  28.5f, GetGroundTextureWidth() * 5.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[17].m_Pos.x += 8 * SCREEN_WIDTH;
	enemys[17].moveSpeed = 0.0f;
	enemys[17].m_w = enemySpritesHWDivData[Turtle].width;
	enemys[17].m_h = enemySpritesHWDivData[Turtle].height;
	enemys[17].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_X;
	enemys[17].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[17].ANIM_PATTERN_NUM = enemys[17].TEXTURE_PATTERN_DIVIDE_X * enemys[17].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[17].rebornPos = enemys[17].m_Pos;
	enemys[17].reborntime = TURTLE_REBORN_TIME;
	enemys[17].turnRight = false;

	// Level 9
	enemys[18].m_TexNo = Slime;
	enemys[18].m_enemyType = Slime;
	enemys[18].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 5.5f, GetGroundTextureWidth() * 19.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[18].m_Pos.x += 9 * SCREEN_WIDTH;
	enemys[18].moveSpeed = 0.0f;
	enemys[18].m_w = enemySpritesHWDivData[Slime].width;
	enemys[18].m_h = enemySpritesHWDivData[Slime].height;
	enemys[18].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[18].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[18].ANIM_PATTERN_NUM = enemys[18].TEXTURE_PATTERN_DIVIDE_X * enemys[18].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[18].rebornPos = enemys[18].m_Pos;
	enemys[18].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[18].slimeType = Aqua;

	enemys[19].m_TexNo = Slime;
	enemys[19].m_enemyType = Slime;
	enemys[19].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 27.5f, GetGroundTextureWidth() * 5.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[19].m_Pos.x += 9 * SCREEN_WIDTH;
	enemys[19].moveSpeed = 0.0f;
	enemys[19].m_w = enemySpritesHWDivData[Slime].width;
	enemys[19].m_h = enemySpritesHWDivData[Slime].height;
	enemys[19].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[19].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[19].ANIM_PATTERN_NUM = enemys[19].TEXTURE_PATTERN_DIVIDE_X * enemys[19].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[19].rebornPos = enemys[19].m_Pos;
	enemys[19].enemyColor = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	enemys[19].slimeType = wind;
	enemys[19].turnRight = false;

	enemys[20].m_TexNo = Turtle;
	enemys[20].m_enemyType = Turtle;
	enemys[20].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() *  37.5f, GetGroundTextureWidth() * 5.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[20].m_Pos.x += 9 * SCREEN_WIDTH;
	enemys[20].moveSpeed = 0.0f;
	enemys[20].m_w = enemySpritesHWDivData[Turtle].width;
	enemys[20].m_h = enemySpritesHWDivData[Turtle].height;
	enemys[20].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_X;
	enemys[20].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[20].ANIM_PATTERN_NUM = enemys[20].TEXTURE_PATTERN_DIVIDE_X * enemys[20].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[20].rebornPos = enemys[20].m_Pos;
	enemys[20].reborntime = TURTLE_REBORN_TIME;
	enemys[20].turnRight = false;

	enemys[21].m_TexNo = Bunny;
	enemys[21].m_enemyType = Bunny;
	enemys[21].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 30.0f, GetGroundTextureWidth() * 21.0f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	enemys[21].m_Pos.x += 9 * SCREEN_WIDTH;
	enemys[21].moveSpeed = 0.5f;
	g_MoveTbl12[0] = D3DXVECTOR3(9 * SCREEN_WIDTH + GetGroundTextureWidth() * 30.0f, GetGroundTextureWidth() * 21.0f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	g_MoveTbl12[1] = D3DXVECTOR3(9 * SCREEN_WIDTH + GetGroundTextureWidth() * 37.0f, GetGroundTextureWidth() * 21.0f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	g_MoveSpd12[0] = 0.008f;
	g_MoveSpd12[1] = 0.008f;
	enemys[21].m_w = enemySpritesHWDivData[Bunny].width;
	enemys[21].m_h = enemySpritesHWDivData[Bunny].height;
	enemys[21].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bunny].TEXTURE_PATTERN_DIVIDE_X;
	enemys[21].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bunny].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[21].ANIM_PATTERN_NUM = enemys[21].TEXTURE_PATTERN_DIVIDE_X * enemys[21].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[21].rebornPos = enemys[21].m_Pos;
	enemys[21].reborntime = MONSTER_REBORN_TIME;
	enemys[21].turnRight = false;

	// Level 10
	enemys[22].m_TexNo = Slime;
	enemys[22].m_enemyType = Slime;
	enemys[22].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 2.5f, GetGroundTextureWidth() * 14.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[22].m_Pos.x += 10 * SCREEN_WIDTH;
	enemys[22].moveSpeed = 0.0f;
	enemys[22].m_w = enemySpritesHWDivData[Slime].width;
	enemys[22].m_h = enemySpritesHWDivData[Slime].height;
	enemys[22].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[22].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[22].ANIM_PATTERN_NUM = enemys[22].TEXTURE_PATTERN_DIVIDE_X * enemys[22].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[22].rebornPos = enemys[22].m_Pos;
	enemys[22].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[22].slimeType = Aqua;

	// Level 11
	enemys[23].m_TexNo = Turtle;
	enemys[23].m_enemyType = Turtle;
	enemys[23].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 2.0f, GetGroundTextureWidth() * 10.5f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	enemys[23].m_Pos.x += 11 * SCREEN_WIDTH;
	enemys[23].moveSpeed = 0.5f;
	g_MoveTbl13[0] = D3DXVECTOR3(11 * SCREEN_WIDTH + GetGroundTextureWidth() * 2.0f, GetGroundTextureWidth() *  10.5f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	g_MoveTbl13[1] = D3DXVECTOR3(11 * SCREEN_WIDTH + GetGroundTextureWidth() * 30.0f, GetGroundTextureWidth() * 10.5f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	g_MoveSpd13[0] = 0.008f;
	g_MoveSpd13[1] = 0.008f;
	enemys[23].m_w = enemySpritesHWDivData[Turtle].width;
	enemys[23].m_h = enemySpritesHWDivData[Turtle].height;
	enemys[23].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_X;
	enemys[23].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[23].ANIM_PATTERN_NUM = enemys[23].TEXTURE_PATTERN_DIVIDE_X * enemys[23].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[23].rebornPos = enemys[23].m_Pos;
	enemys[23].reborntime = MONSTER_REBORN_TIME;

	enemys[24].m_TexNo = Turtle;
	enemys[24].m_enemyType = Turtle;
	enemys[24].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 38.0f, GetGroundTextureWidth() * 15.5f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	enemys[24].m_Pos.x += 11 * SCREEN_WIDTH;
	enemys[24].moveSpeed = 0.5f;
	g_MoveTbl14[0] = D3DXVECTOR3(11 * SCREEN_WIDTH + GetGroundTextureWidth() * 38.0f, GetGroundTextureWidth() * 15.5f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	g_MoveTbl14[1] = D3DXVECTOR3(11 * SCREEN_WIDTH + GetGroundTextureWidth() * 10.0f, GetGroundTextureWidth() * 15.5f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	g_MoveSpd14[0] = 0.005f;
	g_MoveSpd14[1] = 0.005f;
	enemys[24].m_w = enemySpritesHWDivData[Turtle].width;
	enemys[24].m_h = enemySpritesHWDivData[Turtle].height;
	enemys[24].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_X;
	enemys[24].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[24].ANIM_PATTERN_NUM = enemys[24].TEXTURE_PATTERN_DIVIDE_X * enemys[24].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[24].rebornPos = enemys[24].m_Pos;
	enemys[24].reborntime = MONSTER_REBORN_TIME;

	enemys[25].m_TexNo = Bird;
	enemys[25].m_enemyType = Bird;
	enemys[25].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 2.0f, GetGroundTextureWidth() * 19.0f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	enemys[25].m_Pos.x += 11 * SCREEN_WIDTH;
	enemys[25].moveSpeed = 0.5f;
	g_MoveTbl15[0] = D3DXVECTOR3(11 * SCREEN_WIDTH + GetGroundTextureWidth() * 2.0f, GetGroundTextureWidth() * 19.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl15[1] = D3DXVECTOR3(11 * SCREEN_WIDTH + GetGroundTextureWidth() * 20.0f, GetGroundTextureWidth() * 19.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd15[0] = 0.007f;
	g_MoveSpd15[1] = 0.007f;
	enemys[25].m_w = enemySpritesHWDivData[Bird].width;
	enemys[25].m_h = enemySpritesHWDivData[Bird].height;
	enemys[25].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_X;
	enemys[25].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[25].ANIM_PATTERN_NUM = enemys[25].TEXTURE_PATTERN_DIVIDE_X * enemys[25].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[25].rebornPos = enemys[25].m_Pos;
	enemys[25].reborntime = MONSTER_REBORN_TIME;
	enemys[25].turnRight = false;

	enemys[26].m_TexNo = Bird;
	enemys[26].m_enemyType = Bird;
	enemys[26].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 38.0f, GetGroundTextureWidth() * 19.0f - enemySpritesHWDivData[0].height / 1.6f, 0.0f);
	enemys[26].m_Pos.x += 11 * SCREEN_WIDTH;
	enemys[26].moveSpeed = 0.5f;
	g_MoveTbl16[0] = D3DXVECTOR3(11 * SCREEN_WIDTH + GetGroundTextureWidth() * 38.0f, GetGroundTextureWidth() * 19.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveTbl16[1] = D3DXVECTOR3(11 * SCREEN_WIDTH + GetGroundTextureWidth() * 22.0f, GetGroundTextureWidth() * 19.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	g_MoveSpd16[0] = 0.007f;
	g_MoveSpd16[1] = 0.007f;
	enemys[26].m_w = enemySpritesHWDivData[Bird].width;
	enemys[26].m_h = enemySpritesHWDivData[Bird].height;
	enemys[26].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_X;
	enemys[26].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bird].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[26].ANIM_PATTERN_NUM = enemys[26].TEXTURE_PATTERN_DIVIDE_X * enemys[26].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[26].rebornPos = enemys[26].m_Pos;
	enemys[26].reborntime = MONSTER_REBORN_TIME;
	enemys[26].turnRight = false;

	// Level 12
	enemys[27].m_TexNo = Slime;
	enemys[27].m_enemyType = Slime;
	enemys[27].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 2.5f, GetGroundTextureWidth() * 14.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[27].m_Pos.x += 12 * SCREEN_WIDTH;
	enemys[27].moveSpeed = 0.0f;
	enemys[27].m_w = enemySpritesHWDivData[Slime].width;
	enemys[27].m_h = enemySpritesHWDivData[Slime].height;
	enemys[27].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[27].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[27].ANIM_PATTERN_NUM = enemys[27].TEXTURE_PATTERN_DIVIDE_X * enemys[27].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[27].rebornPos = enemys[27].m_Pos;
	enemys[27].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[27].slimeType = Aqua;

	enemys[28].m_TexNo = Slime;
	enemys[28].m_enemyType = Slime;
	enemys[28].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 2.5f, GetGroundTextureWidth() * 2.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[28].m_Pos.x += 12 * SCREEN_WIDTH;
	enemys[28].moveSpeed = 0.0f;
	enemys[28].m_w = enemySpritesHWDivData[Slime].width;
	enemys[28].m_h = enemySpritesHWDivData[Slime].height;
	enemys[28].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[28].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[28].ANIM_PATTERN_NUM = enemys[28].TEXTURE_PATTERN_DIVIDE_X * enemys[28].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[28].rebornPos = enemys[28].m_Pos;
	enemys[28].enemyColor = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	enemys[28].slimeType = wind;

	// Level 12
	enemys[29].m_TexNo = Turtle;
	enemys[29].m_enemyType = Turtle;
	enemys[29].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() *  37.5f, GetGroundTextureWidth() * 17.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[29].m_Pos.x += 12 * SCREEN_WIDTH;
	enemys[29].moveSpeed = 0.0f;
	enemys[29].m_w = enemySpritesHWDivData[Turtle].width;
	enemys[29].m_h = enemySpritesHWDivData[Turtle].height;
	enemys[29].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_X;
	enemys[29].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Turtle].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[29].ANIM_PATTERN_NUM = enemys[29].TEXTURE_PATTERN_DIVIDE_X * enemys[29].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[29].rebornPos = enemys[29].m_Pos;
	enemys[29].reborntime = TURTLE_REBORN_TIME;
	enemys[29].turnRight = false;

	// Level 13
	enemys[30].m_TexNo = Slime;
	enemys[30].m_enemyType = Slime;
	enemys[30].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 36.5f, GetGroundTextureWidth() * 19.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[30].m_Pos.x += 13 * SCREEN_WIDTH;
	enemys[30].moveSpeed = 0.0f;
	enemys[30].m_w = enemySpritesHWDivData[Slime].width;
	enemys[30].m_h = enemySpritesHWDivData[Slime].height;
	enemys[30].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[30].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[30].ANIM_PATTERN_NUM = enemys[30].TEXTURE_PATTERN_DIVIDE_X * enemys[30].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[30].rebornPos = enemys[30].m_Pos;
	enemys[30].enemyColor = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	enemys[30].slimeType = wind;
	enemys[30].turnRight = false;

	enemys[31].m_TexNo = Slime;
	enemys[31].m_enemyType = Slime;
	enemys[31].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 31.5f, GetGroundTextureWidth() * 19.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[31].m_Pos.x += 13 * SCREEN_WIDTH;
	enemys[31].moveSpeed = 0.0f;
	enemys[31].m_w = enemySpritesHWDivData[Slime].width;
	enemys[31].m_h = enemySpritesHWDivData[Slime].height;
	enemys[31].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[31].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[31].ANIM_PATTERN_NUM = enemys[31].TEXTURE_PATTERN_DIVIDE_X * enemys[31].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[31].rebornPos = enemys[31].m_Pos;
	enemys[31].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[31].slimeType = Aqua;
	enemys[31].turnRight = false;

	enemys[32].m_TexNo = Slime;
	enemys[32].m_enemyType = Slime;
	enemys[32].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 31.5f, GetGroundTextureWidth() * 16.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[32].m_Pos.x += 13 * SCREEN_WIDTH;
	enemys[32].moveSpeed = 0.0f;
	enemys[32].m_w = enemySpritesHWDivData[Slime].width;
	enemys[32].m_h = enemySpritesHWDivData[Slime].height;
	enemys[32].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[32].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[32].ANIM_PATTERN_NUM = enemys[32].TEXTURE_PATTERN_DIVIDE_X * enemys[32].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[32].rebornPos = enemys[32].m_Pos;
	enemys[32].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[32].slimeType = Aqua;
	enemys[32].turnRight = false;

	enemys[33].m_TexNo = Slime;
	enemys[33].m_enemyType = Slime;
	enemys[33].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() * 25.5f, GetGroundTextureWidth() * 6.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[33].m_Pos.x += 13 * SCREEN_WIDTH;
	enemys[33].moveSpeed = 0.0f;
	enemys[33].m_w = enemySpritesHWDivData[Slime].width;
	enemys[33].m_h = enemySpritesHWDivData[Slime].height;
	enemys[33].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_X;
	enemys[33].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Slime].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[33].ANIM_PATTERN_NUM = enemys[33].TEXTURE_PATTERN_DIVIDE_X * enemys[33].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[33].rebornPos = enemys[33].m_Pos;
	enemys[33].enemyColor = D3DXCOLOR(0.5f, 0.5f, 1.5f, 1.0f);
	enemys[33].slimeType = Aqua;
	enemys[33].turnRight = false;

	enemys[34].m_TexNo = Bunny;
	enemys[34].m_enemyType = Bunny;
	enemys[34].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() *  28.5f, GetGroundTextureWidth() * 11.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[34].m_Pos.x += 13 * SCREEN_WIDTH;
	enemys[34].moveSpeed = 0.0f;
	enemys[34].m_w = enemySpritesHWDivData[Bunny].width;
	enemys[34].m_h = enemySpritesHWDivData[Bunny].height;
	enemys[34].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Bunny].TEXTURE_PATTERN_DIVIDE_X;
	enemys[34].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Bunny].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[34].ANIM_PATTERN_NUM = enemys[34].TEXTURE_PATTERN_DIVIDE_X * enemys[34].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[34].rebornPos = enemys[34].m_Pos;
	enemys[34].reborntime = TURTLE_REBORN_TIME;
	enemys[34].turnRight = false;

	// Level 14
	enemys[35].m_TexNo = Flag;
	enemys[35].m_enemyType = Flag;
	enemys[35].m_Pos = D3DXVECTOR3(GetGroundTextureWidth() *  33.0f, GetGroundTextureWidth() * 15.0f - enemySpritesHWDivData[0].height / 2.25f, 0.0f);
	enemys[35].m_Pos.x += 14 * SCREEN_WIDTH;
	enemys[35].moveSpeed = 0.0f;
	enemys[35].m_w = enemySpritesHWDivData[Flag].width;
	enemys[35].m_h = enemySpritesHWDivData[Flag].height;
	enemys[35].TEXTURE_PATTERN_DIVIDE_X = enemySpritesHWDivData[Flag].TEXTURE_PATTERN_DIVIDE_X;
	enemys[35].TEXTURE_PATTERN_DIVIDE_Y = enemySpritesHWDivData[Flag].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[35].ANIM_PATTERN_NUM = enemys[35].TEXTURE_PATTERN_DIVIDE_X * enemys[35].TEXTURE_PATTERN_DIVIDE_Y;
	enemys[35].rebornPos = enemys[35].m_Pos;
	enemys[35].reborntime = TURTLE_REBORN_TIME;
	enemys[35].turnRight = false;

	//Collider2d
	SetCollider(enemysCollider2d, Rect, D3DXVECTOR2(enemys[0].m_Pos.x, enemys[0].m_Pos.y), 0.0f, enemys[0].m_w / 2.0f, enemys[0].m_h / 2.0f, enemys[0].m_Use);
	AddColliderInPool(enemysCollider2d, 1);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
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
void UpdateEnemy(void)
{
	for (int c = 0; c < Enemy_MAX; c++)
	{
		if (enemys[c].m_Use == true)		// プレイヤーが使われている？
		{				
			if (!IsStoppingFrame()) {

				float px = enemys[c].m_Pos.x - GetLevelX() * SCREEN_WIDTH;	// プレイヤーの表示位置X
				float py = enemys[c].m_Pos.y - GetLevelY() * SCREEN_HEIGHT;	// プレイヤーの表示位置Y
				if (px < SCREEN_WIDTH && px > 0) {    // Don't too close to frontier 画面以外のを更新しない

					//================Input
					InputUpdate(enemys[c]);

					//================Physics
					PhysicsUpdate(enemys[c]);

					//================Collision
					CollisionUpdate(enemys[c]);

					//================PlayerStatus
					enemyStatusUpdate(enemys[c]);

					// ================アニメーション 
					AnimUpdate(enemys[c]);
				}

			}
		}
	}

	//ライン移動
	linearMove();

	//Revive
	int enemyLeft = Enemy_MAX;

	for (int c = 0; c < Enemy_MAX; c++) {
		if (enemys[c].m_Use == false) enemyLeft--;
		//Enemy reborn
		if (enemys[c].m_Use == false) {
			enemys[c].reborntime -= 1;
			if (enemys[c].reborntime <= 0) {
				SetBullet(enemys[c].rebornPos, D3DXVECTOR3(1.0f, 0.0f, 0.0f), enemyReborn, c);
				enemys[c].reborntime = 100; // just dont let it <= 0
			}
		}
	}

    // SETIMER
	if(HitSETimer > 0)HitSETimer -= 1;
	

#ifdef _DEBUG	// デバッグ情報を表示する
	/*char *str = GetDebugStr();
	sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", m_Pos.x, m_Pos.y);*/

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
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

	for (int c = 0; c < Enemy_MAX; c++)
	{
		if (enemys[c].m_Use == true)		// プレイヤーが使われている？
		{						// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[enemys[c].m_TexNo]);

			// プレイヤーの位置やテクスチャー座標を反映
			float px = enemys[c].m_Pos.x - GetLevelX() * SCREEN_WIDTH;	// プレイヤーの表示位置X
			float py = enemys[c].m_Pos.y - GetLevelY() * SCREEN_HEIGHT;	// プレイヤーの表示位置Y
			if (px - enemys[c].m_w < SCREEN_WIDTH && px + enemys[c].m_w > 0)    // no need to draw 
			{
				float pw = enemys[c].m_w;		// プレイヤーの表示幅
				float ph = enemys[c].m_h;		// プレイヤーの表示高さ

				float tw = 1.0f / enemys[c].TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
				float th = 1.0f / enemys[c].TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
				float tx = ((float)(enemys[c].m_PatternAnim % enemys[c].TEXTURE_PATTERN_DIVIDE_X)) * tw;	// テクスチャの左上X座標
				float ty = ((float)(enemys[c].m_PatternAnim / enemys[c].TEXTURE_PATTERN_DIVIDE_X)) * th;	// テクスチャの左上Y座標

				float r = enemys[c].m_Rot.z;

				D3DXCOLOR color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
				if (enemys[c].m_enemyType == Slime)color = enemys[c].enemyColor;
				// １枚のポリゴンの頂点とテクスチャ座標を設定
				/*if (enemys[c].turnRight)SetSprite(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th);
				else SetSprite(g_VertexBuffer, px, py, pw, ph, tx + tw, ty, -tw, th);*/
				if (enemys[c].turnRight)SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx + tw, ty, -tw, th, color, r);
				else SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th, color, r);
				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}
}


//=============================================================================
// プレイヤーの座標を取得
//=============================================================================
enemyStruct *GetEnemy() {
	return &enemys[0];
}

D3DXVECTOR3 GetEnemyPosition(void)
{
	return enemys[0].m_Pos;
}


void ChangeStatus(enemyStruct * enemy, int status)
{
	enemy->status = status;
	if (enemy->statusPrev != enemy->status || enemy->b_changeAnimInSameState == true) {
		enemy->b_changeAnimInSameState = false;

		if (enemy->m_enemyType == Trunk) enemy->m_PatternAnim = TrunkSpritesData[enemy->status].Data[0];
		if (enemy->m_enemyType == Bunny) enemy->m_PatternAnim = BunnySpritesData[enemy->status].Data[0];
		if (enemy->m_enemyType == Slime) enemy->m_PatternAnim = SlimeSpritesData[enemy->status].Data[0];
		if(enemy->m_enemyType == Turtle) enemy->m_PatternAnim = TurtleSpritesData[enemy->status].Data[0];
		if (enemy->m_enemyType == Bird)  enemy->m_PatternAnim = BirdSpritesData[enemy->status].Data[0];
		if (enemy->m_enemyType == Flag)  enemy->m_PatternAnim = FlagSpritesData[enemy->status].Data[0];

		enemy->m_CountAnim = 0;
		enemy->animDataNo = 0;
	}
}

void InputUpdate(enemyStruct & enemy)
{
}

void PhysicsUpdate(enemyStruct & enemy)
{
	
	//自動移動 
	if (enemy.turnRight) {
		enemy.speed.x = +enemy.moveSpeed;
	}
	else {
		enemy.speed.x = -enemy.moveSpeed;
	}

	if (enemy.m_Pos.x + (enemy.m_w / 2) >= SCREEN_WIDTH + SCREEN_WIDTH * GetLevelX()) enemy.turnRight = false;
	else if (enemy.m_Pos.x - (enemy.m_w / 2) <= SCREEN_WIDTH * GetLevelX())enemy.turnRight = true;

	if (enemy.turnUp) {
		//enemy.speed.y = enemy.moveSpeed;
	}
	else {
		//enemy.speed.y = -enemy.moveSpeed;
	}

	if (enemy.m_Pos.y + (enemy.m_h / 2) >= SCREEN_HEIGHT) enemy.turnUp = true;
	else if (enemy.m_Pos.y - (enemy.m_h / 2) <= 0)enemy.turnUp = false;

	//Speed limited
	if (enemy.speed.y >= enemy.maxSpeedY)enemy.speed.y = enemy.maxSpeedY;
	else if (enemy.speed.y <= -enemy.maxSpeedY)enemy.speed.y = -enemy.maxSpeedY;

	//Gravity
	if (enemy.gravityScale > 0.0f) {
		if (enemy.m_Pos.y < SCREEN_HEIGHT - enemy.m_h / 2)enemy.speed.y += enemy.gravityScale;

		if (enemy.status != A_DEAD) {
			if (enemy.m_Pos.y > SCREEN_HEIGHT - enemy.m_h / 2) { // make sure not keep falling down
				enemy.m_Pos.y = SCREEN_HEIGHT - enemy.m_h / 2;
				enemy.speed.y = 0.0f;
			}
		}
	}

	//Ground Detect
	if (enemy.status != A_DEAD) {

		if (IsGround(enemy.m_Pos.x, enemy.m_Pos.y + enemySpritesHWDivData[enemy.m_TexNo].height / 2)) { // ground
			//enemy.m_Rot.z -= D3DXToRadian(6.0f); // Radian : 弧度 = Degree * PI/180.0f
			if (enemy.speed.y > 0.0f) {
				enemy.m_Pos.y = GetGroundCenterPos(enemy.m_Pos.x, enemy.m_Pos.y + enemySpritesHWDivData[enemy.m_TexNo].height / 2).y - GetGroundTextureWidth() / 2 - enemySpritesHWDivData[enemy.m_TexNo].height / 2;
				enemy.speed.y = 0.0f;
			}
		}
		if (IsGround(enemy.m_Pos.x, enemy.m_Pos.y - enemySpritesHWDivData[enemy.m_TexNo].height / 2)) { //floor
			//enemy.m_Rot.z -= D3DXToRadian(6.0f); // Radian : 弧度 = Degree * PI/180.0f
			if (enemy.speed.y < 0.0f)enemy.speed.y = 0.0f;
		}
		if (IsGround(enemy.m_Pos.x - enemySpritesHWDivData[enemy.m_TexNo].width / 2, enemy.m_Pos.y)) { //KABE L
			//enemy.m_Rot.z -= D3DXToRadian(6.0f); // Radian : 弧度 = Degree * PI/180.0f
			if (enemy.speed.x < 0.0f) {
				enemy.turnRight = true;
				enemy.speed.x = 0.0f;
			}
		}
		if (IsGround(enemy.m_Pos.x + enemySpritesHWDivData[enemy.m_TexNo].width / 2, enemy.m_Pos.y)) { //KABE R
			//enemy.m_Rot.z -= D3DXToRadian(6.0f); // Radian : 弧度 = Degree * PI/180.0f
			if (enemy.speed.x > 0.0f) {
				enemy.turnRight = false;
				enemy.speed.x = 0.0f;
			}
		}

	}
	//Update Speed
	enemy.m_Pos.x += enemy.speed.x;
	enemy.m_Pos.y += enemy.speed.y;

	//Rotate
	//enemy.m_Rot.z += D3DXToRadian(6.0f);
	if (enemy.status == A_DEAD) {
		enemy.m_Rot.z += D3DXToRadian(6.0f);
	}
}


void CollisionUpdate(enemyStruct & enemy)
{
	UpdateCollider(enemysCollider2d, D3DXVECTOR2(enemys[0].m_Pos.x, enemys[0].m_Pos.y), 0.0f);
	AddColliderInPool(enemysCollider2d, 1); // updatePool
	SetColliderEnable(enemysCollider2d, enemys[0].m_Use); // disable collision when enemy_m_Use == false
}

void enemyStatusUpdate(enemyStruct & enemy)
{
	enemy.statusPrev = enemy.status;

	if (enemy.status == A_Idle || enemy.status == A_Move || enemy.status == A_InAir) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}
	}

	if (enemy.status == A_DEAD) {
		enemy.m_setUnUseTime -= 1;
		if (enemy.m_setUnUseTime <= 0)enemy.m_Use = false;
	}

	switch (enemy.m_enemyType)
	{
	case Trunk  : TrunkStatusUpdate(enemy);  break;
	case Bunny  : BunnyStatusUpdate(enemy);  break;
	case Slime  : SlimeStatusUpdate(enemy);  break;
	case Turtle :TurtleStatusUpdate(enemy);  break;
	case Bird   :  BirdStatusUpdate(enemy);  break;
	case Flag   :  FlagStatusUpdate(enemy);  break;

	default:
		break;
	}
	
}

void AnimUpdate(enemyStruct & enemy)
{
	//アニメーション (SpritesSheets)
	enemy.m_CountAnim++;
	if ((enemy.m_CountAnim % ANIM_WAIT) == 0)
	{
		// パターンの切り替え
		enemy.animDataNo += 1;

		switch (enemy.m_enemyType)
		{
		case Trunk : TrunkAnimUpdate(enemy) ;  break;
		case Bunny : BunnyAnimUpdate(enemy) ;  break;
		case Slime : SlimeAnimUpdate(enemy) ;  break;
		case Turtle: TurtleAnimUpdate(enemy);  break;
		case Bird  :   BirdAnimUpdate(enemy);  break;
		case Flag  :   FlagAnimUpdate(enemy);  break;

		default:
			break;
		}

		enemy.m_CountAnim = 0;

	}

}

//===============Trunk's StatusUpdate & AnimUpdate
void TrunkStatusUpdate(enemyStruct & enemy)
{
	if (enemy.status == A_Attack && TrunkSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}
	}

	if (enemy.status == A_Hit && TrunkSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}
	
	}
}

void TrunkAnimUpdate(enemyStruct & enemy)
{
	enemy.m_PatternAnim = TrunkSpritesData[enemy.status].Data[enemy.animDataNo];

	if (TrunkSpritesData[enemy.status].Data[enemy.animDataNo] == APT_LOOP) {
		enemy.animDataNo = 0;
		enemy.m_PatternAnim = TrunkSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (TrunkSpritesData[enemy.status].Data[enemy.animDataNo] == APT_STOPF) {
		enemy.animDataNo -= 1;
		enemy.m_PatternAnim = TrunkSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (TrunkSpritesData[enemy.status].Data[enemy.animDataNo] == APT_CWV) {
		enemy.animDataNo = 0;
	}
	else if (TrunkSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		enemy.m_PatternAnim = TrunkSpritesData[enemy.status].Data[enemy.animDataNo - 1]; // -1 in oder to not display the APT_COS(-4) anim to cause error (frame + 1)
	}


	// アニメーション (スピードによって変わる) ex:InAir
	if (enemy.status == A_InAir) {
		int InAirSpritesData = TrunkSpritesData[enemy.status].Data[enemy.animDataNo];
		if (enemy.speed.y < 0.0f)enemy.m_PatternAnim = TrunkSpritesData[enemy.status].Data[0];
		if (enemy.speed.y > 0.0f)enemy.m_PatternAnim = TrunkSpritesData[enemy.status].Data[1];
		if (enemy.speed.y > 5.0f)enemy.m_PatternAnim = TrunkSpritesData[enemy.status].Data[2];
		if (enemy.speed.y > 10.0f)enemy.m_PatternAnim = TrunkSpritesData[enemy.status].Data[3];
	}
}

//===============Bunny's StatusUpdate & AnimUpdate
void BunnyStatusUpdate(enemyStruct & enemy)
{
	if (enemy.status == A_Attack && BunnySpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}
	}

	if (enemy.status == A_Hit && BunnySpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}

	}
}

void BunnyAnimUpdate(enemyStruct & enemy)
{
	enemy.m_PatternAnim = BunnySpritesData[enemy.status].Data[enemy.animDataNo];

	if (BunnySpritesData[enemy.status].Data[enemy.animDataNo] == APT_LOOP) {
		enemy.animDataNo = 0;
		enemy.m_PatternAnim = BunnySpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (BunnySpritesData[enemy.status].Data[enemy.animDataNo] == APT_STOPF) {
		enemy.animDataNo -= 1;
		enemy.m_PatternAnim = BunnySpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (BunnySpritesData[enemy.status].Data[enemy.animDataNo] == APT_CWV) {
		enemy.animDataNo = 0;
	}
	else if (BunnySpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		enemy.m_PatternAnim = BunnySpritesData[enemy.status].Data[enemy.animDataNo - 1]; // -1 in oder to not display the APT_COS(-4) anim to cause error (frame + 1)
	}


	// アニメーション (スピードによって変わる) ex:InAir
	if (enemy.status == A_InAir) {
		int InAirSpritesData = BunnySpritesData[enemy.status].Data[enemy.animDataNo];
		if (enemy.speed.y < 0.0f)enemy.m_PatternAnim = BunnySpritesData[enemy.status].Data[0];
		if (enemy.speed.y > 0.0f)enemy.m_PatternAnim = BunnySpritesData[enemy.status].Data[1];
		if (enemy.speed.y > 5.0f)enemy.m_PatternAnim = BunnySpritesData[enemy.status].Data[2];
		if (enemy.speed.y > 10.0f)enemy.m_PatternAnim = BunnySpritesData[enemy.status].Data[3];
	}
}
//===============Slime's StatusUpdate & AnimUpdate
void SlimeStatusUpdate(enemyStruct & enemy)
{
	if (enemy.status == A_Attack && SlimeSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}
	}

	if (enemy.status == A_Hit && SlimeSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}

	}
}

void SlimeAnimUpdate(enemyStruct & enemy)
{
	enemy.m_PatternAnim = SlimeSpritesData[enemy.status].Data[enemy.animDataNo];

	if (SlimeSpritesData[enemy.status].Data[enemy.animDataNo] == APT_LOOP) {
		enemy.animDataNo = 0;
		enemy.m_PatternAnim = SlimeSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (SlimeSpritesData[enemy.status].Data[enemy.animDataNo] == APT_STOPF) {
		enemy.animDataNo -= 1;
		enemy.m_PatternAnim = SlimeSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (SlimeSpritesData[enemy.status].Data[enemy.animDataNo] == APT_CWV) {
		enemy.animDataNo = 0;
	}
	else if (SlimeSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		enemy.m_PatternAnim = SlimeSpritesData[enemy.status].Data[enemy.animDataNo - 1]; // -1 in oder to not display the APT_COS(-4) anim to cause error (frame + 1)
	}


	// アニメーション (スピードによって変わる) ex:InAir
	if (enemy.status == A_InAir) {
		int InAirSpritesData = SlimeSpritesData[enemy.status].Data[enemy.animDataNo];
		if (enemy.speed.y < 0.0f)enemy.m_PatternAnim = SlimeSpritesData[enemy.status].Data[0];
		if (enemy.speed.y > 0.0f)enemy.m_PatternAnim = SlimeSpritesData[enemy.status].Data[1];
		if (enemy.speed.y > 5.0f)enemy.m_PatternAnim = SlimeSpritesData[enemy.status].Data[2];
		if (enemy.speed.y > 10.0f)enemy.m_PatternAnim = SlimeSpritesData[enemy.status].Data[3];
	}

}

//===============Turtle's StatusUpdate & AnimUpdate
void TurtleStatusUpdate(enemyStruct & enemy)
{
	if (enemy.status == A_Attack && TurtleSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}
	}

	if (enemy.status == A_Hit && TurtleSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}

	}
}

void TurtleAnimUpdate(enemyStruct & enemy)
{
	enemy.m_PatternAnim = TurtleSpritesData[enemy.status].Data[enemy.animDataNo];

	if (TurtleSpritesData[enemy.status].Data[enemy.animDataNo] == APT_LOOP) {
		enemy.animDataNo = 0;
		enemy.m_PatternAnim = TurtleSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (TurtleSpritesData[enemy.status].Data[enemy.animDataNo] == APT_STOPF) {
		enemy.animDataNo -= 1;
		enemy.m_PatternAnim = TurtleSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (TurtleSpritesData[enemy.status].Data[enemy.animDataNo] == APT_CWV) {
		enemy.animDataNo = 0;
	}
	else if (TurtleSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		enemy.m_PatternAnim = TurtleSpritesData[enemy.status].Data[enemy.animDataNo - 1]; // -1 in oder to not display the APT_COS(-4) anim to cause error (frame + 1)
	}


	// アニメーション (スピードによって変わる) ex:InAir
	if (enemy.status == A_InAir) {
		int InAirSpritesData = TurtleSpritesData[enemy.status].Data[enemy.animDataNo];
		if (enemy.speed.y < 0.0f)enemy.m_PatternAnim = TurtleSpritesData[enemy.status].Data[0];
		if (enemy.speed.y > 0.0f)enemy.m_PatternAnim = TurtleSpritesData[enemy.status].Data[1];
		if (enemy.speed.y > 5.0f)enemy.m_PatternAnim = TurtleSpritesData[enemy.status].Data[2];
		if (enemy.speed.y > 10.0f)enemy.m_PatternAnim = TurtleSpritesData[enemy.status].Data[3];
	}

}

//===============Bird's StatusUpdate & AnimUpdate
void BirdStatusUpdate(enemyStruct & enemy)
{
	if (enemy.status == A_Attack && BirdSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}
	}

	if (enemy.status == A_Hit && BirdSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}

	}
}

void BirdAnimUpdate(enemyStruct & enemy)
{
	enemy.m_PatternAnim = BirdSpritesData[enemy.status].Data[enemy.animDataNo];

	if (BirdSpritesData[enemy.status].Data[enemy.animDataNo] == APT_LOOP) {
		enemy.animDataNo = 0;
		enemy.m_PatternAnim = BirdSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (BirdSpritesData[enemy.status].Data[enemy.animDataNo] == APT_STOPF) {
		enemy.animDataNo -= 1;
		enemy.m_PatternAnim = BirdSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (BirdSpritesData[enemy.status].Data[enemy.animDataNo] == APT_CWV) {
		enemy.animDataNo = 0;
	}
	else if (BirdSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		enemy.m_PatternAnim = BirdSpritesData[enemy.status].Data[enemy.animDataNo - 1]; // -1 in oder to not display the APT_COS(-4) anim to cause error (frame + 1)
	}

}

//===============Flag's StatusUpdate & AnimUpdate
void FlagStatusUpdate(enemyStruct & enemy)
{
	if (enemy.status == A_Attack && FlagSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		if (fabsf(enemy.speed.x) > 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Move);
		}
		if (fabsf(enemy.speed.x) <= 0.0f && fabsf(enemy.speed.y) == 0.0f) {
			ChangeStatus(&enemy, A_Idle);
		}

		if (fabsf(enemy.speed.y) > 0.0f) {
			ChangeStatus(&enemy, A_InAir);
		}
	}
}

void FlagAnimUpdate(enemyStruct & enemy)
{
	enemy.m_PatternAnim = FlagSpritesData[enemy.status].Data[enemy.animDataNo];

	if (FlagSpritesData[enemy.status].Data[enemy.animDataNo] == APT_LOOP) {
		enemy.animDataNo = 0;
		enemy.m_PatternAnim = FlagSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (FlagSpritesData[enemy.status].Data[enemy.animDataNo] == APT_STOPF) {
		enemy.animDataNo -= 1;
		enemy.m_PatternAnim = FlagSpritesData[enemy.status].Data[enemy.animDataNo];
	}
	else if (FlagSpritesData[enemy.status].Data[enemy.animDataNo] == APT_CWV) {
		enemy.animDataNo = 0;
	}
	else if (FlagSpritesData[enemy.status].Data[enemy.animDataNo] == APT_COS) {
		enemy.m_PatternAnim = FlagSpritesData[enemy.status].Data[enemy.animDataNo - 1]; // -1 in oder to not display the APT_COS(-4) anim to cause error (frame + 1)
	}


	// アニメーション (スピードによって変わる) ex:InAir
	if (enemy.status == A_InAir) {
		int InAirSpritesData = FlagSpritesData[enemy.status].Data[enemy.animDataNo];
		if (enemy.speed.y < 0.0f)enemy.m_PatternAnim = FlagSpritesData[enemy.status].Data[0];
		if (enemy.speed.y > 0.0f)enemy.m_PatternAnim = FlagSpritesData[enemy.status].Data[1];
		if (enemy.speed.y > 5.0f)enemy.m_PatternAnim = FlagSpritesData[enemy.status].Data[2];
		if (enemy.speed.y > 10.0f)enemy.m_PatternAnim = FlagSpritesData[enemy.status].Data[3];
	}
}

//==============================other
void getHit(enemyStruct &enemy)
{
	ChangeStatus(& enemy , A_Hit);
	if (HitSETimer == 0) {
		HitSETimer = HIT_SE_TIME;
		PlaySound(SOUND_LABEL_SE_hit2);
	}
}

void enemyGetElimated(enemyStruct &enemy)
{
	ChangeStatus(&enemy, A_DEAD);
	enemy.m_isElimated = true;
	PlaySound(SOUND_LABEL_SE_hit);
}

void setEnemyReborn(enemyStruct & enemy)
{
	enemy.m_Use = true;
	if (enemy.m_enemyType == Slime)enemy.reborntime = SLIME_REBORN_TIME;
	else if (enemy.m_enemyType == Turtle) enemy.reborntime = TURTLE_REBORN_TIME;
	else enemy.reborntime = MONSTER_REBORN_TIME;
	enemy.m_setUnUseTime = ENEMY_SETUNUSEDTIME;
	enemy.m_isElimated = false;
	enemy.m_Pos = enemy.rebornPos;
	enemy.m_Rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	enemy.LMtime = 0.0f;
	ChangeStatus(&enemy, A_Hit);
}

void linearMove() {
	//level 2
	if (enemys[1].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[1].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl2) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl2[nextNo] - g_MoveTbl2[nowNo];
		float nowTime = enemys[1].LMtime - nowNo;
		pos *= nowTime;
		enemys[1].m_Pos = g_MoveTbl2[nowNo] + pos;
		enemys[1].LMtime += g_MoveSpd2[nowNo];
		if ((int)enemys[1].LMtime >= maxNo) {
			enemys[1].LMtime = 0.0f;
		}
		if (nowNo == 0) enemys[1].turnRight = true;
		if (nowNo == 1) enemys[1].turnRight = false;
	}

	//level 4
	if (enemys[3].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[3].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl3) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl3[nextNo] - g_MoveTbl3[nowNo];
		float nowTime = enemys[3].LMtime - nowNo;
		pos *= nowTime;
		enemys[3].m_Pos = g_MoveTbl3[nowNo] + pos;
		enemys[3].LMtime += g_MoveSpd3[nowNo];
		if ((int)enemys[3].LMtime >= maxNo) {
			enemys[3].LMtime = 0.0f;
		}
		if (nowNo == 0) enemys[3].turnRight = true;
		if (nowNo == 1) enemys[3].turnRight = false;
	}

	//level 5
	if (enemys[5].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[5].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl4) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl4[nextNo] - g_MoveTbl4[nowNo];
		float nowTime = enemys[5].LMtime - nowNo;
		pos *= nowTime;
		enemys[5].m_Pos = g_MoveTbl4[nowNo] + pos;
		enemys[5].LMtime += g_MoveSpd4[nowNo];
		if ((int)enemys[5].LMtime >= maxNo) {
			enemys[5].LMtime = 0.0f;
		}
		if (nowNo == 0) enemys[5].turnRight = false;
		if (nowNo == 1) enemys[5].turnRight = true;
	}

	//level 6
	if (enemys[6].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[6].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl5) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl5[nextNo] - g_MoveTbl5[nowNo];
		float nowTime = enemys[6].LMtime - nowNo;
		pos *= nowTime;
		enemys[6].m_Pos = g_MoveTbl5[nowNo] + pos;
		enemys[6].LMtime += g_MoveSpd5[nowNo];
		if ((int)enemys[6].LMtime >= maxNo) {
			enemys[6].LMtime = 0.0f;
		}
	}
	if (enemys[7].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[7].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl6) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl6[nextNo] - g_MoveTbl6[nowNo];
		float nowTime = enemys[7].LMtime - nowNo;
		pos *= nowTime;
		enemys[7].m_Pos = g_MoveTbl6[nowNo] + pos;
		enemys[7].LMtime += g_MoveSpd6[nowNo];
		if ((int)enemys[7].LMtime >= maxNo) {
			enemys[7].LMtime = 0.0f;
		}
	}
	if (enemys[8].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[8].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl7) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl7[nextNo] - g_MoveTbl7[nowNo];
		float nowTime = enemys[8].LMtime - nowNo;
		pos *= nowTime;
		enemys[8].m_Pos = g_MoveTbl7[nowNo] + pos;
		enemys[8].LMtime += g_MoveSpd7[nowNo];
		if ((int)enemys[8].LMtime >= maxNo) {
			enemys[8].LMtime = 0.0f;
		}
	}
	if (enemys[9].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[9].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl8) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl8[nextNo] - g_MoveTbl8[nowNo];
		float nowTime = enemys[9].LMtime - nowNo;
		pos *= nowTime;
		enemys[9].m_Pos = g_MoveTbl8[nowNo] + pos;
		enemys[9].LMtime += g_MoveSpd8[nowNo];
		if ((int)enemys[9].LMtime >= maxNo) {
			enemys[9].LMtime = 0.0f;
		}
	}

	//Level 7
	if (enemys[11].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[11].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl9) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl9[nextNo] - g_MoveTbl9[nowNo];
		float nowTime = enemys[11].LMtime - nowNo;
		pos *= nowTime;
		enemys[11].m_Pos = g_MoveTbl9[nowNo] + pos;
		enemys[11].LMtime += g_MoveSpd9[nowNo];
		if ((int)enemys[11].LMtime >= maxNo) {
			enemys[11].LMtime = 0.0f;
		}
	}

	//Level 8
	if (enemys[14].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[14].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl10) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl10[nextNo] - g_MoveTbl10[nowNo];
		float nowTime = enemys[14].LMtime - nowNo;
		pos *= nowTime;
		enemys[14].m_Pos = g_MoveTbl10[nowNo] + pos;
		enemys[14].LMtime += g_MoveSpd10[nowNo];
		if ((int)enemys[14].LMtime >= maxNo) {
			enemys[14].LMtime = 0.0f;
		}
	}
	if (enemys[15].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[15].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl11) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl11[nextNo] - g_MoveTbl11[nowNo];
		float nowTime = enemys[15].LMtime - nowNo;
		pos *= nowTime;
		enemys[15].m_Pos = g_MoveTbl11[nowNo] + pos;
		enemys[15].LMtime += g_MoveSpd11[nowNo];
		if ((int)enemys[15].LMtime >= maxNo) {
			enemys[15].LMtime = 0.0f;
		}
		if (nowNo == 0) enemys[15].turnRight = true;
		if (nowNo == 1) enemys[15].turnRight = false;
	}
	// Level 9
	if (enemys[21].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[21].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl12) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl12[nextNo] - g_MoveTbl12[nowNo];
		float nowTime = enemys[21].LMtime - nowNo;
		pos *= nowTime;
		enemys[21].m_Pos = g_MoveTbl12[nowNo] + pos;
		enemys[21].LMtime += g_MoveSpd12[nowNo];
		if ((int)enemys[21].LMtime >= maxNo) {
			enemys[21].LMtime = 0.0f;
		}
		if (nowNo == 0) enemys[21].turnRight = true;
		if (nowNo == 1) enemys[21].turnRight = false;
	}

	// Level 11
	if (enemys[23].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[23].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl13) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl13[nextNo] - g_MoveTbl13[nowNo];
		float nowTime = enemys[23].LMtime - nowNo;
		pos *= nowTime;
		enemys[23].m_Pos = g_MoveTbl13[nowNo] + pos;
		enemys[23].LMtime += g_MoveSpd13[nowNo];
		if ((int)enemys[23].LMtime >= maxNo) {
			enemys[23].LMtime = 0.0f;
		}
		if (nowNo == 0) enemys[23].turnRight = true;
		if (nowNo == 1) enemys[23].turnRight = false;
	}
	if (enemys[24].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[24].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl14) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl14[nextNo] - g_MoveTbl14[nowNo];
		float nowTime = enemys[24].LMtime - nowNo;
		pos *= nowTime;
		enemys[24].m_Pos = g_MoveTbl14[nowNo] + pos;
		enemys[24].LMtime += g_MoveSpd14[nowNo];
		if ((int)enemys[24].LMtime >= maxNo) {
			enemys[24].LMtime = 0.0f;
		}
		if (nowNo == 0) enemys[24].turnRight = false;
		if (nowNo == 1) enemys[24].turnRight = true;
	}
	if (enemys[25].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[25].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl15) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl15[nextNo] - g_MoveTbl15[nowNo];
		float nowTime = enemys[25].LMtime - nowNo;
		pos *= nowTime;
		enemys[25].m_Pos = g_MoveTbl15[nowNo] + pos;
		enemys[25].LMtime += g_MoveSpd15[nowNo];
		if ((int)enemys[25].LMtime >= maxNo) {
			enemys[25].LMtime = 0.0f;
		}
		if (nowNo == 0) enemys[25].turnRight = true;
		if (nowNo == 1) enemys[25].turnRight = false;
	}
	if (enemys[26].status != A_DEAD) {
		//=線形移動 // Physics Update 影響がない
		int nowNo = (int)enemys[26].LMtime; //Liner move time
		int maxNo = (sizeof(g_MoveTbl16) / sizeof(D3DXVECTOR3));
		int nextNo = (nowNo + 1) % maxNo;
		D3DXVECTOR3 pos = g_MoveTbl16[nextNo] - g_MoveTbl16[nowNo];
		float nowTime = enemys[26].LMtime - nowNo;
		pos *= nowTime;
		enemys[26].m_Pos = g_MoveTbl16[nowNo] + pos;
		enemys[26].LMtime += g_MoveSpd16[nowNo];
		if ((int)enemys[26].LMtime >= maxNo) {
			enemys[26].LMtime = 0.0f;
		}
		if (nowNo == 0) enemys[26].turnRight = false;
		if (nowNo == 1) enemys[26].turnRight = true;
	}
}

