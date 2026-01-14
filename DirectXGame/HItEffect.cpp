#include "HItEffect.h"
#include <random>
#include<numbers>
#include <cassert>

using namespace KamataEngine;

Model* HitEffect::model_ = nullptr;
Camera* HitEffect::camera_ = nullptr;

void HitEffect::initialize(const Vector3& position) {


	HitEffect::SetModel(model_);
	HitEffect::SetCamera(camera_);

	//乱数生成エンジン
	std::random_device seedGenerator; 
	//メルセンヌ・ツイスターエンジン(64版)
	std::mt19937_64 randomEngine;
	//メルセンヌ・ツイスターエンジンの初期化
	randomEngine.seed(seedGenerator());
	//指定範囲の乱数生成器（浮動少数点用）
	std::uniform_real_distribution<float> rotationDistribution(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);




	//円形エフェクト
	circleWorldTransform_.Initialize();
	circleWorldTransform_.translation_ = position;
	circleWorldTransform_.translation_.z =-1.0f;

	// 楕円エフェクト
	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		worldTransform.scale_ = {ellipseWidth, ellipseLength, 1.0f};
		worldTransform.rotation_ = {0.0f, 0.0f, rotationDistribution(randomEngine)};

		worldTransform.translation_ = position;
		circleWorldTransform_.translation_.z = 1.0f;
		worldTransform.Initialize();
	}
}


void HitEffect::Update()
{


	//円形エフェクト
	WorldTransformUpdate(circleWorldTransform_);

	for (WorldTransform& worldTransform : ellipseWorldTransforms_)
	{
		WorldTransformUpdate(worldTransform);
	}

}

void HitEffect::Draw()
{

	switch (state_) {
	case State::kSpread: {
		++counter_;
		float scale = 0.5f + static_cast<float>(counter_) / kSpreadTime * 0.5f;
		const float slashScale = 2.0f;
		for (auto& slashWorldTransform : ellipseWorldTransforms_) {
			slashWorldTransform.scale_ = {0.1f, scale * slashScale, 1.0f};
		}

		const float circleScale = 1.0;

		circleWorldTransform_.scale_ = {scale * circleScale, scale * circleScale, 1.0f};

		if (counter_ >= kSpreadTime) {
			state_ = State::kFade;
			counter_ = 0; // カウンターをリセット
		}
		break;
	}
	case State::kFade: {
		++counter_;
		

		if (++counter_ >= kFadeTime) {
			state_ = State::kDead;
		}

		break;
	}
	default:
		break;
	}

	//円形エフェクト
	model_->Draw(circleWorldTransform_, *camera_);

	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		model_->Draw(worldTransform, *camera_);
	}

}

HitEffect* HitEffect::Create(const Vector3& position) {

	// インスタンス生成
	HitEffect* instance = new HitEffect();
	//newの失敗を検出
	assert(instance);
	//インスタンスの初期化
	instance->initialize(position);
	//初期化したインスタンスを返す
	return instance;

}





