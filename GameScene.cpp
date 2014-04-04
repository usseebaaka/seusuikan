#include "GameScene.h"
#include "config.h"
#include "AppMacros.h"
#include "Animation.h"

USING_NS_CC;

// コンストラクタ
GameScene::GameScene()
:enemyUnit_num(2),
 player_VIT(2),
 submarine_VIT(202),
 destroyer_VIT(202),
 score_and_Maxplace(0.3),
 buttons_sum(11),
 lifepoint(3){
	scoreText = new CCArray();
	srand((unsigned int)time(NULL));
}

CCScene* GameScene::scene() {
	CCScene* scene = CCScene::create();			// 背景の雛型となるオブジェクトを生成
	GameScene* layer = GameScene::create();		// 背景に処理を重ねるためのオブジェクトを生成
	scene->addChild(layer);						// 雛型にレイヤーを埋め込む

	return scene;								// 作成した雛型を返却
}

// ゲーム内の部品を生成する関数
bool GameScene::init() {
	if (!CCLayer::init()) {
		return false;														// シーンオブジェクトの生成に失敗したらfalseを返す
	}
	this->setTouchEnabled(true);				  							// タッチ可能にする
	this->setTouchMode(kCCTouchesAllAtOnce);								// マルチタップイベントを受け付ける

	sleep(40);
	b2Vec2 gravity;										    				// 重力の設定値を格納するための変数
	gravity.Set(0.0, -10.0);												// 重力を設定

	world = new b2World(gravity);											// 重力を持った世界を生成

	mGamePhysicsContactListener = new GamePhysicsContactListener(this);		// 衝突判定結果を格納するための変数を用意
	world->SetContactListener(mGamePhysicsContactListener);					// 衝突判定処理を追加

	createBackground();														// 背景および海底を生成
	// 自機を生成
	createUnit(player_VIT, kTag_PlayerUnit, submarine_VIT, playerUnit);
	// 敵駆逐艦を生成
	createUnit(destroyer_VIT % 100, kTag_EnemyDestroyer, destroyer_VIT, enemyDestroyer);
	destroyer_VIT /= 100;													// 100で割って次のデータへ

	createScore();															// スコアラベルを生成
	createLifeCounter();													// 残機ラベルを生成
	createControllerPanel();												// 操作部を生成

	// ミサイルの準備
	missileBatchNode = CCSpriteBatchNode::create("missile.png");
	this->addChild(missileBatchNode);

	// カウントダウン開始
	this->scheduleOnce(schedule_selector(GameScene::showCountdown), 1);
	return true;
}

