
#pragma once
#include "KamataEngine.h"
#include "Math.h"
#include <algorithm>
#include <numbers>
#include <vector>
#include "MapChipField.h"

class MapChipField;
class Enemy;
class DeathParticles;


using namespace KamataEngine;

class Player {
public:

	// 左右
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// 角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool ceiling = false; // 天井衝突
		bool landing = false; // 着地
		bool hitWall = false; // 壁接触
		Vector3 move;         // 移動量
	};

	//振るまい
	enum class Behavior
	{
		kUnknown = -1,
		kRoot,//通常状態
		kAttack,//攻撃中
	};

	//攻撃フェーズ（型）
	enum class AttackPhase 
	{
		kUnknown = -1, // 無効な状態
		kReservoir, // 溜め
		kAction,       // 突進
		kRecovery,     // 余韻

	};
	
	private :
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// 加速
	Vector3 velocity_ = {};
	static inline const float kAcceleration = 0.02f;
	// 速度減速率
	static inline const float kAttenuation = 0.05f;
	// 最大速度制限
	static inline const float kLimitRunSpeed = 0.2f;
	// 左右
	LRDirection lrDirection_ = LRDirection::kRight;
	// 旋回開始時の速度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;
	// 接地状態フラグ
	bool onGround_ = true;
	// 重力加速度(下方向)
	static inline const float kGravityAcceleration = 0.98f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.5f;
	// ジャンプ初速(上方向)
	static inline const float kJumpAcceleration = 20.0f;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.0f;
	static inline const float kHeight = 0.9f;

	// 移動入力
	void InputMove();

	

	static inline const float kBlank = 0.04f;

	//着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.01f;
	static inline const float kAttenuationWall = 0.0f;
	//微小な数値
	static inline const float kGroundSearchHeight = 0.06f;

	//デスフラグ
	bool isDead_ = false;

	//攻撃速度
	static inline const float attackSpeed = 0.02f;
	
	//振るまい
	Behavior behavior_ = Behavior::kRoot;
	//次の振るまいリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;

	//攻撃ギミックの経過時間カウンター
	float attackParameter_ = 0.0f;

	//現在の攻撃フェーズ（変数）
	AttackPhase attackPhase_ = AttackPhase::kUnknown;

	//溜め動作時間
	static inline const uint32_t reservoirTime = 8;
	// 突進動作時間
	static inline const uint32_t ActionTime = 5;
	//余韻動作時間
	static inline const uint32_t recoveryTime = 12;

	//エフェクト用のデータ
	Model* modelAttack_ = nullptr;
	WorldTransform worldTransformAttack_;


public:
	// 初期化
	void Initialize(Model* model, Model* modelAttack, Camera* camera, const Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	const WorldTransform& GetWorldTransform() const { return worldTransform_; }

	const Vector3& GetVelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	void CheckMapCollision(CollisionMapInfo& info);
	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);
	Vector3 CornerPosition(const Vector3& center, Corner corner);

	//接地状態の切り替え処理
	void UpdateOnGround(const CollisionMapInfo& info);
	// 壁接触している場合の処理
	void UpdateOnWall(const CollisionMapInfo& info);


	//AABBを取得
	AABB GetAABB();

	//衝突応答
	void OnCollision(const Enemy* enemy);

	//デスフラグのgetter
	bool IsDead() const { return isDead_; }

	// ワールド座標を取得
	Vector3 GetWorldPosition()const;

	//通常行動更新
	void BehaviorRootUpdate();
	//攻撃行動更新
	void BehaviorAttackUpdate();
	//通常行動初期化
	void BehaviorRootInitialize();
	//攻撃行動初期化
	void BehaviorAttackInitialize();

	bool IsAttack() const { return behavior_ == Behavior::kAttack && attackPhase_ == AttackPhase::kAction; }

};