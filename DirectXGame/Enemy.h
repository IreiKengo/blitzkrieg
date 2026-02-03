#pragma once
#include "MapChipField.h"
#include "Math.h"
#include <KamataEngine.h>
#include <algorithm>
#include <numbers>
#include <vector>
#include "Player.h"

class GameScene;



//GameScene;

class Enemy {
private:
	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 歩行の速度
	static inline const float kWalkSpeed = 0.05f;
	// 速度
	KamataEngine::Vector3 velocity_ = {};

	//最初の角度
	static inline const float kWalkMotionAngleStart = -10.0f;
	//最後の角度
	static inline const float kWalkMotionAngleEnd = 40.0f;
	//アニメーションの周期となる時間
	static inline const float kWalkMotionTime = 1.0f;
	//経過時間
	float walkTimer_ = 0.0f;

	float param = 0.0f;
	float degree = 0.0f;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.15f;

	// 微小な数値
	static inline const float kGroundSearchHeight = 0.12f;
	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.02f;
	static inline const float kAttenuationWall = 0.03f;

	// 接地状態フラグ
	bool onGround_ = true;

	// デスフラグ
	bool isDead_ = false;

	// 振るまい
	enum class Behavior {
		kUnknown = -1, // 無効な状態
		kRoll,   // 転がる
		kDeath,   // デス演出
	};

	// 振るまい
	Behavior behavior_ = Behavior::kRoll;
	Behavior behaviorRequest_ = Behavior::kUnknown;
	
	static inline const float kDefeatedMotionAngleStart = 0.0f;

	static inline const float kDefeatedMotionAngleEnd = -60.0f;
	float counter_ = 0.0f; // カウンター

	static inline const float kDefeatedTime = 0.6f;

	bool isCollisionDisabled_ = false;

	GameScene* gameScene_ = nullptr;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// 重力加速度(下方向)
	static inline const float kGravityAcceleration = 0.8f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.4f;


public:

	// 当たり判定用の角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool ceiling = false;       // 天井衝突
		bool landing = false;       // 着地
		bool hitWall = false;       // 壁接触
		KamataEngine::Vector3 move = {0.0f,0.0f,0.0f}; // 移動量
	};


	void initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	// ワールド座標を取得
 KamataEngine::Vector3 GetWorldPosition()const;

	// AABBを取得
	AABB GetAABB()const;

	//衝突応答
	void OnCollision(const Player* player);

	// デスフラグのgetter
	bool IsDead() const { return isDead_; }

	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// 角の座標をまとめて取得
	std::array<KamataEngine::Vector3, kNumCorner> GetCorners(const KamataEngine::Vector3& center) const;

	// マップ衝突系
	void CheckMapCollision(CollisionMapInfo& info);
	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);


	void UpdateOnGround(const CollisionMapInfo& info);
	


	void BehaviorRollInitialize();
	void BehaviorRollUpdate();
	void BehaviorDeathInitialize();
	void BehaviorDeathUpdate();

};