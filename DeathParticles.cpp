#include "DeathParticles.h"

void DeathParticles::Initialize(Model* model, Camera* camera, const Vector3& position)
{

	//モデル
	model_ = model;
	//カメラ
	camera_ = camera;

	//ワールド変換の初期化
	for (WorldTransform& worldTransform : worldTransforms_)
	{
		worldTransform.Initialize();
		worldTransform.translation_ = position;
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

	for (uint32_t i = 0; i < 8; ++i)
	{
		//基本となる速度ベクトル
		Vector3 velocity = {0.1f, 0.0f, 0.0f};
		//回転角を計算する
		float angle = kAngleUnit * i;
		//Z軸周り回転行列
		Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);
		//基本ベクトルを回転させて速度ベクトルを得る
		velocity = Transform(velocity, matrixRotation);
		//移動処理
		worldTransforms_[i].translation_ += velocity;
	}

	//カウンターを1フレーム分の秒数進める
	counter_ += 1.0f / 60.0f;

	//存続時間の上限に達したら
	if (counter_ >= kDuration)
	{
		counter_ = kDuration;
		//終了扱いにする
		isFinished_ = true;
	}

	color_.w = std::clamp(kDuration - counter_, 0.0f, 1.0f);
	//色変更オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);



	for (WorldTransform& worldTransform : worldTransforms_) 
	{
		WorldTransformUpdate(worldTransform);
	}
}

void DeathParticles::Draw()
{
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}


	for (WorldTransform& worldTransform : worldTransforms_) {

		model_->Draw(worldTransform, *camera_,&objectColor_);
	}

}
