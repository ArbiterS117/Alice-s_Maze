//=============================================================================
//
// ゲーム画面処理 [game.h]
// Author : GP11B132 14 ゴショウケン
//
#pragma once

#include "main.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitGame(void);
void UninitGame(void);
void UpdateGame(void);
void DrawGame(void);

void EffectStopFrame(int frame); // number of stop frames
bool IsStoppingFrame();

int  GetGameTimeFrame();
int  GetGameTimeSec();
int  GetGameTimeMin();
int  GetDeathCount();
void AddDeathCount(int i);