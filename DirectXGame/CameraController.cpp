#define NOMINMAX
#include "CameraController.h"
#include "ICameraTarget.h"
//#include <algorithm>

using namespace KamataEngine;

void CameraController::Initialize(Camera*camera)
{

	camera_ = camera;
}

void CameraController::Update()
{
	if (!camera_ || (!target_ && !targetTransform_)) {
		return;
	}
	
	Vector3 targetPos{0.0f, 0.0f, 0.0f};
	Vector3 targetVelocity{0.0f, 0.0f, 0.0f};

	if (target_) {
		targetPos = target_->GetWorldTransform().translation_;
		targetVelocity = target_->GetVelocity();
	} else if (targetTransform_) {
		targetPos = targetTransform_->translation_;
	} 

	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	destination_ = targetPos + targetOffset_ + targetVelocity * kVelocityBias;

	//座標補間によりゆったり追従
	camera_->translation_ = Lerp(camera_->translation_, destination_, kInterpolationRate);

	//if (target_)
	//{
	//	// 追従対象が画面外に出ないように補正
	//	camera_->translation_.x = std::max(camera_->translation_.x, targetPos.x + margin.left);
	//	camera_->translation_.x = std::min(camera_->translation_.x, targetPos.x + margin.right);
	//	camera_->translation_.y = std::max(camera_->translation_.y, targetPos.x + margin.bottom);
	//	camera_->translation_.y = std::min(camera_->translation_.y, targetPos.x + margin.top);
	//}
	

	//移動範囲制限
	camera_->translation_.x = std::max(camera_->translation_.x, movableArea_.left);
	camera_->translation_.x = std::min(camera_->translation_.x, movableArea_.right);
	camera_->translation_.y = std::max(camera_->translation_.y, movableArea_.bottom);
	camera_->translation_.y = std::min(camera_->translation_.y, movableArea_.top);




	//行列を更新する
	camera_->UpdateMatrix();
}

void CameraController::Reset() 
{
	if (!camera_ || (!target_ && !targetTransform_)) {
		return;
	}

	Vector3 targetPos{0.0f, 0.0f, 0.0f};
	if (target_) {
		targetPos = target_->GetWorldTransform().translation_;
	} else if (targetTransform_) {
		targetPos = targetTransform_->translation_;
	} else {
		return;
	}
	camera_->translation_ = targetPos + targetOffset_;
	camera_->UpdateMatrix();
}