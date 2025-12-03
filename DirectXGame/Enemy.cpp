#define NOMINMAX
#include "Enemy.h"
#include "GameScene.h"

using namespace KamataEngine;

void Enemy::initialize(Model* model, Camera* camera, const Vector3& position) {

	assert(model);

	model_ = model;
	camera_ = camera;
	// 初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	// 角度
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;
	// 速度を設定する
	velocity_ = {-kWalkSpeed, 0, 0};

	warkTimer_ = 0.0f;
}

void Enemy::Update() {

	// 変更リクエストがあったら
	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振るまいを変更する
		behavior_ = behaviorRequest_;

		// 各振るまいごとの初期化を実行
		switch (behavior_) {
		case Behavior::kDeath:
		default:
			// counter_ = 0;
			break;
		}

		// 振るまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Behavior::kWalking:

		//	移動
		worldTransform_.translation_ += velocity_;

		// タイマーを
		warkTimer_ += 1.0f / 60.0f;

		// 回転アニメーション
		// worldTransform_.rotation_.x = std::sin(std::numbers::pi_v<float> * 2.0f * (warkTimer_ / kWarkMotionTime));
		param = std::sin(std::numbers::pi_v<float> * 2.0f * (warkTimer_ / kWarkMotionTime));
		degree = kWarkMotionAngleStart + kWarkMotionAngleEnd * (param + 1.0f) / 2.0f;
		worldTransform_.rotation_.x = ToRadians(degree);

		WorldTransformUpdate(worldTransform_);
		break;
	case Behavior::kDeath:

		isCollisionDisabled_ = true;

		// タイマーを加算
		counter_ += 1.0f / 60.0f;

		worldTransform_.rotation_.y += 0.6f;
		worldTransform_.rotation_.x = EaseOut(ToRadians(kDefeatedMotionAngleStart), ToRadians(kDefeatedMotionAngleEnd), counter_ / kDefeatedTime);

		WorldTransformUpdate(worldTransform_);
		if (counter_ >= kDefeatedTime) {
			isDead_ = true;
		}
		break;
	}
}

void Enemy::Draw() { model_->Draw(worldTransform_, *camera_); }

Vector3 Enemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Enemy::OnCollision(const Player* player) 
{

	//(void)player;

	if (behavior_ == Behavior::kDeath) 
	{
		// 敵がやられているなら何もしない
		return;
	}
	// プレイヤーが攻撃中なら敵が死ぬ
	if (player->IsAttack()) {
		// 敵の振るまいをデス演出に変更
		behaviorRequest_ = Behavior::kDeath;

		// 敵と自キャラの中間位置にエフェクトを生成
		Vector3 effectPos = 
		{
		    (GetWorldPosition() + player->GetWorldPosition()).x / 2.0f,//x
		    (GetWorldPosition() + player->GetWorldPosition()).y/ 2.0f,//y
		    (GetWorldPosition() + player->GetWorldPosition()).z / 2.0f,//z
		};
		gameScene_->CreateHitEffect(effectPos);
		
	}
}

