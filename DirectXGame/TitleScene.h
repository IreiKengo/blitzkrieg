#pragma once
#include <KamataEngine.h>
#include <vector>
#include <list>
#include "IScene.h"
#include "Enemy.h"

class Skydome;
class MapChipField;
class CameraController;
class EnemyBullet;
class Fade;

class TitleScene : public IScene
{
public:
	//シーンのフェーズ
	enum class Phase
	{
		kFadeIn,//フェードイン
		kMain,//メイン部
		kFadeOut,//フェードアウト
	};

	struct EnemySpawnData {
		int x;
		int y;
		Enemy::Type type;
	};


private:

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransformTitle_;
	// モデル
	KamataEngine::Model* modelTitle_ = nullptr;
	// カメラ
	KamataEngine::Camera camera_;
	// カメラコントローラー
	CameraController* CController_ = nullptr;
	KamataEngine::WorldTransform cameraTargetWorldTransform_;

	KamataEngine::Sprite* UISpaceToStart_ = nullptr;
	float blinkTimer_ = 0.0f; // 点滅用のタイマー
	

	float counter_ = 0.0f;

	static inline const float kTimeTitleMove = 2.0f;

	
	
	const float kPlayerTitle = 1.5f;

	const float kPlayerScale = 10.0f;

	Fade* fade_ = nullptr;

	//フェード時間
	float fadeTimer_ = 1.0f;

	//現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	uint32_t bgmHandle_ = 0;

	uint32_t voiceHandle = 0;

	// 天球
	Skydome* skydome_ = nullptr;
	// 3Dモデル
	KamataEngine::Model* modelSkydome_ = nullptr;
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
	// ブロックの3Dモデルデータ
	KamataEngine::Model* modelBlock_ = nullptr;
	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	std::list<Enemy*> enemies_;
	KamataEngine::Model* modelChargeEnemy_ = nullptr;
	std::list<EnemyBullet*> enemyBullets_;
	KamataEngine::Model* modelEnemyBullet_ = nullptr;


	//敵の生成・初期化
	void SpawnEnemies(const std::vector<EnemySpawnData>& spawnDatas);
	


public:

	void Initialize()override;

	void Update() override;

	void Draw() override;

	~TitleScene();

	void GenerateBlocks();

};