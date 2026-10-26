#define NOMINMAX
#include "Player.h"
#include "GameScene.h"
#include "MapChipField.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

void Player::Initialize(KamataEngine::Model* model, Model* modelAttack, KamataEngine::Camera* camera, const Vector3& position) {

	// NULLポインタチェック
	assert(model);

	model_ = model;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	modelAttack_ = modelAttack;
	worldTransformAttack_.Initialize();
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	// 引数の内容をメンバ変数に記録
	camera_ = camera;

	behavior_ = Behavior::kRoot;
	behaviorRequest_ = Behavior::kUnknown;
	// 初期接地判定
	CollisionMapInfo info = {};
	info.move = {};
	UpdateOnGround(info); // 初期座標で接地フラグを確認
}

// 移動入力
void Player::InputMoveGround() {

	// 左右移動操作

	// 左右加速
	Vector3 acceleration = {};
	if (Input::GetInstance()->PushKey(DIK_D)) {
		// 左移動中の右入力
		if (velocity_.x < 0.0f) {
			// 速度と逆方向に入力中は急ブレーキ
			velocity_.x *= kBrakePower;
		}
		acceleration.x += kAcceleration;

		// 左右状態の切り替え
		// if (lrDirection_ != LRDirection::kRight) {

		lrDirection_ = LRDirection::kRight; // 強制的に向きを更新
		turnFirstRotationY_ = worldTransform_.rotation_.y;
		turnTimer_ = kTimeTurn; // 旋回タイマーリセット
		                        //	}
	} else if (Input::GetInstance()->PushKey(DIK_A)) {
		// 右移動中の左入力
		if (velocity_.x > 0.0f) {
			// 速度と逆方向に入力中は急ブレーキ
			velocity_.x *= kBrakePower;
		}
		acceleration.x -= kAcceleration;
		// 左右状態の切り替え
		// if (lrDirection_ != LRDirection::kLeft) {
		lrDirection_ = LRDirection::kLeft;
		turnFirstRotationY_ = worldTransform_.rotation_.y;
		turnTimer_ = kTimeTurn;
		//}
	}

	// 加速／減速
	velocity_ += acceleration;

	if (acceleration.x == 0) {
		// 非入力時は移動減衰をかける
		velocity_.x *= (1.0f - kDeceleration);
	}
}

void Player::InputMoveAir() {

	if (wallJumpLockTimer_ > 0.0f) {
		return; // 空中操作を受け付けない
	}

	if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {
		Vector3 acceleration = {};

		if (Input::GetInstance()->PushKey(DIK_D)) {
			if (velocity_.x < 0.0f) {
				// 空中ではブレーキ弱め
				velocity_.x *= kAirBrakePower;
			}
			acceleration.x += kAirAcceleration;

			// 向き切り替え
			lrDirection_ = LRDirection::kRight;
		} else if (Input::GetInstance()->PushKey(DIK_A)) {
			if (velocity_.x > 0.0f) {
				velocity_.x *= kAirBrakePower;
			}
			acceleration.x -= kAirAcceleration;
			lrDirection_ = LRDirection::kLeft;
		}

		velocity_ += acceleration;

	} else {

		velocity_.x *= (1.0f - kAirDeceleration);
	}
}

void Player::Turn() {
	// 左右の自キャラ角度
	float targetY = (lrDirection_ == LRDirection::kRight) ? std::numbers::pi_v<float> / 2.0f : std::numbers::pi_v<float> * 3.0f / 2.0f;
	// 滑らかに回転（kTurnSpeedは0.1~0.3くらいが自然）
	worldTransform_.rotation_.y = EaseInOut(worldTransform_.rotation_.y, targetY, kTurnSpeed);
}

void Player::CheckMapCollision(CollisionMapInfo& info) {

	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}
// マップ衝突判定上方向
void Player::CheckMapCollisionUp(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.move.y <= 0) {
		return;
	}
	// 移動後の4つの角の計算
	auto positionsNew = GetCorners(worldTransform_.translation_ + info.move);

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上の当たり判定を行う
	bool hit = false;

	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, +kHeight / 2.0f, 0));
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
			info.ceiling = true;

			if (velocity_.y > 0.0f) {
				velocity_.y = 0.0f;
			}
		}
	}
}
// マップ衝突判定下方向
void Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	// 下降あり？
	if (info.move.y >= 0) {
		return;
	}

	// 移動後の4つの角の計算
	auto positionsNew = GetCorners(worldTransform_.translation_ + info.move);

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真下の当たり判定を行う
	bool hit = false;
	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, -kHeight / 2.0f, 0));
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, -kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
			// 地面に当たったことを記録する
			info.landing = true;
		}
	}
}
// マップ衝突判定右方向
void Player::CheckMapCollisionRight(CollisionMapInfo& info) {
	// 右移動あり？
	if (info.move.x <= 0) {
		return;
	}

	// 移動後の4つの角の計算
	auto positionsNew = GetCorners(worldTransform_.translation_ + info.move);

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 右の当たり判定を行う
	bool hit = false;
	// 右上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(+kWidth / 2.0f, 0, 0));
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(+kWidth / 2.0f, 0, 0));
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
			info.hitWall = true;
			wallNormalX_ = -1.0f;
		}
	}
}
// マップ衝突判定左方向
void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {

	// 左移動あり？
	if (info.move.x >= 0) {
		return;
	}

	// 移動後の4つの角の計算
	auto positionsNew = GetCorners(worldTransform_.translation_ + info.move);

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 左の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(-kWidth / 2.0f, 0, 0));
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(-kWidth / 2.0f, 0, 0));
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank));
			info.hitWall = true;
			wallNormalX_ = 1.0f;
		}
	}
}

