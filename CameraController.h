#pragma once
#include "KamataEngine.h"
#include"Player.h"
#include"Math.h"
#include <vector>


// 前方宣言
class Player;


using namespace KamataEngine;

class CameraController {

public:


 // 矩形
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	void Initialize(Camera* camera);

	void Update();

	// 外部からポインタをセットするもの
	void SetTarget(Player* target) { target_ = target; }

	void Reset();

	void SetMovableArea(Rect area) { movableArea_ = area; }

private:

	//カメラ
	Camera* camera_ = nullptr;

	Player* target_ = nullptr;

	// 追従対象とカメラの座標の差（オフセット）
	Vector3 targetOffset_ = {0, 0, -20.0f};

	//カメラ移動範囲
	Rect movableArea_ = {0, 100, 0, 100};

	//カメラの目標座標
	Vector3 destination_;
	//座標補間割合
	static inline const float kInterpolationRate = 0.1f;

	//速度掛け率
	static inline const float kVelocityBias = 30.0f;

	//追従対象の各方向へのカメラ移動範囲
	static inline const Rect margin = {15.0f, 85.0f, -10.0f, 100.0f};


};