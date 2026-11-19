#pragma once
#include <KamataEngine.h>
#include "IScene.h"


class Fade;
class Skydome;

class ClearScene : public IScene{
public:
	// シーンのフェーズ
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン部
		kFadeOut, // フェードアウト
	};

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransformClear_;
	// カメラ
	KamataEngine::Camera camera_;

	KamataEngine::Sprite* UISpaceToStart_ = nullptr;
	float blinkTimer_ = 0.0f; // 点滅用のタイマー

	float counter_ = 0.0f;

	static inline const float kTimeClearMove = 2.0f;


	const float kPlayerClear = 2.0f;

	const float kPlayerScale = 10.0f;

	Fade* fade_ = nullptr;

	// フェード時間
	float fadeTimer = 1.0f;

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	// 天球
	Skydome* skydome_ = nullptr;
	KamataEngine::WorldTransform playerTransform_;

	uint32_t BGMHandle = 0;

	uint32_t voiceHandle = 0;

public:
	void Initialize()override;

	void Update() override;

	void Draw() override;

	~ClearScene();

	
};
