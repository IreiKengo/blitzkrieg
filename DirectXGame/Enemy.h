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
	static inline const float kWalkSpeed = 0.02f;
	// 速度
	KamataEngine::Vector3 velocity_ = {};

	//最初の角度
	static inline const float kWarkMotionAngleStart = -10.0f;
	//最後の角度
	static inline const float kWarkMotionAngleEnd = 40.0f;
	//アニメーションの周期となる時間
	static inline const float kWarkMotionTime = 1.0f;
	//経過時間
	float warkTimer_ = 0.0f;

	float param = 0.0f;
	float degree = 0.0f;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.0f;
	static inline const float kHeight = 0.9f;

	// デスフラグ
	bool isDead_ = false;

	// 振るまい
	enum class Behavior {
		kUnknown = -1, // 無効な状態
		kWalking,   // 歩行
		kDeath,   // デス演出
	};

	// 振るまい
	Behavior behavior_ = Behavior::kWalking;
	Behavior behaviorRequest_ = Behavior::kUnknown;
	
	static inline const float kDefeatedMotionAngleStart = 0.0f;

	static inline const float kDefeatedMotionAngleEnd = -60.0f;
	float counter_ = 0.0f; // カウンター

	static inline const float kDefeatedTime = 0.6f;

	bool isCollisionDisabled_ = false;

	GameScene* gameScene_ = nullptr;

public:
	void initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();

	//衝突応答
	void OnCollision(const Player* player);

	// デスフラグのgetter
	bool IsDead() const { return isDead_; }

	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

};