#pragma once
#include <KamataEngine.h>
#include "ITransition.h"

class FlashTransition : public ITransition
{
public:

	// 画面遷移の状態
	enum class TransitionStatus {
		None,        // 遷移なし
		Slash,      // 1.閃光（斬撃線）が走る
		Sliding, // 2.画面が2つに分かれて外側へスライドする
		Finished     // 遷移完了
	};

	~FlashTransition();
	void Initialize()override;
	void Update()override;
	void Draw()override;

	// フェード開始
	void Start(TransitionStatus status, float duration);
	void StartIn(float duration = 1.0f) override;// { Start(TransitionStatus::SlashEffect, duration); }
	void StartOut(float duration = 1.0f) override; // { Start(TransitionStatus::FadeOut, duration); }
	// フェード停止
	void Stop() override;
	// フェード終了判定
	bool IsFinished() const override;

	private:



};