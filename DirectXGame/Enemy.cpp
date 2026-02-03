#define NOMINMAX
#include "Enemy.h"
#include "GameScene.h"
#include <algorithm>
#include <cassert>
#include <numbers>

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
	worldTransform_.scale_ = {0.5f, 0.5f, 0.5f};
	// 速度を設定する
	velocity_ = {-kWalkSpeed, 0, 0};

	walkTimer_ = 0.0f;

	// 初期接地判定
	CollisionMapInfo info = {};
	info.move = {};
	UpdateOnGround(info); // 初期座標で接地フラグを確認
	info.landing = false;

	
}

void Enemy::Update() {

	// 変更リクエストがあったら
	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振るまいを変更する
		behavior_ = behaviorRequest_;

		// 各振るまいごとの初期化を実行
		switch (behavior_) {
		case Behavior::kRoll:
			BehaviorRollInitialize();
			break;
		case Behavior::kDeath:

			BehaviorDeathInitialize();

			break;
		}

		// 振るまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}



	switch (behavior_) {
	case Behavior::kRoll:

		BehaviorRollUpdate();

		break;
	case Behavior::kDeath:

		BehaviorDeathUpdate();

		break;
	}

	WorldTransformUpdate(worldTransform_);
}

void Enemy::Draw() { model_->Draw(worldTransform_, *camera_); }

 Vector3 Enemy::GetWorldPosition() const {
	// ワールド座標を入れる変数
	Vector3 worldPos = {0.0f,0.0f,0.0f};
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Enemy::GetAABB() const{
	Vector3 worldPos = GetWorldPosition();

	AABB aabb = {{0.0f, 0.0f, 0.0f}, {0.0f,0.0f,0.0f}};

	float halfW = (kWidth * worldTransform_.scale_.x) / 2.0f;
	float halfH = (kHeight * worldTransform_.scale_.y) / 2.0f;

	aabb.min = {worldPos.x - halfW, worldPos.y - halfH, worldPos.z - halfW};
	aabb.max = {worldPos.x + halfW, worldPos.y + halfH, worldPos.z + halfW};

	return aabb;
}

void Enemy::OnCollision(const Player* player) {

	if (behavior_ == Behavior::kDeath) {
		// 敵がやられているなら何もしない
		return;
	}
	// プレイヤーが攻撃中なら敵が死ぬ
	if (player->IsAttack()) {
		// 敵の振るまいをデス演出に変更
		behaviorRequest_ = Behavior::kDeath;

		// 敵と自キャラの中間位置にエフェクトを生成
		Vector3 effectPos = {
		    (GetWorldPosition() + player->GetWorldPosition()).x / 2.0f, // x
		    (GetWorldPosition() + player->GetWorldPosition()).y / 2.0f, // y
		    (GetWorldPosition() + player->GetWorldPosition()).z / 2.0f, // z
		};
		gameScene_->CreateHitEffect(effectPos);
	}
}

void Enemy::CheckMapCollision(CollisionMapInfo& info) {

	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}
// マップ衝突判定上方向
void Enemy::CheckMapCollisionUp(CollisionMapInfo& info) {
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
void Enemy::CheckMapCollisionDown(CollisionMapInfo& info) {
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
void Enemy::CheckMapCollisionRight(CollisionMapInfo& info) {
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
		}
	}
}
// マップチップ衝突判定左方向
void Enemy::CheckMapCollisionLeft(CollisionMapInfo& info) {

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
			info.move.x = std::max(0.0f, rect.right - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
			info.hitWall = true;
		}
	}
}

std::array<Vector3, Enemy::kNumCorner> Enemy::GetCorners(const Vector3& center) const {
	std::array<Vector3, kNumCorner> corners = {};

	corners[kRightBottom] = center + Vector3(+kWidth / 2.0f, -kHeight / 2.0f, 0);
	corners[kLeftBottom] = center + Vector3(-kWidth / 2.0f, -kHeight / 2.0f, 0);
	corners[kRightTop] = center + Vector3(+kWidth / 2.0f, +kHeight / 2.0f, 0);
	corners[kLeftTop] = center + Vector3(-kWidth / 2.0f, +kHeight / 2.0f, 0);

	return corners;
}

void Enemy::UpdateOnGround(const CollisionMapInfo& info) {
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
			// V速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}


void Enemy::BehaviorRollInitialize() { counter_ = 0.0f; }
void Enemy::BehaviorRollUpdate() {

	//重力処理
	if (!onGround_) {
		velocity_.y -= kGravityAcceleration / 60.0f;

		 velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}

	// 4. 衝突情報を初期化

	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.move = velocity_;

	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;
	collisionMapInfo.ceiling = false;

	// 5. マップ衝突チェック
	CheckMapCollision(collisionMapInfo);

	// 7. 移動
	worldTransform_.translation_ += collisionMapInfo.move;

	// 8. 天井に当たった場合の速度リセット
	if (collisionMapInfo.ceiling) {
		velocity_.y = 0.0f;
	}

	// 9. 接地判定更新
	UpdateOnGround(collisionMapInfo);




	 if (collisionMapInfo.hitWall) {
		velocity_.x *= -1.0f;

		// 反転した方向に少しだけオフセットして埋まり防止
		worldTransform_.translation_.x += velocity_.x * 0.5f;
	 } else
	 {
		 float direction = (velocity_.x > 0) ? 1.0f : -1.0f;
		 Vector3 forwardFoot = worldTransform_.translation_ + Vector3(direction * kWidth / 2, -kHeight / 2 - 0.5f, 0);
		 MapChipField::IndexSet idx = mapChipField_->GetMapChipIndexSetByPosition(forwardFoot);
		 MapChipType below = mapChipField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex); // -1 は不要かも

		 if (below != MapChipType::kBlock) {
			 velocity_.x *= -1.0f;
			 // オフセットも小さく
			 worldTransform_.translation_.x += velocity_.x * 0.1f;
		 }
	 }


	// 移動距離
	float moveLength = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
	// 半径（width から算出）
	float radius = kWidth / 2.0f;
	// 転がり回転量（rad）
	float rollAngle = moveLength / radius;

	if (velocity_.x < 0) {
		worldTransform_.rotation_.x += rollAngle; 
	} else {
		worldTransform_.rotation_.x -= rollAngle;
	}

}

void Enemy::BehaviorDeathInitialize() { counter_ = 0; }

void Enemy::BehaviorDeathUpdate()
{

	isCollisionDisabled_ = true;

	// タイマーを加算
	counter_ += 1.0f / 60.0f;

	worldTransform_.rotation_.y += 0.6f;
	worldTransform_.rotation_.x = EaseOut(ToRadians(kDefeatedMotionAngleStart), ToRadians(kDefeatedMotionAngleEnd), counter_ / kDefeatedTime);

	if (counter_ >= kDefeatedTime) {
		isDead_ = true;
	}

}
