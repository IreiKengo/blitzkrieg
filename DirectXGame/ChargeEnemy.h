#pragma once
#include "Enemy.h"

class ChargeEnemy : public Enemy {
public:
	ChargeEnemy() = default;
	~ChargeEnemy() override = default;

protected:

	void UpdateAttack() override;
	void UpdateWalk() override;
	void UpdateChase(const Player* player) override;

private:

	// 攻撃フェーズ（型）
	enum class AttackPhase {
		kUnknown = -1, // 無効な状態
		kReservoir,    // 溜め
		kAction,       // 突進
		kRecovery,     // 余韻

	};
	AttackPhase attackPhase_ = AttackPhase::kUnknown;

	KamataEngine::Vector3 chargeDir_ = {0.0f, 0.0f, 0.0f}; // 突進時の固定方向

	// 調整パラメータ
	float chargeSpeed_ = 0.35f;   // 突進速度
	float chargeRangeSq_ = 25.0f; // 突進を開始する距離の2乗
};