// 背景および海底を生成
void GameScene::createBackground() {
	CCSprite* pBgUnder = CCSprite::create("game_bg.png");					// 背景画像を取得し、オブジェクトに格納
	pBgUnder->setPosition(ccp(getWindowSize().width * 0.5, getWindowSize().height * 0.5));	// 背景画像の中心を画面の中心にセット
	this->addChild(pBgUnder, kZOrder_Background, kTag_Background);			// 背景画像をシーンに登録

	CCNode* pSeabed = CCNode::create();										// 海底を実体を持つオブジェクトとして生成
	pSeabed->setTag(kTag_Seabed);											// 海底をタグと関連付ける
	this->addChild(pSeabed);												// 海底をシーンに登録

	b2BodyDef seabedBodyDef;												// 物理構造を持つ海底構造体を生成
	float seabedBottom = pBgUnder->getContentSize().height / 3;				// 海底とコントローラ部の境界座標を設定
	seabedBodyDef.position.Set(0, seabedBottom);							// 海底の位置をセット
	seabedBodyDef.userData = pSeabed;										// 海底オブジェクトのデータを構造体に登録

	b2Body* seabedBody = world->CreateBody(&seabedBodyDef);					// 重力世界に海底構造体を登録

	float seabedHeight = pBgUnder->getContentSize().height / 10;			// 海底構造体の高さを設定

	// コントローラ部とプレイ部を分けるための変数
	b2EdgeShape seabedBox;
	// 2つの部位の境界線を設定
	seabedBox.Set(b2Vec2(0, (seabedBottom + seabedHeight) / PTM_RATIO),
			b2Vec2(getWindowSize().width / PTM_RATIO, (seabedBottom + seabedHeight) / PTM_RATIO));
	// 指定した境界線と密度を海底オブジェクトに登録
	seabedBody->CreateFixture(&seabedBox, 0);

	CCNode* pBorderline = CCNode::create();									// 境界線を実体を持つオブジェクトとして生成
	pBorderline->setTag(kTag_Borderline);									// 境界線をタグと関連付ける
	this->addChild(pBorderline);											// 境界線をシーンに登録

	b2BodyDef borderlineBodyDef;											// 物理構造を持つ境界線構造体を生成
	borderlineBodyDef.position.Set(0, seabedBottom);						// 境界線の位置をセット
	borderlineBodyDef.userData = pBorderline;								// 境界線オブジェクトのデータを構造体に登録

	b2Body* borderlineBody = world->CreateBody(&borderlineBodyDef);			// 重力世界に境界線構造体を登録

	// 敵の移動範囲を制限するための変数
	b2EdgeShape borderlineBox;
	// 画面の左端より奥に行けなくなるように設定
	borderlineBox.Set(b2Vec2(0, seabedBottom / PTM_RATIO),
			b2Vec2( 0, getWindowSize().height));
	// 指定した境界線と密度を海底オブジェクトに登録
	borderlineBody->CreateFixture(&borderlineBox, 0);
	// 画面の右端より奥に行けなくなるように設定
	borderlineBox.Set(b2Vec2(getWindowSize().width, seabedBottom / PTM_RATIO),
			b2Vec2( getWindowSize().width, getWindowSize().height));
	// 指定した境界線と密度を海底オブジェクトに登録
	borderlineBody->CreateFixture(&borderlineBox, 0);
}
// ユニットを生成する
void GameScene::createUnit(int hp, int kTag, int vit, b2Body* body) {
	CCSize winSize = getWindowSize();												// ウィンドウサイズを取得
	CCSize bgSize = this->getChildByTag(kTag_Background)->getContentSize();			// 背景サイズを取得
	PhysicsSprite* pUnit = new PhysicsSprite(hp);										// 物理構造を持った画像オブジェクトを生成
	pUnit->autorelease();															// メモリ領域を開放
	if (vit == submarine_VIT) {														// 潜水艦の場合
		pUnit->initWithFile("submarine.png");										// 潜水艦画像を取得し、オブジェクトに格納
		if (kTag == kTag_PlayerUnit) {												// プレイヤーユニットの場合
			pUnit->setPosition(ccp(bgSize.width * 0.5,								// 任意の位置にオブジェクトをセット
					bgSize.height * 0.35 - (bgSize.height - getWindowSize().height) / 2));
		} else {																	// プレイヤーの潜水艦でない場合
			pUnit->setPosition(ccp(bgSize.width * 0.5,								// 任意の位置にオブジェクトをセット
					bgSize.height * 0.35 - (bgSize.height - getWindowSize().height) / 2));
		}
	} else {																		// 潜水艦でない場合
		pUnit->initWithFile("destroyer.png");										// 駆逐艦画像を取得し、オブジェクトに格納
		pUnit->setPosition(ccp(bgSize.width * 0.5,									// 任意の位置にオブジェクトをセット
				bgSize.height * 0.35 - (bgSize.height - getWindowSize().height) / 2));
	}
	pUnit = createPhysicsBody(kTag_DynamicBody, pUnit, body, kTag_Polygon);		// オブジェクトに物理構造を持たせる
	this->addChild(pUnit, kZOrder_Unit, kTag);										// タグとオブジェクトを関連づける
}
// 物理構造を持ったユニットノードを作成
PhysicsSprite* GameScene::createPhysicsBody(int kTag, PhysicsSprite* pNode, b2Body* body, int shape) {
	b2BodyDef physicsBodyDef;														// 物理構造を持ったユニット変数
	if (kTag == kTag_StaticBody) {													// 静的構造が選択された場合
		physicsBodyDef.type = b2_staticBody;										// オブジェクトは静的になる
	} else if (kTag == kTag_DynamicBody) {											// 動的構造が選択された場合
		physicsBodyDef.type = b2_dynamicBody;										// オブジェクトは動的になる
	} else {																		// どちらでもない場合
		physicsBodyDef.type = b2_kinematicBody;										// オブジェクトは運動学的になる
	}

	physicsBodyDef.position.Set(pNode->getPositionX() / PTM_RATIO,					// 物理オブジェクトの位置をセット
			pNode->getPositionY() / PTM_RATIO);
	physicsBodyDef.userData = pNode;												// ノードをデータに登録
	body = world->CreateBody(&physicsBodyDef);										// 物理世界に登録
	b2FixtureDef physicsFixturedef;													// 物理形状を決める変数
	if (!shape) {																	// shapeが0の場合
		b2PolygonShape physicsShape;												// 形状を角形に
		physicsShape.SetAsBox(pNode->getContentSize().width * 0.8 / 2 / PTM_RATIO,	// 角形の範囲を設定
				1 / PTM_RATIO);
		physicsFixturedef.shape = &physicsShape;									// 設定した形状を登録
	} else {																		// shapeが0でない場合
		b2CircleShape physicsShape;													// 形状を円形
		physicsShape.m_radius = pNode->getContentSize().width * 0.4 / PTM_RATIO;	// 円形の範囲を設定
		physicsFixturedef.shape = &physicsShape;									// 設定した形状を登録
	}

	physicsFixturedef.density = 1;													// オブジェクトの密度を設定
	physicsFixturedef.friction = 0.3;												// オブジェクトの摩擦を設定

	body->CreateFixture(&physicsFixturedef);										// 構造体に情報を登録
	pNode->setPhysicsBody(body);													// ノードに構造体を登録
	return pNode;																	// 作成したノードを返却
}
// スコア部を生成
void GameScene::createScore() {
	float scale = CCEGLView::sharedOpenGLView()->getScaleY();						// シーンの倍率を計算
	float offset = (getWindowSize().height * scale - getViewSize().height) / 2 / scale;			// 画面に対する配置が行えるように倍率から逆算

	CCNode* pScore = CCNode::create();												// スコアノードの生成
	pScore->setPosition(ccp(getWindowSize().width - pScore->getContentSize().width / 2,		// スコアノードの位置を設定
			getWindowSize().width - pScore->getContentSize().width / 2));
	this->addChild(pScore, kZOrder_Label, kTag_Score);								// タグとノードを関連づけ

	// スコア部分の生成
	scoreText = CCArray::create();													// スコアを入れるための配列を生成
	// 任意の桁数だけ繰り返し
	for (int i = score_and_Maxplace * 10; i % 10; i--) {
		CCLabelTTF* tmp;															// 配列に格納するオブジェクトを宣言
		i * 10 != 1 ?												// 配列の最後の要素であるか？
				tmp = CCLabelTTF::create("", "", NUMBER_FONT_SIZE) :					// であれば数字表示なし
				tmp = CCLabelTTF::create("0", "", NUMBER_FONT_SIZE);					// でなければ数字表示あり
		tmp->setPosition(ccp(1, 3));												// 座標をセット
		tmp->setColor(ccBLACK);														// フォントカラーをセット
		scoreText->addObject(tmp);													// 配列の末尾にオブジェクトをセット
		pScore->addChild((CCLabelTTF*)scoreText->lastObject());						// スコアノードに情報を登録
	}
}
// 残機カウンターを生成
void GameScene::createLifeCounter() {
	// 画像ファイルをバッチノード化
	CCSpriteBatchNode* pLifeCounterBatchNode = CCSpriteBatchNode::CCSpriteBatchNode::create("lifeCounter.png");
	this->addChild(pLifeCounterBatchNode, kZOrder_Label, kTag_LifeCounter);			// タグとノードを関連づけ
	// ライフの数だけ繰り返し
	for (int i = 0; i < lifepoint; i++) {
		// バッチノードから画像を取得してオブジェクト化
		CCSprite* pLifeCounter = CCSprite::createWithTexture(pLifeCounterBatchNode->getTexture());
		// 任意の位置に画像をセット
		pLifeCounter->setPosition(ccp(getWindowSize().width - pLifeCounter->getContentSize().width / 2 - 0.225 * i,		// スコアノードの位置を設定
				getWindowSize().width - pLifeCounter->getContentSize().width / 2));
		pLifeCounterBatchNode ->addChild(pLifeCounter);		// オブジェクト情報をバッチノードにセット
	}
}
// 操作部を生成
void GameScene::createControllerPanel() {

}
// ゲーム開始時のカウントダウン
void GameScene::showCountdown() {

	// 「3」の表示
	CCSprite* pNum3 = CCSprite::create("count3.png");
	pNum3->setPosition(ccp(getWindowSize().width * 0.5, getWindowSize().height * 0.5));
	pNum3->setScale(0);
	pNum3->runAction(Animation::num3Animation());
	this->addChild(pNum3, kZOrder_Countdown);

	// 「2」の表示
	CCSprite* pNum2 = CCSprite::create("count2.png");
	pNum2->setPosition(pNum3->getPosition());
	pNum2->setScale(0);
	pNum2->runAction(Animation::num2Animation());
	this->addChild(pNum2, kZOrder_Countdown);

	// 「1」の表示
	CCSprite* pNum1 = CCSprite::create("count1.png");
	pNum1->setPosition(pNum3->getPosition());
	pNum1->setScale(0);
	pNum1->runAction(Animation::num1Animation());
	this->addChild(pNum1, kZOrder_Countdown);

	// 「GO」の表示
	CCSprite* pGo = CCSprite::create("go.png");
	pGo->setPosition(pNum3->getPosition());
	pGo->setScale(0);
	pGo->runAction(Animation::gameStartAnimation(this, callfunc_selector(GameScene::startGame)));
	this->addChild(pGo, kZOrder_Countdown);
}

