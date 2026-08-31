#include "GameScene.h"
#include <cmath>
using namespace KamataEngine;


void GameScene::Initialize() {
	// ここにシーンの初期化処理を書く

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// カメラの初期化
	// camera_.farZ = 1400;
	camera_.Initialize();

	// マップチップフィールド
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

	// 自キャラの生成
	player_ = new Player();
	// 自キャラの初期化
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	modelAttack_ = Model::CreateFromOBJ("attack_effect", true);

	player_->SetMapChipField(mapChipField_);

	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	player_->Initialize(modelPlayer_, modelAttack_, &camera_, playerPosition);

	// ブロックの3Dモデルデータの生成
	modelBlock_ = Model::CreateFromOBJ("block");
	// modelBlock_ = Model::Create();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	skydome_->Initialize(modelSkydome_, &camera_);

	GenerateBlocks();

	// カメラコントローラーの初期化
	// 生成
	CController_ = new CameraController();
	// 初期化
	CController_->Initialize(&camera_);
	// 追従対象をセット
	CController_->SetTarget(player_);
	// リセット
	CController_->Reset();

	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 100.0f - 6.0f};
	CController_->SetMovableArea(cameraArea);

	modelEnemy_ = Model::CreateFromOBJ("enemy");

	std::vector<std::pair<int, int>> enemySpawnIndices = {
	    {17, 14},
        {26, 13},
        {35, 10},
        {63, 13},
        {71, 12},
        {83, 12},
        {83, 17},
        {60, 1 },
        {72, 1 },
        {93, 1 }
    };
	SpawnEnemies(enemySpawnIndices);

	modelDeathParticles_ = Model::CreateFromOBJ("deathParticle");
	// ゲームプレイフェーズから開始
	phase_ = Phase::kFadeIn;

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	modelHitEffect_ = Model::CreateFromOBJ("particle");

	HitEffect::SetModel(modelHitEffect_);
	HitEffect::SetCamera(&camera_);

	result = GameResult::None;

	operateSprite = Sprite::Create(TextureManager::Load("UI/operate.png"), {0.0f, 0.0f});
	conditionsSprite = Sprite::Create(TextureManager::Load("UI/ClearConditions.png"), {420.0f, 0.0f});
	remainingNumberSprite = Sprite::Create(TextureManager::Load("UI/remainingNumber.png"), {0.0f, 650.0f});
	numbersSprite[0] = Sprite::Create(TextureManager::Load("numbers/0.png"), {140.0f, 650.0f});
	numbersSprite[1] = Sprite::Create(TextureManager::Load("numbers/1.png"), {140.0f, 650.0f});
	numbersSprite[2] = Sprite::Create(TextureManager::Load("numbers/2.png"), {140.0f, 650.0f});
	numbersSprite[3] = Sprite::Create(TextureManager::Load("numbers/3.png"), {140.0f, 650.0f});
	numbersSprite[4] = Sprite::Create(TextureManager::Load("numbers/4.png"), {140.0f, 650.0f});
	numbersSprite[5] = Sprite::Create(TextureManager::Load("numbers/5.png"), {140.0f, 650.0f});
	numbersSprite[6] = Sprite::Create(TextureManager::Load("numbers/6.png"), {140.0f, 650.0f});
	numbersSprite[7] = Sprite::Create(TextureManager::Load("numbers/7.png"), {140.0f, 650.0f});
	numbersSprite[8] = Sprite::Create(TextureManager::Load("numbers/8.png"), {140.0f, 650.0f});
	numbersSprite[9] = Sprite::Create(TextureManager::Load("numbers/9.png"), {140.0f, 650.0f});


	arrowSprite_ = Sprite::Create(
		TextureManager::Load("UI/arrow.png"),
		{640.0f, 600.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f});

	arrowSprite_->SetSize({100.0f, 100.0f});

	// サウンドデータの読み込み
	BGMHandle = Audio::GetInstance()->LoadWave("BGM/GameBGM.wav");

	voiceHandle = Audio::GetInstance()->PlayWave(BGMHandle, true,0.5f);

}

