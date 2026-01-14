#include "ClearScene.h"
#include <numbers>

using namespace KamataEngine;

void ClearScene::Initialize() {
	modelClear_ = Model::CreateFromOBJ("Clear");

	// カメラ初期化
	camera_.Initialize();

	worldTransformClear_.Initialize();

	worldTransformClear_.scale_ = {kPlayerClear, kPlayerClear, kPlayerClear};

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeTimer);
}

void ClearScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
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
	counter_ = std::fmod(counter_, kTimeClearMove);

	float angle = counter_ / kTimeClearMove * 2.0f * std::numbers::pi_v<float>;

	worldTransformClear_.translation_.y = std::sin(angle) + 10.0f;

	camera_.TransferMatrix();

	WorldTransformUpdate(worldTransformClear_);
}

void ClearScene::Draw() {

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Model::PreDraw(commandList);

	modelClear_->Draw(worldTransformClear_, camera_);

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

ClearScene::~ClearScene() { delete fade_; }