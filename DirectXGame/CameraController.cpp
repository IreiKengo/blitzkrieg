#define NOMINMAX
#include "CameraController.h"
#include "Player.h"
//#include <algorithm>

using namespace KamataEngine;

void CameraController::Initialize(Camera*camera)
{

	camera_ = camera;
}

void CameraController::Update()
{
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	//02_06スライド29で追加
	const Vector3& targetVelocity = target_->GetVelocity();

	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	destination_ = targetWorldTransform.translation_ + targetOffset_ + targetVelocity * kVelocityBias;

	//座標補間によりゆったり追従
	camera_->translation_ = Lerp(camera_->translation_, destination_, kInterpolationRate);

	//追従対象が画面外に出ないように補正
	camera_->translation_.x = std::max(camera_->translation_.x, target_->GetWorldTransform().rotation_.x + margin.left);
	camera_->translation_.x = std::min(camera_->translation_.x, target_->GetWorldTransform().rotation_.x + margin.right);
	camera_->translation_.y = std::max(camera_->translation_.y, target_->GetWorldTransform().rotation_.x + margin.bottom);
	camera_->translation_.y = std::min(camera_->translation_.y, target_->GetWorldTransform().rotation_.x + margin.top);

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
	//追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	//追従対象とオフセットからカメラの座標を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
}