void GameScene::Update() {
	// ここにシーンの更新処理を書く

	// デスフラグの立ったエフェクトを削除
	hitEffects_.remove_if([](HitEffect* hitEffect) {
		if (hitEffect->IsDead()) {
			delete hitEffect;

			return true;
		}
		return false;
	});

	ChangePhase();

	// デバッグカメラの更新
	debugCamera_->Update();
#ifdef _DEBUG

	if (Input::GetInstance()->TriggerKey(DIK_G)) {
		// フラグをトグル
		isDebugCameraActive_ = !isDebugCameraActive_;
	}

#endif

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kPlay;
		}

		// 天球の更新
		skydome_->Update();

		//// カメラコントローラーの更新
		CController_->Update();
		// カメラの処理
		if (isDebugCameraActive_) {
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}
		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}

		// 自キャラの更新
		player_->Update();
		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update(player_);
		}

		break;
	case Phase::kPlay:
		// 天球の更新
		skydome_->Update();
		// 自キャラの更新
		player_->Update();
		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update(player_);
		}
		// カメラコントローラーの更新
		CController_->Update();
		// カメラの処理
		if (isDebugCameraActive_) {
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}
		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}

		// すべての当たり判定を行う
		CheckAllCollisions();

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}
		UpdateEnemyArrow();

		break;
	case Phase::kDeath:
		// 天球の更新
		skydome_->Update();
		// カメラコントローラーの更新
		CController_->Update();
		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update(player_);
		}
		if (deathParticles_) {
			deathParticles_->Update();
		}

		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			result = GameResult::GameOver;
			Audio::GetInstance()->StopWave(voiceHandle);
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}

		// 天球の更新
		skydome_->Update();
		// カメラコントローラーの更新
		CController_->Update();
		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update(player_);
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		break;
	}

	// デスフラグの立った敵を削除
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});
}

void GameScene::Draw() {
	// ここにシーンの描画処理を書く

	// DirectX共通インスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	// ここに3Dモデルインスタンスの描画処理を記述する

	// 天球の描画
	skydome_->Draw();

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;

			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	// 敵の描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	// 自キャラの描画
	if (!player_->IsDead()) {
		player_->Draw();
	}

	if (deathParticles_) {
		deathParticles_->Draw();
	}

	for (HitEffect* hitEffect : hitEffects_) {
		hitEffect->Draw();
	}

	// 3Dモデル描画後処理
	Model::PostDraw();

	Sprite::PreDraw(dxCommon->GetCommandList());

	operateSprite->Draw();
	conditionsSprite->Draw();
	remainingNumberSprite->Draw();

	NumbersDraw();
	
	if (phase_ == Phase::kPlay && !enemies_.empty() && player_ && !player_->IsDead()) {
		if (arrowSprite_) {
			arrowSprite_->Draw();
		}
	}

	Sprite::PostDraw();

	fade_->Draw();
}

void GameScene::GenerateBlocks() {

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

void GameScene::CheckAllCollisions() {
#pragma region 自キャラと敵キャラの当たり判定
	// 判定対象1と2の座標
	AABB aabb1, aabb2;

	// 自キャラの座標
	aabb1 = player_->GetAABB();

	// 自キャラと敵弾すべての当たり判定
	for (Enemy* enemy : enemies_) {
		if (enemy->IsCollisionDisabled())
			// コリジョン向こうの敵はスキップ
			continue;

		// 敵弾の座標
		aabb2 = enemy->GetAABB();

		// AABB同士の交差判定
		if (IsCollision(aabb1, aabb2)) {
			// 自キャラの衝突時関数を呼び出す
			player_->OnCollision(enemy);
			// 敵の衝突時関数を呼び出す
			enemy->OnCollision(player_);
		}
	}

#pragma endregion
}

void GameScene::ChangePhase() {

	switch (phase_) {

	case Phase::kPlay:
		if (player_->IsDead()) {
			// 死亡演出フェーズに切り替え
			phase_ = Phase::kDeath;

			if (!deathParticles_) {
				// 自キャラの座標を取得
				const Vector3& deathParticlePosition = player_->GetWorldPosition();

				// 生成処理
				deathParticles_ = new DeathParticles;

				deathParticles_->Initialize(modelDeathParticles_, &camera_, deathParticlePosition);
			}
		}

		// 敵が全滅したらクリア
		else if (enemies_.empty()) {
			phase_ = Phase::kFadeOut;
			result = GameResult::Clear;
			Audio::GetInstance()->StopWave(voiceHandle);
		}

		break;
	case Phase::kDeath:

		break;
	}
}

void GameScene::CreateHitEffect(const Vector3& position) {

	HitEffect* newHitEffect = HitEffect::Create(position);
	hitEffects_.push_back(newHitEffect);
}

GameScene::~GameScene() {

	for (int i = 0; i < 9; i++)
	{
	delete numbersSprite[i];

	}
	delete remainingNumberSprite;
	delete conditionsSprite;
	delete operateSprite;

	delete modelPlayer_;
	delete player_;
	delete modelAttack_;

	delete deathParticles_;
	deathParticles_ = nullptr;

	delete modelBlock_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
	delete debugCamera_;
	delete skydome_;
	delete modelSkydome_;
	delete mapChipField_;
	delete modelEnemy_;

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}

	delete deathParticles_;
	delete modelDeathParticles_;

	delete modelHitEffect_;
	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}
	delete arrowSprite_;
}

