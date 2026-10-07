#pragma once
#include <KamataEngine.h>

class ICameraTarget {
public:
	virtual ~ICameraTarget() = default;

	//ワールドトランスフォーム
	virtual const KamataEngine::WorldTransform& GetWorldTransform() const = 0;

	//速度を取得
	virtual KamataEngine::Vector3 GetVelocity() const { return KamataEngine::Vector3{0.0f, 0.0f, 0.0f};}
};