std::array<Vector3, Player::kNumCorner> Player::GetCorners(const Vector3& center) const {
	std::array<Vector3, kNumCorner> corners = {};

	corners[kRightBottom] = center + Vector3(+kWidth / 2.0f, -kHeight / 2.0f, 0);
	corners[kLeftBottom] = center + Vector3(-kWidth / 2.0f, -kHeight / 2.0f, 0);
	corners[kRightTop] = center + Vector3(+kWidth / 2.0f, +kHeight / 2.0f, 0);
	corners[kLeftTop] = center + Vector3(-kWidth / 2.0f, +kHeight / 2.0f, 0);

	return corners;
}

void Player::UpdateOnGround(const CollisionMapInfo& info) {
	// 自キャラが接地状態？
	if (onGround_) {
		// 接地状態の処理
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {

			// 移動後の4つの角の計算
			std::array<Vector3, kNumCorner> positionsNew = GetCorners(worldTransform_.translation_ + info.move);

			MapChipType mapChipType;
			// 真下の当たり判定を行う
			bool hit = false;
			// 左下点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 落下開始
			if (!hit && info.move.y <= 0.0f) {
				// 空中状態に切り替える
				onGround_ = false;
			}
		}

	} // 空中
	else {
		// 空中状態の処理
		// 着地フラグ
		if (info.landing) {
			// 着地状態に切り替える(落下を止める)
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// V速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

Vector3 Player::GetWorldPosition() const {
	// ワールド座標を入れる変数
	Vector3 worldPos = {0.0f, 0.0f, 0.0f};
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Player::GetAABB() const {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb = {0.0f, 0.0f, 0.0f};

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {
	if (behavior_ == Behavior::kDeath || isDead_) {
		return;
	}
	if (IsAttack()) {
		return; // 攻撃中はダメージ無効
	}
	(void)enemy;

	behaviorRequest_ = Behavior::kDeath;
}

void Player::OnCollision(const EnemyBullet* bullet) {

	if (behavior_ == Behavior::kDeath || isDead_) {
		return;
	}
	(void)bullet;
	behaviorRequest_ = Behavior::kDeath;
}

// 通常
void Player::BehaviorRootInitialize() {

	// velocity_.y = 0.0f;
}
void Player::BehaviorRootUpdate() {

	// 1. 移動入力
	InputMoveGround();

	// 2. 旋回処理
	Turn();

	// 3. X速度制限
	velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

	// 4. 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.move = velocity_;

	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	// 5. マップ衝突チェック
	CheckMapCollision(collisionMapInfo);

	// 6. ジャンプ判定（地面にいる場合のみ）
	if (onGround_ && jumpInputTimer_ > 0.0f) {
		behaviorRequest_ = Behavior::kAir;
		velocity_.y = kJumpAcceleration / 60.0f;
		justJumpedTimer_ = 0.1f;
		onGround_ = false; // 空中状態にする
		jumpInputTimer_ = 0.0f;
		return;
	}

	// 7. 移動
	worldTransform_.translation_ += collisionMapInfo.move;

	// 8. 天井に当たった場合の速度リセット
	if (collisionMapInfo.ceiling) {
		velocity_.y = 0.0f;
	}

	// 9. 接地判定更新
	UpdateOnGround(collisionMapInfo);

	if (collisionMapInfo.hitWall) {
		behaviorRequest_ = Behavior::kWall;
		return;
	}

	// 11. 攻撃判定
	if (attackInputTimer_ > 0.0f) {
		behaviorRequest_ = Behavior::kAttack;
		attackInputTimer_ = 0.0f;
		return;
	}

	// 12. 空中に移行する場合
	if (!onGround_) {
		behaviorRequest_ = Behavior::kAir;
		return;
	}
}

// アタック
void Player::BehaviorAttackInitialize() {

	// カウンター初期化
	attackParameter_ = 0;

	velocity_ = {};

	// 溜めフェーズから始める
	attackPhase_ = AttackPhase::kReservoir;
}
void Player::BehaviorAttackUpdate() {

	const Vector3 attackVelocity = {0.8f, 0.0f, 0.0f};

	// 攻撃動作用の速度
	Vector3 velocity{};

	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
		// 溜め
	case AttackPhase::kReservoir:
	default: {
		velocity = {};
		float t = static_cast<float>(attackParameter_) / reservoirTime;
		worldTransform_.scale_.z = EaseInOut(1.0f, 0.3f, t);
		worldTransform_.scale_.y = EaseInOut(1.0f, 1.6f, t);
		// 前進動作に移行
		if (attackParameter_ >= reservoirTime) {
			attackPhase_ = AttackPhase::kAction;
			attackParameter_ = 0.0f; // カウンターリセット
		}
		break;
	}
		// 突進
	case AttackPhase::kAction: {
		if (lrDirection_ == LRDirection::kRight) {
			velocity = +attackVelocity;
		} else {
			velocity = -attackVelocity;
		}
		float t = static_cast<float>(attackParameter_) / kActionTime;
		worldTransform_.scale_.z = EaseInOut(0.3f, 1.3f, t);
		worldTransform_.scale_.y = EaseInOut(1.6f, 0.7f, t);
		// 余韻動作に移行
		if (attackParameter_ >= kActionTime) {
			attackPhase_ = AttackPhase::kRecovery;
			attackParameter_ = 0.0f; // カウンターリセット
		}
	} break;
		// 余韻
	case AttackPhase::kRecovery: {
		velocity = {};
		float t = static_cast<float>(attackParameter_) / recoveryTime;
		worldTransform_.scale_.z = EaseInOut(1.3f, 1.0f, t);
		worldTransform_.scale_.y = EaseInOut(0.7f, 1.0f, t);

		// 通常行動に戻る
		if (attackParameter_ >= recoveryTime) {
			behaviorRequest_ = Behavior::kRoot;
		}
		break;
	}
	}

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity;

	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	// マップ衝突チェック
	CheckMapCollision(collisionMapInfo);

	// 移動
	worldTransform_.translation_ += collisionMapInfo.move;

	// 移動後のプレイヤー位置
	Vector3 playerPos = worldTransform_.translation_;

	// 向いてる方向（Xのみ）
	// float dir = (lrDirection_ == LRDirection::kRight) ? 1.0f : -1.0f;

	worldTransformAttack_.translation_ = playerPos;

	// 向きはプレイヤーと同じ
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	// スケールはいじらない（裏面消えるから）
	worldTransformAttack_.scale_ = {1, 1, 1};
}

//デス演出
void Player::BehaviorDeathInitialize()
{
	deathTimer_ = 0.0f;
	
	velocity_ = {0.0f,0.3f,0.3f};
	

}
void Player::BehaviorDeathUpdate() 
{
	deathTimer_ += deltaTime_;

	//デス演出
	 
	//落下速度
	velocity_.y -= kGravityAcceleration / 60.0f;

	// 速度制限
	velocity_.y = std::max(velocity_.y, -kDeathLimitFallSpeed);

	

	//衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.move = velocity_;

	//移動
	worldTransform_.translation_ += collisionMapInfo.move;

	if (deathTimer_ >= kDeathDuration) {
		isDead_ = true;
	}
}
// 空中
void Player::BehaviorAirInitialize() {

	behavior_ = Behavior::kAir;
	onGround_ = false;
}
void Player::BehaviorAirUpdate() {

	if (wallJumpLockTimer_ > 0.0f) {
		wallJumpLockTimer_ -= 1.0f / 60.0f;
	}

	InputMoveAir();
	Turn();
	// 落下速度
	velocity_.y -= kGravityAcceleration / 60.0f;

	// 速度制限
	velocity_.x = std::clamp(velocity_.x, -kLimitAirSpeed, kLimitAirSpeed);
	velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);

	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;

	CheckMapCollision(collisionMapInfo);

	worldTransform_.translation_ += collisionMapInfo.move;

	// 接地判定
	if (collisionMapInfo.landing && justJumpedTimer_ <= 0.0f) {
		onGround_ = true;
		velocity_.x *= (1.0f - kAttenuationLanding);
		velocity_.y = 0.0f;
		behaviorRequest_ = Behavior::kRoot;
		return;
	}

	// 壁に接触したらWallへ
	if (collisionMapInfo.hitWall) {
		velocity_.x = 0.0f;
		behaviorRequest_ = Behavior::kWall;
		return;
	}
}
// 壁
void Player::BehaviorWallInitialize() {

	// 壁ジャンプ可能フラグをセット
	canWallJump_ = true;

	// 壁にくっついた状態をフラグで管理
	onGround_ = false;
	behavior_ = Behavior::kWall;
}
void Player::BehaviorWallUpdate() {

	// 壁に触れている間は横速度を止める（基本スライド）
	velocity_.x = 0.0f;

	// 重力（壁スライド時は減速）
	velocity_.y -= kGravityAcceleration / 60.0f;
	velocity_.y = std::max(velocity_.y, -kLimitFallSpeed * 0.3f); // 最大落下速度も抑える

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;
	// 衝突チェック
	CheckMapCollision(collisionMapInfo);

	worldTransform_.translation_ += collisionMapInfo.move;

	if (collisionMapInfo.landing) {
		onGround_ = true;

		velocity_.y = 0.0f;
		behaviorRequest_ = Behavior::kRoot;
		return;
	}

	if (collisionMapInfo.hitWall) {
		wallJumpBufferTimer_ = kWallJumpBufferTime; // 接触維持中はリセット

	} else {
		if (wallJumpBufferTimer_ > 0.0f) {
			wallJumpBufferTimer_ -= 1.0f / 60.0f;
		}
	}

	// 壁ジャンプ
	if (canWallJump_ && jumpInputTimer_ > 0.0f) {
		// 壁と反対方向へ跳ぶ
		velocity_.x = wallNormalX_ * kLimitRunSpeed;
		velocity_.y = kWallJumpPower / 60.0f;

		// 向き更新
		lrDirection_ = (wallNormalX_ > 0.0f) ? LRDirection::kRight : LRDirection::kLeft;

		worldTransform_.rotation_.y = (lrDirection_ == LRDirection::kRight) ? std::numbers::pi_v<float> / 2.0f : std::numbers::pi_v<float> * 3.0f / 2.0f;

		wallJumpLockTimer_ = 0.2f;
		justJumpedTimer_ = 0.1f;

		canWallJump_ = false;
		wallJumpBufferTimer_ = 0.0f;
		jumpInputTimer_ = 0.0f;

		behaviorRequest_ = Behavior::kAir;
		return;
	}

	// 壁から離れる入力ならAirへ
	bool awayFromWall = (wallNormalX_ > 0.0f && Input::GetInstance()->PushKey(DIK_A)) || (wallNormalX_ < 0.0f && Input::GetInstance()->PushKey(DIK_D));

	if (awayFromWall) {
		behaviorRequest_ = Behavior::kAir;
		return;
	}

	if (!collisionMapInfo.hitWall && wallJumpBufferTimer_ <= 0.0f) {
		canWallJump_ = false;
		velocity_.x = 0.0f;
		behaviorRequest_ = Behavior::kAir;
		return;
	}
}

void Player::Update() {


	if (gameScene_ != nullptr) {
		if (!gameScene_->IsGameStart()) {
			WorldTransformUpdate(worldTransform_);
			return; // カウントダウン中は動きを止める
		}
	}
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		jumpInputTimer_ = jumpBufferTime_;
	}

	if (jumpInputTimer_ > 0.0f) {
		jumpInputTimer_ -= 1.0f / 60.0f;
		if (jumpInputTimer_ < 0.0f) {
			jumpInputTimer_ = 0.0f;
		}
	}

	if (Input::GetInstance()->TriggerKey(DIK_J)) {
		attackInputTimer_ = kAttackBufferTime_;
	}
	if (attackInputTimer_ > 0.0f) {
		attackInputTimer_ -= 1.0f / 60.0f;
	}

	if (justJumpedTimer_ > 0.0f) {
		justJumpedTimer_ -= 1.0f / 60.0f;
	}

	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振るまいを変更する
		behavior_ = behaviorRequest_;
		// 各振るまいごとの初期化を実行
		switch (behavior_) {
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;
		case Behavior::kAir:
			BehaviorAirInitialize();
			break;
		case Behavior::kWall:

			BehaviorWallInitialize();
			break;
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		case Behavior::kDeath:

			BehaviorDeathInitialize();
			break;
		}
		// 振るまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;
	case Behavior::kAir:
		BehaviorAirUpdate();
		break;
	case Behavior::kWall:
		BehaviorWallUpdate();
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	case Behavior::kDeath:
		BehaviorDeathUpdate();
		break;
	}

	WorldTransformUpdate(worldTransform_);
	WorldTransformUpdate(worldTransformAttack_);
}

void Player::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);
	if (behavior_ == Behavior::kAttack) {
		switch (attackPhase_) {
		case AttackPhase::kReservoir:
			break;
		case AttackPhase::kAction:
		case AttackPhase::kRecovery:
			modelAttack_->Draw(worldTransformAttack_, *camera_);
			break;
		}
	}
}
