#include "Animation.h"		//Animation.hをインクルード

/* Animationクラスのnum3Animation関数の定義
 * 返却型 : CCFiniteTimeAciton型ポインタ
 * 仮引数 : なし
 */
CCFiniteTimeAction* Animation::num3Animation() {
	CCDelayTime* delay1 = CCDelayTime::create(0.5);		//0.5秒ディレイ
	CCScaleTo* scaleUp = CCScaleTo::create(0.1, 1);		//0.1秒で対象物を1倍の大きさで表示
	CCDelayTime* delay2 = CCDelayTime::create(0.3);		//0.3秒ディレイ
	CCScaleTo* scaleDown = CCScaleTo::create(0.1, 0);	//0.1秒で0倍の大きさに

	return CCSequence::create(delay1, scaleUp, delay2, scaleDown, NULL);	//全ての変数と最後にNULLを引数に持たせcreate関数を返却
}

/* Animationクラスのnum2Animation関数の定義
 * 返却型 : CCFiniteTimeAciton型ポインタ
 * 仮引数 : なし
 */
CCFiniteTimeAction* Animation::num2Animation() {
	CCDelayTime* delay = CCDelayTime::create(1);		//1秒ディレイ

	return CCSequence::create(delay, Animation::num3Animation(), NULL);		//delay num3Animation NULLを引数にcreate関数を返却
}

/* Animationクラスのnum2Animation関数の定義
 * 返却型 : CCFiniteTimeAciton型ポインタ
 * 仮引数 : なし
 */
CCFiniteTimeAction* Animation::num1Animation() {
	CCDelayTime* delay = CCDelayTime::create(2);		//2秒ディレイ

	return CCSequence::create(delay, Animation::num3Animation(), NULL);		//delay num3Animation NULLを引数にcreate関数を返却
}

/* Animationクラスのnum2Animation関数の定義
 * 返却型 : CCFiniteTimeAciton型ポインタ
 * 仮引数 : CCObject型ポインタtarget, SEL_CallFunc型変数selector
 */
CCFiniteTimeAction* Animation::gameStartAnimation(CCObject* target, SEL_CallFunc selector) {
	CCDelayTime* delay = CCDelayTime::create(3);				//3秒ディレイ
	CCCallFunc* func = CCCallFunc::create(target, selector);	//targetとseletorと引数に持たせたcreate関数をCCCallFunc型変数funcに代入

	return CCSequence::create(delay, Animation::num3Animation(), func, NULL);	//delay num3Animation()関数 func NULLをを持たせたcreate関数を返却
}

/* AnimatioinクラスhitAnimation関数定義
 * 返却型 : CCFiniteTimeAction
 * 仮引数 : int型変数count
 */
CCFiniteTimeAction* Animation::hitAnimation(int count) {
	CCArray *animFrames = CCArray::create();	//CCArray型配列を初期化
	CCSpriteFrameCache *cache = CCSpriteFrameCache::sharedSpriteFrameCache();	//キャッシュより指定したファイル名のインスタンスを取得しCCSpriteFrame型のオブジェクトframeに代入
	/* 初期値0のint型変数iが
	 * count未満の間
	 * インクリメント
	 */
	for(int i=0; i<count; i++) {
		CCString* fileName =
				CCString::createWithFormat(count == 13 ? "hit%d.png" : "defeat%d.png", i);	//countが13と同値であればhit%d.pngの、違う場合はdefeat%d.pngのファイルネームを取得しfileNameに代入
		CCSpriteFrame *frame = cache->spriteFrameByName(fileName->getCString());	//キャッシュからインスタンスを取得

		//インスタンスが取得できない場合(初回読み込み時まだインスタンスが生成されていない為)
		if(!frame) {
			CCTexture2D *texture = CCTextureCache::sharedTextureCache()->addImage(fileName->getCString());	//fileNameに一致するファイルの取得
			CCRect bounds = CCRectMake(0, 0, texture->getContentSize().width, texture->getContentSize().height);	//インスタンスの座標とサイズを取得
			frame = CCSpriteFrame::createWithTexture(texture, bounds);	//テクスチャをframeに代入
		}
		animFrames->addObject(frame);	//インスタンスを配列に追加
	}
	CCAnimation* animation = CCAnimation::createWithSpriteFrames(animFrames, 0.1);	//CCSpriteFrameCacheクラスの配列を引数にCCAnimationクラスのインスタンスの生成 第二引数は1フレームの時間

	return CCAnimate::create(animation);	//CCAnimateクラスのアクションを生成して返却
}

/* screenChangeAnimation関数
 * 返却型 : CCFiniteTimeAction型ポインタ
 * 仮引数 : CCObject型ポインタtarget, SEL_CallFunc型変数selector
 */
CCFiniteTimeAction* screenChangeAnimation(CCObject* target, SEL_CallFunc selector) {
	CCScaleTo* scaleUp = CCScaleTo::create(0.5, 1);			//0.5秒で対象物1倍に
	CCDelayTime* delay = CCDelayTime::create(2.5);			//そのまま2.5秒待機
	CCCallFunc* func = CCCallFunc::create(target, selector);	//呼び出す関数をfuncに代入

	return CCSequence::create(scaleUp, delay, func, NULL);	//scaleUp delay func NULLを引数に
}

// ゲームオーバー表示
CCFiniteTimeAction* Animation::gameOverAnimation(CCObject* target, SEL_CallFunc selector) {
    CCScaleTo* scaleUp = CCScaleTo::create(0.5, 1);				// 0.5秒でスケールを等倍にする
    CCDelayTime* delay = CCDelayTime::create(4);				// 4秒間待機
    CCCallFunc* func = CCCallFunc::create(target, selector);

    return CCSequence::create(scaleUp, delay, func, NULL);
}