void GameScene::SpawnEnemies(const std::vector<std::pair<int, int>>& indices) {
	for (const auto& [xIndex, yIndex] : indices) {
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex + 1);

		newEnemy->SetMapChipField(mapChipField_);
		newEnemy->SetGameScene(this);
		newEnemy->initialize(modelEnemy_, &camera_, enemyPosition);
		enemies_.push_back(newEnemy);
	}
}

void GameScene::UpdateEnemyArrow() 
{
	// 敵がいない、プレイヤーが死亡している場合は処理しない
	if (enemies_.empty() || player_->IsDead()) {
		return;
	}

	const Vector3& playerPosition = player_->GetWorldPosition();
	Enemy* closestEnemy = nullptr;
	float minDistanceSq = (std::numeric_limits<float>::max)();

	// 1. 最も近い敵を探す（距離の2乗で比較して計算を軽量化）
	for (Enemy* enemy : enemies_) {
		if (enemy->IsDead())
			continue;

		Vector3 enemyPosition = enemy->GetWorldPosition();
		float dx = enemyPosition.x - playerPosition.x;
		float dy = enemyPosition.y - playerPosition.y;
		

		float distSq = (dx * dx) + (dy * dy);
		if (distSq < minDistanceSq) {
			minDistanceSq = distSq;
			closestEnemy = enemy;
		}

	}

	if (!closestEnemy)
	{
		return;
	}

	// 2. プレイヤーから最寄りの敵への方向
	Vector3 enemyPosition = closestEnemy->GetWorldPosition();
	float diffX = enemyPosition.x - playerPosition.x;
	float diffY = enemyPosition.y - playerPosition.y;

	// 3. 角度の計算
	float angle = std::atan2(-diffY, diffX);

	// 4. 画像の補正
	float baseOffset = std::numbers::pi_v<float> / 2.0f;
	float finalAngle = angle - baseOffset;

	// 3. プレイヤーの頭上の3D座標を設定
	Vector3 arrowWorldPos = playerPosition;
	arrowWorldPos.y += 1.2f; // ★プレイヤーの少し頭上に配置（お好みで調整）

	Matrix4x4 matViewProjection = camera_.matView * camera_.matProjection;
	Matrix4x4 matViewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0.0f, 1.0f);
	Matrix4x4 matVPV = matViewProjection * matViewport;
	Vector3 screenPos = Transform(arrowWorldPos, matVPV);


	
	if (arrowSprite_) {
		// 回転角を設定
		arrowSprite_->SetRotation(finalAngle);
		arrowSprite_->SetPosition({screenPos.x, screenPos.y});

	
	}

}

void GameScene::NumbersDraw() 
{

	size_t remaining = enemies_.size();
	std::string remainingStr = std::to_string(remaining);

	float digitWidth = 35.0f; // 数字1つ分の幅
	float baseX = 150.0f;     // 中央X座標
	float y = 650.0f;

	// 総幅を計算して描画開始Xを中央に寄せる
	float totalWidth = remainingStr.size() * digitWidth;
	float startX = baseX - totalWidth / 2.0f;

	// 各桁を描画
	for (char c : remainingStr) {
		int digit = c - '0'; // '0'～'9' -> 0～9
		numbersSprite[digit]->SetPosition({startX, y});
		numbersSprite[digit]->Draw();
		startX += digitWidth; // 次の桁に移動
	}

}