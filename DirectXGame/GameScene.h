#pragma once
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Math.h"
#include "Player.h"
#include "Skydome.h"
#include <vector>
#include "CameraController.h"
#include "Enemy.h"
#include "DeathParticles.h"
#include "Fade.h"
#include "HItEffect.h"


// ゲームシーン
class GameScene {
public:
	enum class GameResult {
		None,
		Clear,
		GameOver

	};

private:
	// 自キャラ
	Player* player_ = nullptr;
	// プレイヤーの3Dモデルデータ
	KamataEngine::Model* modelPlayer_ = nullptr;
	//攻撃エフェクト用データ
	KamataEngine::Model* modelAttack_ = nullptr;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// ブロックの3Dモデルデータ
	KamataEngine::Model* modelBlock_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	

	// 天球
	Skydome* skydome_ = nullptr;
	// 3Dモデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;
	

	//カメラコントローラー
	CameraController* CController_ = nullptr;

	//敵
	std::list<Enemy*> enemies_;
	// エネミーの3Dモデルデータ
	KamataEngine::Model* modelEnemy_ = nullptr;

	//パーティクル
	DeathParticles* deathParticles_ = nullptr;
	KamataEngine::Model* modelDeathParticles_ = nullptr;

	Fade* fade_ = nullptr;


	//ゲームのフェーズ（型）
	enum class Phase
	{
		kFadeIn,//フェードイン
		kPlay,//ゲームプレイ
		kDeath,//デス演出
		kFadeOut,//フェードアウト
	};

	//ゲームの現在フェーズ（変数）
	Phase phase_;

	// 終了フラグ
	bool finished_ = false;

	//ヒットエフェクト
	std::list<HitEffect*> hitEffects_;
	KamataEngine::Model* modelHitEffect_ = nullptr;

	GameResult result = GameResult::None;

	KamataEngine::Sprite* operateSprite = nullptr;
	KamataEngine::Sprite* conditionsSprite = nullptr;
	KamataEngine::Sprite* remainingNumberSprite = nullptr;
	KamataEngine::Sprite* numbersSprite[10];
	KamataEngine::Sprite* arrowSprite_ = nullptr;


	uint32_t BGMHandle = 0;

	uint32_t voiceHandle = 0;

	void NumbersDraw();

public:




	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	~GameScene();

	void GenerateBlocks();

	//すべての当たり判定を行う
	void CheckAllCollisions();

	void ChangePhase();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

	//エフェクトを生成
	void CreateHitEffect(const KamataEngine::Vector3& position);


	GameResult GetResult() const { return result; }

	void SpawnEnemies(const std::vector<std::pair<int, int>>& indices);

	void UpdateEnemyArrow();

};