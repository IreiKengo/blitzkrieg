#include "EnemyBullet.h"
#include <cassert>

using namespace KamataEngine;

void EnemyBullet::Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& velocity) {
	assert(model);
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {0.7f, 0.7f, 0.7f}; // 弾の大き
	velocity_ = velocity;
	isDead_ = false;
	deathTimer_ = 0.0f;
}

void EnemyBullet::Update() {
	float kDeltaTime = 1.0f / 60.0f;

	// 移動処理
	worldTransform_.translation_ += velocity_;

	// 一定時間（3秒）経過したら消滅
	deathTimer_ += kDeltaTime;
	if (deathTimer_ >= kLifeTime) {
		isDead_ = true;
	}

	WorldTransformUpdate(worldTransform_);
}

void EnemyBullet::Draw() {
	if (!isDead_)
	{
		model_->Draw(worldTransform_, *camera_);
	}
}

Vector3 EnemyBullet::GetWorldPosition() const { return {worldTransform_.matWorld_.m[3][0], worldTransform_.matWorld_.m[3][1], worldTransform_.matWorld_.m[3][2]}; }

AABB EnemyBullet::GetAABB() const {
	Vector3 worldPos = GetWorldPosition();
	float halfW = kWidth / 2.0f;
	float halfH = kHeight / 2.0f;

	AABB aabb;
	aabb.min = {worldPos.x - halfW, worldPos.y - halfH, worldPos.z - halfW};
	aabb.max = {worldPos.x + halfW, worldPos.y + halfH, worldPos.z + halfW};
	return aabb;
}