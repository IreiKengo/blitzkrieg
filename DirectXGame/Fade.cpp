#include "Fade.h"
#include <algorithm>



using namespace KamataEngine;

void Fade::Initialize() 
{
	
	sprite_ = Sprite::Create(0, Vector2{});
	sprite_->SetSize(Vector2(WinApp::kWindowWidth, WinApp::kWindowHeight));
	sprite_->SetColor(Vector4(0, 0, 0, 1));

}

void Fade::Update()
{

	//フェード状態による分岐
	switch (status_) {
	case Status::None:
		// 何もしない
		break;
	case Status::FadeIn:

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード持続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、経過時間がフェード持続時間に近づくほどアルファ値を大きくする
		sprite_->SetColor(Vector4(0, 0, 0, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));

		break;
	case Status::FadeOut:

		//1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		//フェード持続時間に達したら打ち止め
		if (counter_ >= duration_)
		{
			counter_ = duration_;
			
		}
		//0.0fから1.0fの間で、経過時間がフェード持続時間に近づくほどアルファ値を大きくする
		sprite_->SetColor(Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));



		break;
	}


}

void Fade::Draw() 
{ 
	// DirectX共通インスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	if (status_ == Status::None)
	{
		return;
	}
	sprite_->Draw();


	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration)
{


	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;


}

void Fade::Stop()
{

	status_ = Status::None;

}

bool Fade::IsFinished()const  {
	//フェード状態による分岐
	switch (status_) {
	case Status::FadeIn:
	case Status::FadeOut:
		if (counter_ >= duration_)
		{
			return true;
		} else
		{
			return false;
		}
	}
	return true;
}