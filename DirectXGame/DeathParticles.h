#pragma once
#include <KamataEngine.h>
#include <numbers>
#include <array>


class DeathParticles
{
private:

	// モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	//パーティクルの個数
	static inline const uint32_t kNumParticles = 20;

	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_;

	std::array<KamataEngine::Vector3, kNumParticles> velocities_;//速度
	std::array<float, kNumParticles> initialScales_;//初期サイズ

	//存続時間（消滅までの時間）
	static inline const float kDuration = 0.45f;

	
	//終了フラグ
	bool isFinished_ = false;
	//経過時間カウント
	float counter_ = 0.0f;

	//色変更オブジェクト
	KamataEngine::ObjectColor objectColor_;
	//色の数値
	KamataEngine::Vector4 color_;

public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();
	bool IsFinished() const { return isFinished_; }

};
