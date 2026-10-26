#define NOMINMAX
#include "CameraController.h"
#include "ICameraTarget.h"
#include "Math.h"
#include <algorithm>

using namespace KamataEngine;

void CameraController::Initialize(Camera* camera) {

	camera_ = camera;
	directionPhase_ = DirectionPhase::kNone;
}

void CameraController::Update() {
	if (!camera_) {
		return;
	}

	if (directionPhase_ != DirectionPhase::kNone) {
		Vector3 targetPos{0.0f, 0.0f, 0.0f};

		if (target_) {
			targetPos = target_->GetWorldTransform().translation_;
		} else if (targetTransform_) {
			targetPos = targetTransform_->translation_;
		} else if (savedTarget_) {
			targetPos = savedTarget_->GetWorldTransform().translation_;
		} else if (savedTargetTransform_) {
			targetPos = savedTargetTransform_->translation_;
		}

		Vector3 playerDestination = targetPos + targetOffset_;
		UpdateStartDirection(playerDestination);
		return;
	}

	

	if (!target_ && !targetTransform_) {
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

	// 座標補間によりゆったり追従
	camera_->translation_ = Lerp(camera_->translation_, destination_, kInterpolationRate);

	// 移動範囲制限
	/*camera_->translation_.x = std::max(camera_->translation_.x, movableArea_.left);
	camera_->translation_.x = std::min(camera_->translation_.x, movableArea_.right);
	camera_->translation_.y = std::max(camera_->translation_.y, movableArea_.bottom);
	camera_->translation_.y = std::min(camera_->translation_.y, movableArea_.top);*/

	camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
	camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);

	// 行列を更新する
	camera_->UpdateMatrix();
}

void CameraController::Reset() {
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

void CameraController::StartStartDirection(float directionY) {
	directionPhase_ = DirectionPhase::kPanRight;
	directionTimer_ = 0.0f;

	if (directionY >= 0.0f) {
		directionY_ = directionY;
	}

	savedTarget_ = target_;
	savedTargetTransform_ = targetTransform_;

	ClearTarget();
	camera_->UpdateMatrix();
}

void CameraController::UpdateStartDirection(const KamataEngine::Vector3& playerDestination) {

	directionTimer_ += 1.0f / 60.0f;

	switch (directionPhase_) {
	case DirectionPhase::separation:

		camera_->translation_.z -= 0.5f;

		if (camera_->translation_.z <= kDirectionZ)
		{
			camera_->translation_.z = kDirectionZ;
			directionPhase_ = DirectionPhase::kPanRight;
			directionTimer_ = 0.0f;
		}

		break;
	case DirectionPhase::kPanRight: {
		// 左端から右端へゆっくり移動
		float t = std::min(directionTimer_ / kPanRightTime, 1.0f);
		camera_->translation_.x = Lerp(movableArea_.left, movableArea_.right, t);

		camera_->translation_.z -= 0.5f;
		if (camera_->translation_.z <= kDirectionZ) {
			camera_->translation_.z = kDirectionZ;
		}

		if (t >= 1.0f) {
			directionPhase_ = DirectionPhase::kReturnLeft;
			directionTimer_ = 0.0f;
			returnStartPos_ = camera_->translation_;
		}
	}
		break;
	case DirectionPhase::kReturnLeft: {
		// 右端からプレイヤー位置へ素早く移動
		float t = std::min(directionTimer_ / kReturnLeftTime, 1.0f);
		camera_->translation_.x = Lerp(returnStartPos_.x, playerDestination.x, t);
		camera_->translation_.y = Lerp(returnStartPos_.y, playerDestination.y, t);
		camera_->translation_.z = Lerp(returnStartPos_.z, playerDestination.z, t);


		camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
		camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);

		if (t >= 1.0f) {

			camera_->translation_ = playerDestination;
			camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
			camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);

			directionPhase_ = DirectionPhase::kNone;
			if (savedTarget_) {
				SetTarget(savedTarget_);
			} else if (savedTargetTransform_) {
				SetTarget(savedTargetTransform_);
			}

		}
	}
		break;
	case DirectionPhase::kNone:
	default:
		break;
	}

	camera_->UpdateMatrix();
}
