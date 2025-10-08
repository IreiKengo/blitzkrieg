#pragma once
#include <KamataEngine.h>

using namespace KamataEngine;

class Fade 
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

	Sprite* sprite_ = nullptr;
	//現在のフェードの状態
	Status status_ = Status::None;
	//フェードの持続時間
	float duration_ = 0.0f;
	//経過時間カウンター
	float counter_ = 0.0f;

public:
	
	void Initialize();
	void Update();
	void Draw();
	//フェード開始
	void Start(Status status, float duration);
	//フェード停止
	void Stop();
	bool IsFinished();
	//フェード終了判定
	bool IsFinished() const;
};
