#include "DeathParticles.h"
#include "Math.h"
#include <algorithm>
#include <cmath>
#include <random>


using namespace KamataEngine;

void DeathParticles::Initialize(Model* model, Camera* camera, const Vector3& position)
{

	//モデル
	model_ = model;
	//カメラ
	camera_ = camera;


	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());
	std::uniform_real_distribution<float> angleDist(0.0f, std::numbers::pi_v<float> * 2.0f);
	std::uniform_real_distribution<float> speedDist(0.22f, 0.55f);
	std::uniform_real_distribution<float> scaleDist(0.6f, 1.3f);

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		worldTransforms_[i].Initialize();
		worldTransforms_[i].translation_ = position;

		float angle = angleDist(randomEngine);
		float speed = speedDist(randomEngine);
		float scale = scaleDist(randomEngine);

		velocities_[i] = {std::cos(angle) * speed, std::sin(angle) * speed, 0.0f};
		initialScales_[i] = scale;
		worldTransforms_[i].scale_ = {scale * 2.2f, scale * 0.18f, scale * 0.18f};
		worldTransforms_[i].rotation_ = {0.0f, 0.0f, angle};
	}

	//色の変更
	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};
}

void DeathParticles::Update()
{
	//終了なら何もしない
	if (isFinished_)
	{
		return;
	}

	// カウンターを1フレーム分の秒数進める
	counter_ += 1.0f / 60.0f * gTimeScale;

	// 存続時間の上限に達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		// 終了扱いにする
		isFinished_ = true;
	}


	float t = counter_ / kDuration;

	//初速から空気抵抗で減速しながら徐々に小さくなって消える
	// 減衰率
	float damping = std::pow(0.87f, gTimeScale);
	float shrink = 1.0f - (t * t);//小さくなる

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 移動と減速
		worldTransforms_[i].translation_ += velocities_[i] * gTimeScale;
		velocities_[i] *= damping;

		float length = initialScales_[i] * 2.2f * shrink;
		float thickness = initialScales_[i] * 0.18f * (1.0f - t);
		worldTransforms_[i].scale_ = {length, thickness, thickness};
	}

	

	color_.x = 1.0f;
	color_.y = std::clamp(1.0f - t * 0.5f, 0.0f, 1.0f);
	color_.z = std::clamp(1.0f - t * 1.2f, 0.0f, 1.0f);
	color_.w = std::clamp(1.0f - t, 0.0f, 1.0f);
	// 色変更オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);

	for (WorldTransform& worldTransform : worldTransforms_) {
		WorldTransformUpdate(worldTransform);
	}

	//color_.w = std::clamp(kDuration - counter_, 0.0f, 1.0f);
	

}

void DeathParticles::Draw()
{
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}


	for (WorldTransform& worldTransform : worldTransforms_) {
		if (worldTransform.scale_.x <= 0.001f) {
			continue;
		}

		model_->Draw(worldTransform, *camera_,&objectColor_);
	}

}