// ゲームスタートの処理を行う
void GameScene::startGame() {

	//敵：潜水艦、駆逐艦のaiを呼び出す
	//this->schedule(schedule_selector(GameScene::destroyerAI));
	//this->schedule(schedule_selector(GameScene::submarineAI));

}

// 毎フレームごとに衝突判定をチェックする
void GameScene::update(float dt) {
	// 物理シミュレーションの正確さを決定するパラメーター
	int velocityIterations = 8;
	int positionIterations = 1;
	// worldを更新する
	world->Step(dt, velocityIterations, positionIterations);

	// world内の全オブジェクトをループする
	for (b2Body* b = world->GetBodyList(); b; b = b->GetNext()) {
		if (!b->GetUserData()) {
			continue;													// オブジェクトが見つからない場合は次のループへ
		}
		CCNode* object = (CCNode*)b->GetUserData();						// オブジェクトを取得
		int objectTag = object->getTag();								// オブジェクトのタグを取得
		// 機体タグだった場合
		if (objectTag >= kTag_PlayerUnit && objectTag <= kTag_Missile) {
			// 被弾したユニットを判別
			PhysicsSprite* damagedUnit = (PhysicsSprite*)this->getChildByTag(objectTag);
			// 被弾したユニットのHPを減らす
			damagedUnit->setHp(damagedUnit->getHp() - 1);
			if (damagedUnit->getHp()) {									// HPが0になっていない場合
				Animation::hitAnimation(kTag_HitAnimation);				// 被弾アニメーションを取得
			} else {													// 0になった場合
				Animation::hitAnimation(kTag_DefeatAnimation);			// 撃沈アニメーションを取得
				if (objectTag == kTag_PlayerUnit) {						// 自機が撃沈した場合
					defeatPlayer();										// 自機撃沈関数を呼び出す
					createLifeCounter();								// 残機を再表示
				}else {													// 敵機を撃沈した場合
					removeObject(object, (void*)b);						// 撃沈したオブジェクトを削除
					if (objectTag != kTag_Missile) {
						enemyUnit_num--;									// 敵機の数を減らす
						if(!enemyUnit_num) {								// 敵機がなくなった場合
							finishGame();									// ゲームクリア
							break;											// 繰り返しから抜ける
						}
					}
				}
			}
		} else if (objectTag == kTag_Collision) {						// 機体同士もしくはプレイヤーが海底に衝突した場合
			defeatPlayer();												// 自機が撃沈される
		}
	}
	CCNode* position_of_destroyer = this->getChildByTag(kTag_EnemyDestroyer);	// ネコ画像のオブジェクトを生成
	CCPoint destroyer_loc = position_of_destroyer->getPosition();			// ネコの現在位置を取得
	CCNode* position_of_submarine = this->getChildByTag(kTag_EnemySubmarine);	// ネコ画像のオブジェクトを生成
	CCPoint submarine_loc = position_of_submarine->getPosition();			// ネコの現在位置を取得
	if (!(rand() % 30)) {
		destroyerAI();														// ランダムで駆逐艦のAIを呼び出す
	}
	if (!(rand() % 30)) {
		submarineAI();														// ランダムで潜水艦のAiを呼び出す
	}
}
// 自機撃沈関数
void GameScene::defeatPlayer () {
	this->lifepoint--;									// 残機を減らす
	if (lifepoint == -1) {								// 残機がなくなった場合
		finishGame();									// ゲームオーバー
	}
	// 自機を生成
	createUnit(player_VIT, kTag_PlayerUnit, submarine_VIT, playerUnit);
	createLifeCounter();								// 残機を再表示
}
// オブジェクトを除去する
void GameScene::removeObject(CCNode* pObject, void* body) {
	pObject->removeFromParentAndCleanup(true);			// シーンからオブジェクトを削除する
	world->DestroyBody((b2Body*)body);					// 重力世界からオブジェクトを削除する
}

