#pragma once
#include <KamataEngine.h>
#include "Math.h"
#include "Fade.h"



class TitleScene
{
public:
	//シーンのフェーズ
	enum class Phase
	{
		kFadeIn,//フェードイン
		kMain,//メイン部
		kFadeOut,//フェードアウト
	};
private:

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransformTitle_;
	KamataEngine::WorldTransform worldTransformPlayer_;
	// モデル
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::Model* modelTitle_ = nullptr;
	// カメラ
	KamataEngine::Camera camera_;

	float counter_ = 0.0f;

	static inline const float kTimeTitleMove = 2.0f;

	//終了フラグ
	bool finished_ = false;
	
	const float kPlayerTitle = 2.0f;

	const float kPlayerScale = 10.0f;

	Fade* fade_ = nullptr;

	//フェード時間
	float fadeTimer = 1.0f;

	//現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	uint32_t BGMHandle = 0;

	uint32_t voiceHandle = 0;


public:

	void Initialize();

	void Update();

	void Draw();

	~TitleScene();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

};
