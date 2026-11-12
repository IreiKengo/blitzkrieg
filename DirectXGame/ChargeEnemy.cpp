#include "ChargeEnemy.h"
#include "MapChipField.h"
#include "Player.h"
#include <cmath>

using namespace KamataEngine;

void ChargeEnemy::UpdateAttack() 
{

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
		velocity_ = chargeDir_ * chargeSpeed_;
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
			attackPhase_ = AttackPhase::kUnknown;
			stateTimer_ = 0.0f;
			attackCooldown_ = 1.5f;
		}
		break;

	case AttackPhase::kUnknown:
		break;
	}

}

void ChargeEnemy::UpdateWalk() 
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

void ChargeEnemy::UpdateChase(const Player* player) 
{

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

	if (attackCooldown_ <= 0.0f && absDy <= kMaxHeightDiff && absDx <= 6.0f && absDx > 0.5f) {
		state_ = State::kAttack;
		attackPhase_ = AttackPhase::kReservoir; // 溜め
		stateTimer_ = 0.0f;
		velocity_ = {0.0f, 0.0f, 0.0f}; // その場で停止

		// 突進方向を確定
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
