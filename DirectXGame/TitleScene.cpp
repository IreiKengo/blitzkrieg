#include "TitleScene.h"
#include <numbers>
#include "Skydome.h"
#include "MapChipField.h"
#include "CameraController.h"
#include "Math.h"
#include "Enemy.h"
#include "EnemyBullet.h"
#include "Fade.h"

using namespace KamataEngine;

void TitleScene::Initialize()
{
	modelTitle_ = Model::CreateFromOBJ("titleFont");
	UISpaceToStart_ = Sprite::Create(TextureManager::Load("UI/spaceToStart.png"), {400.0f, 400.0f});

	// カメラ初期化
	camera_.Initialize();

	cameraTargetWorldTransform_.Initialize();
	cameraTargetWorldTransform_.translation_ = {30.0f, 15.0f, 0.0f};


	// カメラコントローラーの初期化
	// 生成
	CController_ = new CameraController();
	CameraController::Rect cameraArea = {-100.0f, 100.0f, -100.0f, 100.0f};
	CController_->SetMovableArea(cameraArea);
	// 初期化
	CController_->Initialize(&camera_);
	CController_->SetTarget(&cameraTargetWorldTransform_);
	CController_->SetTargetOffset({-4.0f, -2.0f, -30.0f});
	CController_->Reset();
	
	// マップチップフィールド
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/title.csv");
	modelBlock_ = Model::CreateFromOBJ("block");

	GenerateBlocks();
	
	worldTransformTitle_.Initialize();

	worldTransformTitle_.scale_ = 
	{
		kPlayerTitle, kPlayerTitle, kPlayerTitle
	};
	worldTransformTitle_.translation_ = {27.0f, 20.0f, -5.0f};

	//敵の生成初期化
	modelEnemy_ = Model::CreateFromOBJ("enemy");
	std::vector<std::pair<int, int>> enemySpawnIndices = {
	    {1, 12},
        {2, 12},   
    };
	SpawnEnemies(enemySpawnIndices);

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	skydome_->Initialize(modelSkydome_, &camera_);

	

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeTimer_);


	// サウンドデータの読み込み
	bgmHandle_ = Audio::GetInstance()->LoadWave("BGM/TitleBGM.wav");

	voiceHandle = Audio::GetInstance()->PlayWave(bgmHandle_, true, 0.5f);

}

void TitleScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		skydome_->Update();
		// カメラコントローラーの更新
		CController_->Update();

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}

		break;
	case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
			Audio::GetInstance()->StopWave(voiceHandle);
		}
		skydome_->Update();

		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update(nullptr);
		}

		// カメラコントローラーの更新
		CController_->Update();
		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}

		BlinkUI(UISpaceToStart_, blinkTimer_,1.0f);

		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		skydome_->Update();
		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update(nullptr);
		}
		break;
	}

	counter_ += 1.0f / 60.0f;
	counter_ = std::fmod(counter_, kTimeTitleMove);

	float angle = counter_ / kTimeTitleMove * 2.0f * std::numbers::pi_v<float>;

	worldTransformTitle_.translation_.y = std::sin(angle) + 18.0f;

	

	WorldTransformUpdate(worldTransformTitle_);
	
};

void TitleScene::Draw()
{
	
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Model::PreDraw(commandList);

	skydome_->Draw();

	// 敵の更新
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;

			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	modelTitle_->Draw(worldTransformTitle_, camera_);

	Model::PostDraw();
	Sprite::PreDraw(commandList);

	UISpaceToStart_->Draw();

	Sprite::PostDraw();
	
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Draw();
		
		break;
	case Phase::kMain:

		break;
	case Phase::kFadeOut:
		fade_->Draw();
		
		break;
	}
}

void TitleScene::GenerateBlocks() {

	// 要素数
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();
	// 要素数を変更する
	// 列数を設定(縦方向のブロック数)
	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		// 1列の要素数を設定（横方向のブロック数）
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {

			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform;
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

TitleScene::~TitleScene() 
{
	delete modelBlock_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
	delete mapChipField_;
	delete skydome_;
	delete fade_;
	delete CController_;
	delete UISpaceToStart_;

	delete modelEnemy_;
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}


}

void TitleScene::SpawnEnemies(const std::vector<std::pair<int, int>>& indices) {
	for (const auto& [xIndex, yIndex] : indices) {
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex + 1);

		newEnemy->SetMapChipField(mapChipField_);
		newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);
		if (modelEnemyBullet_) {
			newEnemy->SetBulletInfo(modelEnemyBullet_, &enemyBullets_);
		}
		enemies_.push_back(newEnemy);
	}
}

