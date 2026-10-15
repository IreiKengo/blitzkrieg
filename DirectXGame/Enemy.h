#pragma once
#include "KamataEngine.h"
#include "Math.h"

class GameScene;
class Player;
class EnemyBullet;
class MapChipField;

class Enemy {

public:
	// 敵の状態
	enum class State {
		kWalk,   // 巡回
		kChase,  // 追従
		kAttack, // 攻撃

	};

	// 攻撃の種類
	enum class AttackType {
		kNone,
		kCharge, // 突進攻撃
		kShot, // 遠距離攻撃
	};

	// 攻撃フェーズ（型）
	enum class AttackPhase {
		kUnknown = -1, // 無効な状態
		kReservoir,    // 溜め
		kAction,       // 突進
		kRecovery,     // 余韻

	};

	// 当たり判定用の角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool ceiling = false;                            // 天井衝突
		bool landing = false;                            // 着地
		bool hitWall = false;                            // 壁接触
		KamataEngine::Vector3 move = {0.0f, 0.0f, 0.0f}; // 移動量
	};

private:
	// 振るまい
	enum class Behavior {
		kUnknown = -1, // 無効な状態
		kRoll,         // 転がる
		kDeath,        // デス演出
	};

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 歩行の速度
	static inline const float kWalkSpeed = 0.05f;
	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 最初の角度
	static inline const float kWalkMotionAngleStart = -10.0f;
	// 最後の角度
	static inline const float kWalkMotionAngleEnd = 40.0f;
	// アニメーションの周期となる時間
	static inline const float kWalkMotionTime = 1.0f;
	// 経過時間
	float walkTimer_ = 0.0f;
	float param = 0.0f;
	float degree = 0.0f;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.15f;

	/// 接地・物理関連///

	// 微小な数値
	static inline const float kGroundSearchHeight = 0.12f;
	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.02f;
	static inline const float kAttenuationWall = 0.03f;
	// 重力加速度(下方向)
	static inline const float kGravityAcceleration = 0.8f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.4f;

	// 接地状態フラグ
	bool onGround_ = true;
	// デスフラグ
	bool isDead_ = false;
	bool isCollisionDisabled_ = false;

	// 振るまい
	Behavior behavior_ = Behavior::kRoll;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	static inline const float kDefeatedMotionAngleStart = 0.0f;
	static inline const float kDefeatedMotionAngleEnd = -60.0f;
	float counter_ = 0.0f; // カウンター
	static inline const float kDefeatedTime = 0.6f;

	GameScene* gameScene_ = nullptr;
	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	/// ステート管理 ///
	State state_ = State::kWalk;
	AttackType attackType_ = AttackType::kNone;
	AttackPhase attackPhase_ = AttackPhase::kUnknown;

	float stateTimer_ = 0.0f;                // 汎用タイマー（秒単位）
	KamataEngine::Vector3 chargeDir_ = {0.0f, 0.0f, 0.0f}; // 突進時の固定方向

	// 調整パラメータ
	float chargeSpeed_ = 0.35f;   // 突進速度
	float chargeRangeSq_ = 25.0f; // 突進を開始する距離の2乗

	// 遠距離攻撃のパラメータ
	const float kShotDistanceMin = 3.5f; // これより離れていたら遠距離攻撃
	const float kShotDistanceMax = 7.0f; // 届く最大の距離

	std::list<EnemyBullet*>* enemyBullets_;
	KamataEngine::Model* bulletModel_ = nullptr;

	float attackCooldown_ = 0.0f;


	void BehaviorRollInitialize();
	void BehaviorRollUpdate(const Player* player);
	void BehaviorDeathInitialize();
	void BehaviorDeathUpdate();

	//行動処理
	void UpdateWalk();
	void UpdateChase(const Player* player);
	void UpdateAttack();
	void DoChargeAttack();
	// 遠距離攻撃用の処理
	void DoShotAttack();
	void FireBullet(); // 弾を生成する関数

	// マップ衝突系
	void CheckMapCollision(CollisionMapInfo& info);
	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);
	void UpdateOnGround(const CollisionMapInfo& info);

public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update(const Player* player);
	void Draw();

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition() const;
	// AABBを取得
	AABB GetAABB() const;
	// 衝突応答
	void OnCollision(const Player* player);
	// デスフラグのgetter
	bool IsDead() const { return isDead_; }
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }
	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	void SetBulletInfo(KamataEngine::Model* bulletModel,std::list<EnemyBullet*>* enemyBullets) {
		bulletModel_ = bulletModel;
		enemyBullets_ = enemyBullets;
	}

	// 角の座標をまとめて取得
	std::array<KamataEngine::Vector3, kNumCorner> GetCorners(const KamataEngine::Vector3& center) const;
	
};