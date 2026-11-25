#pragma once
#include "KamataEngine.h"
#include <vector>
#include <list>
#include "IScene.h"
#include "Enemy.h"
#include "MapChipField.h"

class CameraController;
class DeathParticles;
class Fade;
class Player;
class Skydome;
class EnemyBullet;
class HitEffect;

// ゲームシーン
class GameScene : public IScene {
public:
	enum class GameResult {
		None,
		Clear,
		GameOver

	};

	struct EnemySpawnData {
		int x;
		int y;
		Enemy::Type type;
	};

private:
	// 自キャラ
	Player* player_ = nullptr;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// カメラ
	KamataEngine::Camera camera_;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	// カメラコントローラー
	CameraController* CController_ = nullptr;
	MapChipField::Rect blockBounds;

	// 敵
	std::list<Enemy*> enemies_;
	std::list<EnemyBullet*> enemyBullets_;


	// パーティクル
	std::list<DeathParticles*> deathParticles_;

	Fade* fade_ = nullptr;

	// ゲームのフェーズ（型）
	enum class Phase {
		kFadeIn,  // フェードイン
		kStartDirection,//スタート演出
		kPlay,    // ゲームプレイ
		kDeath,   // デス演出
		kFadeOut, // フェードアウト
	};

	// ゲームの現在フェーズ（変数）
	Phase phase_;
	

	// ヒットエフェクト
	std::list<HitEffect*> hitEffects_;

	GameResult result = GameResult::None;

	KamataEngine::Sprite* operateSprite = nullptr;
	KamataEngine::Sprite* conditionsSprite = nullptr;
	KamataEngine::Sprite* remainingNumberSprite = nullptr;
	KamataEngine::Sprite* numbersSprite[10];
	KamataEngine::Sprite* countDownSprite[4];
	KamataEngine::Sprite* arrowSprite_ = nullptr;

	uint32_t BGMHandle = 0;
	uint32_t voiceHandle = 0;

	float startTimer_ = 3.0f; // スタート演出のタイマー（秒単位）

	//クリア演出の変数
	bool isClearSlow_ = false;
	float clearTimer_ = 0.0f;
	const float kClearSlowDuration = 1.6f;
	const float kSlowTimeScale = 0.2f;



	void NumbersDraw(KamataEngine::Sprite* sprite[], int number, const KamataEngine::Vector2& position);

	void GenerateBlocks();
	// すべての当たり判定を行う
	void CheckAllCollisions();
	void ChangePhase();
	void SpawnEnemies(const std::vector<EnemySpawnData>& spawnDatas);

	void UpdateEnemyArrow();

public:
	// 初期化
	void Initialize() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	~GameScene();

	// デスフラグのgetter
	
	bool IsGameStart() const { return phase_ == Phase::kPlay; }

	// エフェクトを生成
	void CreateHitEffect(const KamataEngine::Vector3& position);
	void CreateDeathParticles(const KamataEngine::Vector3& position);

	GameResult GetResult() const { return result; }

};
