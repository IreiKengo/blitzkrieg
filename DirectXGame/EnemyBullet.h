#pragma once
#include "KamataEngine.h"
#include "Math.h"


class EnemyBullet
{
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);
	void Update();
	void Draw();
	bool IsDead() const { return isDead_; }
	AABB GetAABB() const;
	KamataEngine::Vector3 GetWorldPosition() const;

	private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Vector3 velocity_ = {0.0f, 0.0f, 0.0f};

	bool isDead_ = false;
	float deathTimer_ = 0.0f;
	const float kLifeTime = 3.0f; // 3秒経過したら自動消滅

	// 当たり判定用のサイズ設定
	const float kWidth = 0.4f;
	const float kHeight = 0.4f;
};