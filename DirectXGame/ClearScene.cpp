#include "ClearScene.h"
#include <numbers>
#include "Math.h"
#include "Fade.h"
#include "Skydome.h"
#include "ModelManager.h"

using namespace KamataEngine;

void ClearScene::Initialize() {
	ModelManager::GetInstance()->GetModel("Clear");
	ModelManager::GetInstance()->GetModel("player", true);

	UISpaceToStart_ = Sprite::Create(TextureManager::Load("UI/spaceToStart.png"), {500.0f, 550.0f});

	// カメラ初期化
	camera_.Initialize();

	worldTransformClear_.Initialize();

	worldTransformClear_.scale_ = {kPlayerClear, kPlayerClear, kPlayerClear};

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize(ModelManager::GetInstance()->GetModel("skydome", true), &camera_);

	playerTransform_.Initialize();

	playerTransform_.translation_.y = -0.5f;
	playerTransform_.translation_.z = -45.0f;

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeTimer, {1.0f,1.0f,1.0f});



	// サウンドデータの読み込み
	BGMHandle = Audio::GetInstance()->LoadWave("BGM/ClearBGM.wav");

	voiceHandle = Audio::GetInstance()->PlayWave(BGMHandle, true, 0.5f);


}

void ClearScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		skydome_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kMain:
		skydome_->Update();
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
			Audio::GetInstance()->StopWave(voiceHandle);
		}

		BlinkUI(UISpaceToStart_, blinkTimer_, 1.0f);

		break;
	case Phase::kFadeOut:
		skydome_->Update();
		fade_->Update();
		if (fade_->IsFinished()) {
			sceneNo = TITLE;
			isFinished_ = true;
		}
		break;
	}

	playerTransform_.rotation_.y += 0.03f;
	counter_ += 1.0f / 60.0f;
	counter_ = std::fmod(counter_, kTimeClearMove);

	float angle = counter_ / kTimeClearMove * 2.0f * std::numbers::pi_v<float>;

	worldTransformClear_.translation_.y = std::sin(angle) + 10.0f;

	camera_.TransferMatrix();

	WorldTransformUpdate(worldTransformClear_);
	WorldTransformUpdate(playerTransform_);
}

void ClearScene::Draw() {

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Model::PreDraw(commandList);

	// 天球の描画
	skydome_->Draw();
	ModelManager::GetInstance()->GetModel("player", true)->Draw(playerTransform_, camera_);
	ModelManager::GetInstance()->GetModel("Clear")->Draw(worldTransformClear_, camera_);

	Model::PostDraw();

	Sprite::PreDraw(commandList);

	UISpaceToStart_->Draw();

	Sprite::PostDraw();

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

ClearScene::~ClearScene() { 
	delete fade_; 
	delete skydome_;
	delete UISpaceToStart_;
}