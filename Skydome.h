#pragma once
#include "KamataEngine.h"



class Skydome {
private:
	//ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	//モデル
	KamataEngine::Model* model_ = nullptr;
	//カメラ
	KamataEngine::Camera* camera_ = nullptr;


public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	// 更新
	void Update();

	// 描画
	void Draw();
};
