﻿//=============================================================================
//
// フェード処理 [fade.h]
// Author : GP11B132 14 ゴショウケン
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************

// フェードの状態
typedef enum
{
	FADE_NONE = 0,		// 何もない状態
	FADE_IN,			// フェードイン処理
	FADE_OUT,			// フェードアウト処理
	FADE_MAX
} FADE;



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitFade(void);
void UninitFade(void);
void UpdateFade(void);
void DrawFade(void);

void SetFade(FADE fade, MODE modeNext);
FADE GetFade(void);



