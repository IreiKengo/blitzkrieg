#pragma once
#include "Math.h"
#include <KamataEngine.h>
#include <array>
#include <algorithm>
#include <numbers>
#include <vector>

using namespace KamataEngine;

class DeathParticles
{
private:

	// モデル
	Model* model_ = nullptr;
	// カメラ
	Camera* camera_ = nullptr;

	//パーティクルの個数
	static inline const uint32_t kNumParticles = 8;

	std::array<WorldTransform, kNumParticles> worldTransforms_;

	//存続時間（消滅までの時間）
	static inline const float kDuration = 1.0f;
	//移動の速さ
	static inline const float kSpeed = 0.2f;
	//分割した一個分の角度
	static inline const float kAngleUnit = std::numbers::pi_v<float> * 2.0f / 8;
	//終了フラグ
	bool isFinished_ = false;
	//経過時間カウント
	float counter_ = 0.0f;

	//色変更オブジェクト
	ObjectColor objectColor_;
	//色の数値
	Vector4 color_;

public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();
	bool IsFinished() { return isFinished_; }

};
