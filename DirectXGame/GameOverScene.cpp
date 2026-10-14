#include "GameOverScene.h"
#include <numbers>
#include "ITransition.h"

using namespace KamataEngine;

void GameOverScene::Initialize() {
	modelGameOver_ = Model::CreateFromOBJ("GameOver");

	// カメラ初期化
	camera_.Initialize();

	worldTransformGameOver_.Initialize();

	worldTransformGameOver_.scale_ = {kPlayerGameOver, kPlayerGameOver, kPlayerGameOver};

	transition_ = new Fade();
	transition_->Initialize();
	transition_->StartIn(fadeTimer);

	// サウンドデータの読み込み
	BGMHandle = Audio::GetInstance()->LoadWave("BGM/GameOverBGM.wav");

	voiceHandle = Audio::GetInstance()->PlayWave(BGMHandle, true, 0.5f);

}

void GameOverScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		transition_->Update();
		if (transition_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			transition_->StartOut(1.0f);
			phase_ = Phase::kFadeOut;
			Audio::GetInstance()->StopWave(voiceHandle);
		}

		break;
	case Phase::kFadeOut:
		transition_->Update();
		if (transition_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	counter_ += 1.0f / 60.0f;
	counter_ = std::fmod(counter_, kTimeGameOverMove);

	float angle = counter_ / kTimeGameOverMove * 2.0f * std::numbers::pi_v<float>;

	worldTransformGameOver_.translation_.y = std::sin(angle) + 10.0f;

	camera_.TransferMatrix();

	WorldTransformUpdate(worldTransformGameOver_);
}

void GameOverScene::Draw() {

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Model::PreDraw(commandList);

	modelGameOver_->Draw(worldTransformGameOver_, camera_);

	Model::PostDraw();

	switch (phase_) {
	case Phase::kFadeIn:
		transition_->Draw();

		break;
	case Phase::kMain:

		break;
	case Phase::kFadeOut:
		transition_->Draw();

		break;
	}
}

GameOverScene::~GameOverScene() { delete transition_; }