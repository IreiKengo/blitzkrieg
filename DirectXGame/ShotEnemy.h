#pragma once
#include "Enemy.h"
#include <list>

class EnemyBullet;

class ShotEnemy : public Enemy {
public:
	ShotEnemy() = default;
	~ShotEnemy() = default;
	 void SetBulletInfo(KamataEngine::Model* bulletModel, std::list<EnemyBullet*>* enemyBullets) {
		bulletModel_ = bulletModel;
		enemyBullets_ = enemyBullets;
	 }

protected:
	void UpdateWalk() override;
	void UpdateChase(const Player* player) override;
	void UpdateAttack() override;

private:

	float GetSearchDistanceX() const override { return 10.0f; };

	// 攻撃フェーズ（型）
	enum class AttackPhase {
		kUnknown = -1, // 無効な状態
		kReservoir,    // 溜め
		kAction,       // 撃つ
		kRecovery,     // 余韻

	};

	AttackPhase attackPhase_ = AttackPhase::kUnknown;

	KamataEngine::Vector3 shotDir_ = {0.0f, 0.0f, 0.0f}; // 発射時の固定方向

	const float kShotDistanceMin = 3.5f; // これより離れていたら遠距離攻撃
	const float kShotDistanceMax = 10.0f; // 届く最大の距離

	std::list<EnemyBullet*>* enemyBullets_;
	KamataEngine::Model* bulletModel_ = nullptr;

	void FireBullet();
};
