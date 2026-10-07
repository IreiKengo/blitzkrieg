	#define NOMINMAX
	#include "Enemy.h"
	#include "GameScene.h"
	#include "Player.h"
	#include <algorithm>
	#include <cassert>
	#include <numbers>

	using namespace KamataEngine;

	void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

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

		state_ = State::kWalk;
		attackType_ = AttackType::kNone;
		attackPhase_ = AttackPhase::kUnknown;
		stateTimer_ = 0.0f;

		// 初期接地判定
		CollisionMapInfo info = {};
		info.move = {};
		UpdateOnGround(info); // 初期座標で接地フラグを確認
		info.landing = false;
	}

	void Enemy::Update(const Player* player) {

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

			BehaviorRollUpdate(player);

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
		Vector3 worldPos = {0.0f, 0.0f, 0.0f};
		// ワールド行列の平行移動成分を取得(ワールド座標)
		worldPos.x = worldTransform_.matWorld_.m[3][0];
		worldPos.y = worldTransform_.matWorld_.m[3][1];
		worldPos.z = worldTransform_.matWorld_.m[3][2];
		return worldPos;
	}

	AABB Enemy::GetAABB() const {
		Vector3 worldPos = GetWorldPosition();

		AABB aabb = {
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f}
		};

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
	void Enemy::BehaviorRollUpdate(const Player* player) {
		float kDeltaTime = 1.0f / 60.0f;
		stateTimer_ += kDeltaTime;

		if (attackCooldown_ > 0.0f) {
			attackCooldown_ -= kDeltaTime;
		}

		// 1.重力処理
		if (!onGround_) {
			velocity_.y -= kGravityAcceleration / 60.0f;

			velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
		}

		// 2.衝突情報を初期化
		CollisionMapInfo collisionMapInfo;
		collisionMapInfo.move = velocity_;
		collisionMapInfo.landing = false;
		collisionMapInfo.hitWall = false;
		collisionMapInfo.ceiling = false;

		// 3. ステート更新
		switch (state_) {
		case Enemy::State::kWalk:
			UpdateWalk();
			// プレイヤーを見つけたとき
			if (player) {
				Vector3 playerPosition = player->GetWorldPosition();
				Vector3 d = playerPosition - worldTransform_.translation_;
				float absDx = std::abs(d.x);
				float absDy = std::abs(d.y);

				if (absDx < 6.0f && absDy < 1.0f) {
					state_ = State::kChase;
					stateTimer_ = 0.0f;
				}
			}
			break;
		case Enemy::State::kChase:
			UpdateChase(player);
			break;
		case Enemy::State::kAttack:
			UpdateAttack();
			if (attackType_ == AttackType::kCharge && attackPhase_ == AttackPhase::kAction) {
				velocity_ = chargeDir_ * chargeSpeed_;
			}
			break;
		}

		// 4. マップ衝突チェック
		collisionMapInfo.move = velocity_;
		CheckMapCollision(collisionMapInfo);

		if (collisionMapInfo.hitWall) {
			// 1. 突進中に壁に当たったら後隙へ強制移行
			if (state_ == State::kAttack && attackPhase_ == AttackPhase::kAction) {
				attackPhase_ = AttackPhase::kRecovery;
				stateTimer_ = 0.0f;
				velocity_ = {0.0f, 0.0f, 0.0f}; // その場で停止してスキを作る
			}
			// 2. 通常の巡回・追従中の壁衝突なら反転して巡回に戻す
			else {
				if (state_ == State::kChase) {
					state_ = State::kWalk;
				}
				stateTimer_ = 0.0f;
				velocity_.x *= -1.0f;
			}
		}

		worldTransform_.translation_ += collisionMapInfo.move;

		// 天井に当たった場合の速度リセット
		if (collisionMapInfo.ceiling) {
			velocity_.y = 0.0f;
		}

		// 接地判定更新
		UpdateOnGround(collisionMapInfo);

		// 5. 向きと見た目の回転（突進中でなければ移動方向にモデルを向ける）
		if (!(state_ == State::kAttack && attackPhase_ == AttackPhase::kAction)) {
			if (velocity_.x > 0.01f) {
				worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
			} else if (velocity_.x < -0.01f) {
				worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;
			}
		}

		// 転がり回転計算
		float moveLength = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
		float radius = kWidth / 2.0f;
		float rollAngle = moveLength / radius;

		if (std::abs(velocity_.x) > 0.01f) {
		    worldTransform_.rotation_.x += rollAngle;
	    }
	}

	void Enemy::BehaviorDeathInitialize() { counter_ = 0; }

	void Enemy::BehaviorDeathUpdate() {

		isCollisionDisabled_ = true;

		// タイマーを加算
		counter_ += 1.0f / 60.0f;

		worldTransform_.rotation_.y += 0.6f;
		worldTransform_.rotation_.x = EaseOut(ToRadians(kDefeatedMotionAngleStart), ToRadians(kDefeatedMotionAngleEnd), counter_ / kDefeatedTime);

		if (counter_ >= kDefeatedTime) {
			isDead_ = true;
		}
	}

	void Enemy::UpdateWalk() {

		// 速度の向き（正または負）を維持しつつ歩行速度をセット
		float currentDir = (velocity_.x >= 0.0f) ? 1.0f : -1.0f;
		velocity_.x = currentDir * kWalkSpeed;

		// 崖っぷち判定
		float direction = (velocity_.x > 0.0f) ? 1.0f : -1.0f;

		Vector3 forwardFoot = worldTransform_.translation_ + Vector3(direction * (kWidth / 2.0f + 0.1f), -kHeight / 2.0f - 0.2f, 0.0f);
		MapChipField::IndexSet idx = mapChipField_->GetMapChipIndexSetByPosition(forwardFoot);
		MapChipType below = mapChipField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);

		// 足元がブロックでなければ反転
		if (below != MapChipType::kBlock) {
			velocity_.x *= -1.0f;
		
		}
	}

	void Enemy::UpdateChase(const Player* player) {

		if (!player)
			return;

		Vector3 pPos = player->GetWorldPosition();
		Vector3 d = pPos - worldTransform_.translation_;

		float absDx = std::abs(d.x);
		float absDy = std::abs(d.y);

		// 追従をあきらめる距離
		const float kGiveUpDistanceX = 8.0f;
		const float kGiveUpDistanceY = 2.5f;

		if (absDx > kGiveUpDistanceX || absDy > kGiveUpDistanceY) {
			state_ = State::kWalk; // 巡回状態に戻る
			stateTimer_ = 0.0f;
			return;
		}

	
		const float kMaxHeightDiff = 0.4f;

		// 1. [近距離]突進攻撃状態
		if (attackCooldown_ <= 0.0f && absDy <= kMaxHeightDiff && absDx <= 3.0f && absDx > 0.5f) {
			state_ = State::kAttack;
			attackType_ = AttackType::kCharge;
			attackPhase_ = AttackPhase::kReservoir; // 溜め
			stateTimer_ = 0.0f;
			velocity_ = {0.0f, 0.0f, 0.0f}; // その場で停止

			// 突進方向を確定
			float dirX = (d.x >= 0.0f) ? 1.0f : -1.0f;
			chargeDir_ = {dirX, 0.0f, 0.0f};

			return;
		}

		// 2. [遠距離]
		if (attackCooldown_ <= 0.0f && absDy <= kMaxHeightDiff && absDx > 6.0f && absDx <= 10.0f) {
			state_ = State::kAttack;
			attackType_ = AttackType::kShot;
			attackPhase_ = AttackPhase::kReservoir; // 溜め（構えモーション等）
			stateTimer_ = 0.0f;
			velocity_ = {0.0f, 0.0f, 0.0f};
			// プレイヤーの方向に向く
			float dirX = (d.x >= 0.0f) ? 1.0f : -1.0f;
			chargeDir_ = {dirX, 0.0f, 0.0f};
			return;
		}

		// 2. 通常追従（プレイヤーの方向へ移動）
		float targetDir = (d.x >= 0.0f ? 1.0f : -1.0f);

		Vector3 forwardFoot = worldTransform_.translation_ + Vector3(targetDir * (kWidth / 2.0f + 0.1f), -kHeight / 2.0f - 0.2f, 0.0f);
		MapChipField::IndexSet idx = mapChipField_->GetMapChipIndexSetByPosition(forwardFoot);
		MapChipType below = mapChipField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);

		// 足元がブロックでないなら、追従して落下するのを防ぐために停止
		if (below != MapChipType::kBlock) {
			velocity_.x = 0.0f; 
		                    
		} else {
			velocity_.x = targetDir * kWalkSpeed;
		}
	}

	void Enemy::UpdateAttack() {

		switch (attackType_) {
		case AttackType::kCharge:
			DoChargeAttack();
			break;
		case AttackType::kShot:
			DoShotAttack();
			break;
		default:
			break;
		}
	}

	void Enemy::DoChargeAttack() {

		switch (attackPhase_) {
		case AttackPhase::kReservoir:       // 【溜め】
			velocity_ = {0.0f, 0.0f, 0.0f}; // 動かない
			// 0.4秒間溜めたら突進へ
			if (stateTimer_ >= 0.4f) {
				attackPhase_ = AttackPhase::kAction;
				stateTimer_ = 0.0f;
			}
			break;

		case AttackPhase::kAction: // 【突進】
		{
			float direction = (chargeDir_.x > 0.0f) ? 1.0f : -1.0f;
			// 突進方向の前方足元
			Vector3 forwardFoot = worldTransform_.translation_ + Vector3(direction * (kWidth / 2.0f + 0.1f), -kHeight / 2.0f - 0.2f, 0.0f);
			MapChipField::IndexSet idx = mapChipField_->GetMapChipIndexSetByPosition(forwardFoot);
			MapChipType below = mapChipField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);

			// 下がブロックでない（崖になっている）なら突進中断！
			if (below != MapChipType::kBlock) {
				attackPhase_ = AttackPhase::kRecovery;
				stateTimer_ = 0.0f;
				velocity_ = {0.0f, 0.0f, 0.0f};
				break;
			}
		}
			// 0.35秒間突進を実行（速度は BehaviorRollUpdate 内で設定）
			if (stateTimer_ >= 0.35f) {
				attackPhase_ = AttackPhase::kRecovery;
				stateTimer_ = 0.0f;
				velocity_ = {0.0f, 0.0f, 0.0f}; // 突進終了時に減速
			}
			break;

		case AttackPhase::kRecovery:        // 【余韻（後隙）】
			velocity_ = {0.0f, 0.0f, 0.0f}; // 動きを止めてスキを作る
			// 0.6秒後に追従に戻る
			if (stateTimer_ >= 0.6f) {
				state_ = State::kChase;
				attackType_ = AttackType::kNone;
				attackPhase_ = AttackPhase::kUnknown;
				stateTimer_ = 0.0f;
				attackCooldown_ = 1.5f;
			}
			break;

		case AttackPhase::kUnknown:
			break;
		}
	}

	void Enemy::DoShotAttack() {

		switch (attackPhase_) {
		case AttackPhase::kReservoir:
			velocity_ = {0.0f, 0.0f, 0.0f};
			if (stateTimer_ >= 0.3f) {
				attackPhase_ = AttackPhase::kAction;
				stateTimer_ = 0.0f;

				// 弾を発射
				FireBullet();
			}
			break;

		case AttackPhase::kAction:
			if (stateTimer_ >= 0.1f) {
				attackPhase_ = AttackPhase::kRecovery;
				stateTimer_ = 0.0f;
			}
			break;

		case AttackPhase::kRecovery:
			velocity_ = {0.0f, 0.0f, 0.0f};
			if (stateTimer_ >= 0.5f) {
				state_ = State::kChase;
				attackType_ = AttackType::kNone;
				attackPhase_ = AttackPhase::kUnknown;
				stateTimer_ = 0.0f;
				attackCooldown_ = 2.0f;
			}
			break;

		case AttackPhase::kUnknown:
			break;
		}
	}

	void Enemy::FireBullet() {

		//// 弾の飛ぶスピード
		const float kBulletSpeed = 0.2f;

		//// 敵の向いている方向（chargeDir_）に合わせた速度ベクトル
		Vector3 bulletVelocity = chargeDir_ * kBulletSpeed;

		Vector3 spawnPosition = worldTransform_.translation_ + chargeDir_ * 0.5f;

		if (enemyBullets_) {
			EnemyBullet* newBullet = new EnemyBullet();
			newBullet->SetMapChipField(mapChipField_);
			newBullet->Initialize(bulletModel_, camera_, spawnPosition, bulletVelocity);
			enemyBullets_->push_back(newBullet);
		}
	}
