//インクルードガード
#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "cocos2d.h"	//cocosd.hをインクルード

USING_NS_CC;			//名前空間CCの使用

/*----- Animationクラスの宣言 -----*/
class Animation {
public:

    /*----- カウントダウン -----*/
    static CCFiniteTimeAction* num3Animation();	//｢3｣の表示
    static CCFiniteTimeAction* num2Animation();	//｢2｣の表示
    static CCFiniteTimeAction* num1Animation();	//｢1｣の表示
    static CCFiniteTimeAction* gameStartAnimation(CCObject* target, SEL_CallFunc selector);		//[start]の表示後、次の処理に移行するため引数を持たす
    // ゲームオーバー表示
    static CCFiniteTimeAction* gameOverAnimation(CCObject* target, SEL_CallFunc selector);
    static CCFiniteTimeAction* screenChangeAnimation(CCObject* target, SEL_CallFunc selector);	//ゲームクリアの表示後、次の処理に移行するため引数を持たすの処理に以降するため引数を持たす
    static CCFiniteTimeAction* hitAnimation(int count);		//ミサイル爆発のアニメーション(被弾時or撃墜時)
    static CCFiniteTimeAction* scrollSeaBed();				//海底スクロールのアニメーション
};

#endif // __ANIMATION_H__	//インクルードガード終了
