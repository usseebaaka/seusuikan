#ifndef __GAMEPHYSICS_CONTACT_LISTENER_H__
#define __GAMEPHYSICS_CONTACT_LISTENER_H__

#include "cocos2d.h"
#include "box2d/Box2D.h"

USING_NS_CC;

class GamePhysicsContactListener : public b2ContactListener {
protected:
    CCObject* m_target;
public:
    GamePhysicsContactListener(CCObject* target);		// よくわからんコンストラクタ
    void BeginContact(b2Contact* contact);				// 物理構造を持つ2つのオブジェクトが衝突した時に呼ばれる関数
};

#endif // __GAMEPHYSICS_CONTACT_LISTENER_H__
