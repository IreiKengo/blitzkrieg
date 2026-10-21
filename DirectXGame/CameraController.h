#pragma once
#include "KamataEngine.h"

// 前方宣言
class ICameraTarget;

class CameraController {

public:

	//カメラスライド演出
	enum DirectionPhase {
		kNone,//通常追従
		kPanRight,//右へスライド
		kReturnLeft,//左へ戻る
	};


	// 矩形
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	void Initialize(KamataEngine::Camera* camera);

	void Update();

	void Reset();


	// カメラのスタート演出を開始
	void StartStartDirection(float directionY = -1.0f);
	void SetDirectionY(float y) { directionY_ = y; }

	bool IsDirectionFinished() const { return directionPhase_ == DirectionPhase::kNone; }

	void SetTargetOffset(const KamataEngine::Vector3& offset) { targetOffset_ = offset; }
	void SetMovableArea(const Rect& area) {
		if (area.left >= area.right || area.bottom >= area.top) {
			return;
		}
		movableArea_ = area;
	}

	// ICameraTargetの場合
	void SetTarget(const ICameraTarget* target) {
		target_ = target;
		targetTransform_ = nullptr;
	}
	// WorldTransformの場合
	void SetTarget(const KamataEngine::WorldTransform* transform) {
		targetTransform_ = transform;
		target_ = nullptr; // ICameraTarget側をクリア
	}

private:
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	const ICameraTarget* target_ = nullptr;
	const KamataEngine::WorldTransform* targetTransform_ = nullptr;

	// カメラ移動範囲
	Rect movableArea_ = {0, 100, 0, 50};
	// 追従対象とカメラの座標の差（オフセット）
	KamataEngine::Vector3 targetOffset_ = {0, -5.0f, -20.0f};
	// カメラの目標座標
	KamataEngine::Vector3 destination_ = {0.0f, 0.0f, 0.0f};
	// 座標補間割合
	static inline const float kInterpolationRate = 0.1f;

	// 速度掛け率
	static inline const float kVelocityBias = 0.2f;


	//スタート演出用変数
	DirectionPhase directionPhase_ = DirectionPhase::kNone;
	float directionTimer_ = 0.0f;

	 float kPanRightTime = 2.0f;
	 float kReturnLeftTime = 0.8f;

	//カメラスライド演出
	void UpdateStartDirection(const KamataEngine::Vector3& playerDestination);
	 static inline const float kDirectionZ = -45.0f;//演出中に引くZ距離
	 float directionY_ = 10.0f;//演出中のY座標
};