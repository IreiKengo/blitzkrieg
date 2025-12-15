#pragma once
#include "KamataEngine.h"
#include "Math.h"
#include <algorithm>
#include <numbers>
#include <vector>
#include <array>
#include "MapChipField.h"

class MapChipField;
class Enemy;
class DeathParticles;


class Player {
public:

	 // 基本的な列挙型・構造体


	// 左右向き
	enum class LRDirection {
		kRight,
		kLeft,
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
		bool ceiling = false; // 天井衝突
		bool landing = false; // 着地
		bool hitWall = false; // 壁接触
		KamataEngine::Vector3 move; // 移動量
	};

	  // 行動状態
	enum class Behavior
	{
		kUnknown = -1,
		kRoot,//通常状態
		kAir,    // 空中
		kWall,   // 壁
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
	


	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* modelAttack, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	// 更新
	void Update();
	// 描画
	void Draw();


	 // ゲッター
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }
	KamataEngine::Vector3 GetWorldPosition() const;
	bool IsDead() const { return isDead_; }
	bool IsAttack() const { return behavior_ == Behavior::kAttack && attackPhase_ == AttackPhase::kAction; }

	// セッター
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// 衝突応答
	void OnCollision(const Enemy* enemy);

	// AABB計算
	AABB GetAABB();

private :

	 //  モデル / トランスフォーム / カメラ

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// エフェクト用のデータ
	KamataEngine::Model* modelAttack_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;



	// 物理・移動パラメータ

	KamataEngine::Vector3 velocity_ = {};

	// 地上パラメータ

	// 加速
	static inline const float kAcceleration = 0.06f;
	//非入力時の減速率
	const float kDeceleration = 0.12f;
	// 反対入力時の急ブレーキ
	const float kBrakePower = 0.5f;
	// 最大速度制限
	static inline const float kLimitRunSpeed = 0.3f;

	

	 // 空中パラメータ

	const float kAirBrakePower = 0.7f;
	static inline const float kAirAcceleration = 0.02f;
	const float kAirDeceleration = 0.02f;
	// 最大速度制限
	static inline const float kLimitAirSpeed = 0.22f;


	// 重力関連

	// 重力加速度(下方向)
	static inline const float kGravityAcceleration = 0.8f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.4f;
	// ジャンプ初速(上方向)
	static inline const float kJumpAcceleration = 18.0f;


	 // 壁関連

	// 壁キック時の速度
	static inline const KamataEngine::Vector3 wallKickSpeed = {0.15f, 0.35f, 0.0f};
	// 壁接触時の最大落下速度（下方向）
	static inline const float kWallSlideSpeed = 0.08f;
	// 壁ジャンプが可能かのフラグ
	bool canWallJump_ = false;
	//壁ジャンプタイマー
	float wallJumpBufferTimer_ = 0.0f;
	const float kWallJumpBufferTime = 1.0f;
	// 壁ジャンプ後の操作不能時間
	float wallJumpLockTimer_ = 0.0f; 

	// 状態フラグ / 回転補助

	// 接地状態フラグ
	bool onGround_ = true;
	// 左右
	LRDirection lrDirection_ = LRDirection::kRight;
	// 旋回開始時の速度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間<秒>
	static inline const float kTimeTurn = 0.2f;
	static inline const float kTurnSpeed = 0.2f;
	


	 // 当たり判定 / マップ

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;
	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.04f;
	//微小な数値
	static inline const float kGroundSearchHeight = 0.12f;
	//着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.02f;
	static inline const float kAttenuationWall = 0.03f;


	 // 攻撃関連

	//攻撃速度
	static inline const float attackSpeed = 0.02f;
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
	KamataEngine::WorldTransform worldTransformAttack_;


	 // 行動制御

	//振るまい
	Behavior behavior_ = Behavior::kRoot;
	//次の振るまいリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;


	float justJumpedTimer_ = 0.0f;
	bool justJumped_ = false;

	 // 状態フラグ

	//デスフラグ
	bool isDead_ = false;


	  // 内部処理系（入力や補助）

	// 移動入力
	//地上
	void InputMoveGround();
	//空中
	void InputMoveAir();
	

	void Turn();


	 // 接地・壁の更新
	void UpdateOnGround(const CollisionMapInfo& info);
	void UpdateOnWall(const CollisionMapInfo& info);

	// 角の座標をまとめて取得
	std::array<KamataEngine::Vector3, kNumCorner> GetCorners(const KamataEngine::Vector3& center) const;

	

	   // マップ衝突系
	void CheckMapCollision(CollisionMapInfo& info);
	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);

	
	
	 // 行動初期化・更新
	void BehaviorRootInitialize();
	void BehaviorRootUpdate();
	void BehaviorAirInitialize();
	void BehaviorAirUpdate();
	void BehaviorWallInitialize();
	void BehaviorWallUpdate();
	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();




};