void GameScene::finishGame() {

	const char* fileName = lifepoint == -1 ? "gameover.png" : "clear.png";	//countが13と同値であればhit%d.pngの、違う場合はdefeat%d.pngのファイルネームを取得しfileNameに代入

	// 「Game Over」を表示する
	CCSprite* gameOver = CCSprite::create(fileName);
	gameOver->setPosition(ccp(getWindowSize().width * 0.5, getWindowSize().height * 0.5));
	gameOver->setScale(0);
	gameOver->runAction(Animation::gameOverAnimation(this, callfunc_selector(GameScene::moveToNextScene)));
	this->addChild(gameOver, kZOrder_Countdown, kTag_Animation);
}

void GameScene::setScoreNumber() {
	int showScore;											// 表示するスコア
	int maxScore = 1;										// 表示可能な最大スコア
	// 繰返し文により、表示可能最大値を算出
	for(int i = score_and_Maxplace; i % 10; i--) {
		maxScore *= 10;
	}

	if (score_and_Maxplace > maxScore) {
		showScore = maxScore - 1;
	} else {
		showScore = score_and_Maxplace;
	}

	// 任意の桁数だけ繰り返し
	for (int i = score_and_Maxplace * 10; i % 10; i--) {

		CCString* tmp = (CCString*)scoreText->objectAtIndex(i % 10);							// 配列に格納するオブジェクトを宣言
		if (showScore * 10 / maxScore == 0) {
			i * 10 != 1 ?												// 配列の最後の要素であるか？
					tmp = CCString::create(" "):
					tmp = CCString::create("%d");

		} else {
			tmp = CCString::create("%d");
		}
		scoreText->addObject(tmp);													// 配列の末尾にオブジェクトをセット
	}
}
// 駆逐艦AI
void GameScene::destroyerAI() {
	CCNode* position_of_destroyer = this->getChildByTag(kTag_EnemyDestroyer);	// ネコ画像のオブジェクトを生成
	CCPoint destroyer_loc = position_of_destroyer->getPosition();			// ネコの現在位置を取得
	if(!(rand() % lifepoint / 3)) {
		createMissile(destroyer_loc);
	} else if(!rand() % 3) {
		CCMoveBy* action = CCMoveBy::create(80, ccp(getViewSize().width, 0));
		// 向きを反転
	}
}
// 潜水艦AI
void GameScene::submarineAI() {
	CCNode* position_of_submarine = this->getChildByTag(kTag_EnemySubmarine);	// ネコ画像のオブジェクトを生成
	CCPoint submarine_loc = position_of_submarine->getPosition();			// ネコの現在位置を取得
	if(!(rand() % lifepoint / 3)) {
		createMissile(submarine_loc);												// ランダムでミサイルを生成
	} else if(!rand() % 3) {
		CCMoveBy* action = CCMoveBy::create(80, ccp(getViewSize().width, 0));		// ランダムで移動方向を変更
		// 向きを反転
	} else if(!rand() % 3) {
		CCMoveBy* action = CCMoveBy::create(80, ccp(0, getViewSize().height));		// ランダムで向きを変更
		// 向きを反転
	}
}
// ミサイル作成
void GameScene::createMissile(CCPoint point) {
	PhysicsSprite* pMissile = new PhysicsSprite(1);										// 物理構造を持った画像オブジェクトを生成
	pMissile->autorelease();
	pMissile->initWithTexture(missileBatchNode->getTexture());						// りんごを指定位置にセット
	pMissile->setPosition(point);													// りんごを指定位置にセット
	this->addChild(pMissile, kZOrder_Missile, kTag_Missile);
	b2Body* missileBody;
	pMissile = createPhysicsBody(kTag_DynamicBody, pMissile, missileBody, kTag_Polygon);		// オブジェクトに物理構造を持たせる
}
// タッチ開始時のイベント
void GameScene::touchesBegan(CCSet* touches, CCEvent* pEvent ) {
	for (CCSetIterator it = touches->begin(); it != touches->end(); ++it) {	// タッチ位置を順に探索
		CCTouch* touch  = (CCTouch *)(*it);									// 取得した値をノードと照合するためccTouch型に変換
		bool touch_judge = 0;											// 返り値として渡すためのタッチ判定フラグ
		tag_no = this->kTag_Key_Up;								// 各種ボタンの先頭のタグを取得
		CCPoint loc = touch->getLocation();							// タッチ位置を取得
		// 各ボタンのタッチ判定を繰り返し
		for (CCNode* i; tag_no - this->kTag_Key_Up < buttons_sum; tag_no++) {
			i = this->getChildByTag(tag_no);							// 各種ハンドルオブジェクトでiを初期化し、タップ可能にする
			touch_judge = i->boundingBox().containsPoint(loc);			// タグの座標がタッチされたかの判定を行う
			m_touchFlag[tag_no] = touch_judge;
			m_touchAt[tag_no] = touch->getLocation();
		}
	}
}
// スワイプしている途中に呼ばれる
void GameScene::touchesMoved(CCSet* touches, CCEvent* pEvent ) {

		//タッチしたボタンの数だけループ
		for (CCSetIterator it = touches->begin(); it != touches->end(); ++it) {
			CCTouch *touch = (CCTouch *)(*it);					// タッチボタンの取得
			tag_no = this->getTag();								// タグを取得

			//もし取得したタグとスイッチタグが同値であるならば以下ブロック
			if(tag_no == kTag_Switch) {							//タッチボタンがスイッチであれば以下ブロック
				CCPoint loc = touch->getLocation();										// タッチ座標の取得

				float threshold = CCDirector::sharedDirector()->getWinSize().width * 0.005f;	// しきい値の取得
				CCSprite* pSwitch = (CCSprite*)this->getChildByTag(kTag_Switch);		// スイッチのオブジェクトを取得

				pSwitch->setPosition(ccp(loc.x, pSwitch->getPositionY()));				//スイッチをタップ位置に追従

				m_touchAt[tag_no]  = touch->getLocation();									// タッチ位置を更新
				// 以下、必要な情報を取得し、update内で処理してやると良い
			}
	}
}
// タッチ終了時のイベント
void GameScene::touchesEnded(CCSet* touches, CCEvent* pEvent ) {
	for (CCSetIterator it = touches->begin(); it != touches->end(); ++it) {
		CCTouch* touch  = (CCTouch *)(*it);
		bool touch_judge = 0;											// 返り値として渡すためのタッチ判定フラグ
		tag_no = this->kTag_Key_Up;								// 各種ボタンの先頭のタグを取得
		CCPoint loc = touch->getLocation();							// タッチ位置を取得
		// 各ボタンのタッチ判定を繰り返し
		for (CCNode* i; tag_no - this->kTag_Key_Up < buttons_sum; tag_no++) {
			i = this->getChildByTag(tag_no);							// 各種ハンドルオブジェクトでiを初期化し、タップ可能にする
			touch_judge = i->boundingBox().containsPoint(loc);			// タグの座標がタッチされたかの判定を行う

			m_touchFlag[tag_no] = !(touch_judge);
		}
	}
}
// ウィンドウサイズをゲットする
CCSize GameScene::getWindowSize() {
	return CCDirector::sharedDirector()->getWinSize();					// ウィンドウサイズを取得
}
// 背景サイズをゲットする
CCSize GameScene::getViewSize() {
	return CCEGLView::sharedOpenGLView()->getFrameSize();				// シーンのサイズを取得
}
// スクロールスピードの倍率をゲットする
float GameScene::getdealofScrollSpead() { return this->dealofScrollSpead; }
