#pragma once
#include <KamataEngine.h>
#include "ITransition.h"


class Fade : public ITransition
{
public:
	// フェードの状態
	enum class Status 
	{
		None,//フェードなし
		FadeIn,//フェードイン中
		FadeOut,//フェードアウト中
	};

private:

	KamataEngine::Sprite* sprite_ = nullptr;
	//現在のフェードの状態
	Status status_ = Status::None;
	//フェードの持続時間
	float duration_ = 0.0f;
	//経過時間カウンター
	float counter_ = 0.0f;

public:
	~Fade() override;
	
	void Initialize()override;
	void Update() override;
	void Draw() override;
	//フェード開始
	void Start(Status status, float duration);
	void StartIn(float duration = 1.0f) override { Start(Status::FadeIn, duration); }
	void StartOut(float duration = 1.0f) override { Start(Status::FadeOut, duration); }
	//フェード停止
	void Stop() override;
	//フェード終了判定
	bool IsFinished() const override;
};
