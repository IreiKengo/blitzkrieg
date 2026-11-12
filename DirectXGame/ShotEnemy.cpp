#include "ShotEnemy.h"
#include "EnemyBullet.h"
#include "MapChipField.h"
#include "Player.h"
#include <cmath>

using namespace KamataEngine;

void ShotEnemy::UpdateWalk()
{

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

void ShotEnemy::UpdateChase(const Player* player)
{

	if (!player)
		return;

	Vector3 pPos = player->GetWorldPosition();
	Vector3 d = pPos - worldTransform_.translation_;

	float absDx = std::abs(d.x);
	float absDy = std::abs(d.y);

	// 追従をあきらめる距離
	const float kGiveUpDistanceX = 11.0f;
	const float kGiveUpDistanceY = 2.5f;

	if (absDx > kGiveUpDistanceX || absDy > kGiveUpDistanceY) {
		state_ = State::kWalk; // 巡回状態に戻る
		stateTimer_ = 0.0f;
		return;
	}

	const float kMaxHeightDiff = 0.4f;

	if (attackCooldown_ <= 0.0f && absDy <= kMaxHeightDiff && absDx >= kShotDistanceMin && absDx <= kShotDistanceMax) {
		state_ = State::kAttack;
		attackPhase_ = AttackPhase::kReservoir;
		stateTimer_ = 0.0f;
		velocity_ = {0.0f, 0.0f, 0.0f};
		// プレイヤーの方向に向く
		float dirX = (d.x >= 0.0f) ? 1.0f : -1.0f;
		shotDir_ = {dirX, 0.0f, 0.0f};
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

void ShotEnemy::UpdateAttack()
{

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
			attackPhase_ = AttackPhase::kUnknown;
			stateTimer_ = 0.0f;
			attackCooldown_ = 2.0f;
		}
		break;

	case AttackPhase::kUnknown:
		break;
	}

}

void ShotEnemy::FireBullet()
{

	//弾の飛ぶスピード
	const float kBulletSpeed = 0.2f;

	//向いている方に合わせた速度ベクトル
	Vector3 bulletVelocity = shotDir_ * kBulletSpeed;

	Vector3 spawnPosition = worldTransform_.translation_ + shotDir_ * 0.5f;

	if (enemyBullets_) {
		EnemyBullet* newBullet = new EnemyBullet();
		newBullet->SetMapChipField(mapChipField_);
		newBullet->Initialize(bulletModel_, camera_, spawnPosition, bulletVelocity);
		enemyBullets_->push_back(newBullet);
	}

}
