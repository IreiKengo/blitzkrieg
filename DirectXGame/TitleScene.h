#pragma once
#include <KamataEngine.h>
#include "Math.h"
#include "Fade.h"

using namespace KamataEngine;

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
	WorldTransform worldTransformTitle_;
	WorldTransform worldTransformPlayer_;
	// モデル
	Model* modelPlayer_ = nullptr;
	Model* modelTitle_ = nullptr;
	// カメラ
	Camera camera_;

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

public:

	void Initialize();

	void Update();

	void Draw();

	~TitleScene();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

};
