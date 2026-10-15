#pragma once
#include "Math.h"
#include <KamataEngine.h>

class Fade;

class GameOverScene {
public:
	// シーンのフェーズ
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン部
		kFadeOut, // フェードアウト
	};

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransformGameOver_;

	// モデル

	KamataEngine::Model* modelGameOver_ = nullptr;
	// カメラ
	KamataEngine::Camera camera_;

	float counter_ = 0.0f;

	static inline const float kTimeGameOverMove = 2.0f;

	// 終了フラグ
	bool finished_ = false;

	const float kPlayerGameOver = 2.0f;

	const float kPlayerScale = 10.0f;

	Fade* fade_ = nullptr;

	// フェード時間
	float fadeTimer = 1.0f;

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	uint32_t BGMHandle = 0;

	uint32_t voiceHandle = 0;

public:
	void Initialize();

	void Update();

	void Draw();

	~GameOverScene();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }
};