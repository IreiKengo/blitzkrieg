#include "TitleScene.h"
#include <numbers>

using namespace KamataEngine;

void TitleScene::Initialize()
{
	modelTitle_ = Model::CreateFromOBJ("titleFont");
	modelPlayer_ = Model::CreateFromOBJ("player");

	// カメラ初期化
	camera_.Initialize();

	worldTransformTitle_.Initialize();

	worldTransformTitle_.scale_ = 
	{
		kPlayerTitle, kPlayerTitle, kPlayerTitle
	};


	worldTransformPlayer_.Initialize();

	worldTransformPlayer_.scale_ =
	{
		kPlayerScale, kPlayerScale, kPlayerScale
	};

	worldTransformPlayer_.rotation_.y = 0.95f * std::numbers::pi_v<float>;

	worldTransformPlayer_.translation_.x = -2.0f;

	worldTransformPlayer_.translation_.y = -10.0f;


	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeTimer);


	// サウンドデータの読み込み
	BGMHandle = Audio::GetInstance()->LoadWave("BGM/TitleBGM.wav");

	voiceHandle = Audio::GetInstance()->PlayWave(BGMHandle, true, 0.5f);

}

void TitleScene::Update()
{
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished())
		{
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut,1.0f);
			phase_ = Phase::kFadeOut;
			Audio::GetInstance()->StopWave(voiceHandle);
		}

		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	counter_ += 1.0f / 60.0f;
	counter_ = std::fmod(counter_, kTimeTitleMove);

	float angle = counter_ / kTimeTitleMove * 2.0f * std::numbers::pi_v<float>;

	worldTransformTitle_.translation_.y = std::sin(angle) + 10.0f;

	camera_.TransferMatrix();

	WorldTransformUpdate(worldTransformTitle_);
	WorldTransformUpdate(worldTransformPlayer_);
}


void TitleScene::Draw()
{
	
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Model::PreDraw(commandList);

	modelTitle_->Draw(worldTransformTitle_, camera_);
	modelPlayer_->Draw(worldTransformPlayer_, camera_);

	Model::PostDraw();

	
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Draw();
		
		break;
	case Phase::kMain:

		break;
	case Phase::kFadeOut:
		fade_->Draw();
		
		break;
	}
}

TitleScene::~TitleScene() 
{


	delete fade_;


}
