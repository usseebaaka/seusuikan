/*
 * マルチレゾリューションに対応させるためのマクロ
 * (各画像サイズと対応画面サイズを定義)
 */
#ifndef __APPMACROS_H__
#define __APPMACROS_H__

#include "cocos2d.h"

typedef struct tagResource {
    cocos2d::CCSize size;
    char directory[100];
} Resource;

static Resource smallResource  = { cocos2d::CCSizeMake( 512,  384), "S"};	// Sサイズ用の画像の画面サイズを定義
static Resource mediumResource = { cocos2d::CCSizeMake(1024,  768), "M"};	// Mサイズ用の画像の画面サイズを定義
static Resource largeResource  = { cocos2d::CCSizeMake(2048, 1536), "L"};	// Lサイズ用の画像の画面サイズを定義

static cocos2d::CCSize designResolutionSize = cocos2d::CCSizeMake(1024, 768);

#define NUMBER_FONT_SIZE (cocos2d::CCEGLView::sharedOpenGLView()->getDesignResolutionSize().width / mediumResource.size.width * 48)

#endif // __APPMACROS_H__
