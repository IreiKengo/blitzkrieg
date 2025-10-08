#pragma once
#include <KamataEngine.h>
#include "Math.h"

using namespace KamataEngine;


class HitEffect
{
private:

	//モデル（借りてくる用）
	static Model* model_;
	static Camera* camera_;

	//円のワールドトランスフォーム
	WorldTransform circleWorldTransform_;

	static const inline uint32_t ellipseNum = 2;

	//楕円のワールドトランスフォーム
	std::array<WorldTransform, ellipseNum> ellipseWorldTransforms_;

	float ellipseWidth =2.0f;
	float ellipseLength = 0.2f;


	enum class State {
		kSpread, // 拡大中
		kFade,   // フェードアウト中
		kDead    // 死亡
	};

	// ゲームの現在フェーズ（変数）
	State state_ = State::kSpread;


	// 拡大アニメーションの時間
	static inline const uint32_t kSpreadTime = 10;

	// フェードアウトアニメーションの時間
	static inline const uint32_t kFadeTime = 20;

	// エフェクトの寿命
	static inline const uint32_t kLifetime = kSpreadTime + kFadeTime;


	// カウンター
	uint32_t counter_ = 0;

public:
	void initialize(const Vector3& position);
	void Update();
	void Draw();
	

	static void SetModel(Model* model) { model_ = model; }
	static void SetCamera(Camera* camera) { camera_ = camera; }

	//インスタンス生成と初期化
	static HitEffect* Create(const Vector3& position);

	//デスフラグの取得
	bool IsDead() const { return state_ == State::kDead; }

};