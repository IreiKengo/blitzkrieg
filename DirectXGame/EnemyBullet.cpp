#include "EnemyBullet.h"
#include <cassert>
#include "MapChipField.h"

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
	if (isDead_) {
		return;
	}
	float kDeltaTime = 1.0f / 60.0f;

	// 移動処理
	worldTransform_.translation_ += velocity_;

	// 一定時間（3秒）経過したら消滅
	deathTimer_ += kDeltaTime;
	if (deathTimer_ >= kLifeTime) {
		isDead_ = true;
	}

	WorldTransformUpdate(worldTransform_);

	CheckMapCollision();
}

void EnemyBullet::Draw() {
	if (!isDead_)
	{
		model_->Draw(worldTransform_, *camera_);
	}
}

Vector3 EnemyBullet::GetWorldPosition() const { return {worldTransform_.matWorld_.m[3][0], worldTransform_.matWorld_.m[3][1], worldTransform_.matWorld_.m[3][2]}; }

void EnemyBullet::CheckMapCollision()
{

	if (!mapChipField_)
		return;

	// 弾の現在座標からマップチップのインデックスを取得
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);

	// インデックス位置のマップチップ種別を取得
	MapChipType type = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	
	if (type == MapChipType::kBlock) {
		isDead_ = true;
	}

}

AABB EnemyBullet::GetAABB() const {
	Vector3 worldPos = GetWorldPosition();
	float halfW = kWidth / 2.0f;
	float halfH = kHeight / 2.0f;

	AABB aabb;
	aabb.min = {worldPos.x - halfW, worldPos.y - halfH, worldPos.z - halfW};
	aabb.max = {worldPos.x + halfW, worldPos.y + halfH, worldPos.z + halfW};
	return aabb;
}

void EnemyBullet::OnCollision() 
{

	isDead_ = true;

}
