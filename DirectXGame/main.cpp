#include <Windows.h>
#include "KamataEngine.h"
#include"GameScene.h"
#include "TitleScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"
using namespace KamataEngine;

// ゲームシーンのインスタンスの生成
GameScene* gameScene = nullptr;

TitleScene* titleScene = nullptr;

ClearScene* clearScene = nullptr;

GameOverScene* gameOverScene = nullptr;

// シーン（型）
enum class Scene {
	kUnknown = 0,
	kTitle,
	kGame, kClear, kGameOver };
// 現在シーン（型）
Scene scene = Scene::kUnknown;

void ChangeScene() {
	switch (scene) {

	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// シーン更新
			scene = Scene::kGame;
			// 旧シーンの解放
			delete titleScene;
			titleScene = nullptr;
			// 新シーンの生成と初期化
			gameScene = new GameScene;
			gameScene->Initialize();
		}
		break;
	case Scene::kGame:

		if (gameScene->IsFinished()) {
			// ゲームの結果を取得
			GameScene::GameResult result = gameScene->GetResult();

			// 旧シーンの解放
			delete gameScene;
			gameScene = nullptr;

			if (result == GameScene::GameResult::Clear) {
				// シーン更新
				scene = Scene::kClear;
				// 新シーンの生成と初期化
				clearScene = new ClearScene;
				clearScene->Initialize();
			} else if (result == GameScene::GameResult::GameOver) {
				// シーン更新
				scene = Scene::kGameOver;
				// 新シーンの生成と初期化
				gameOverScene = new GameOverScene;
				gameOverScene->Initialize();
			}
		}
		break;
	case Scene::kClear:
		if (clearScene->IsFinished()) {
			// シーン更新
			scene = Scene::kTitle;
			// 旧シーンの解放
			delete clearScene;
			clearScene = nullptr;
			// 新シーンの生成と初期化
			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;
	case Scene::kGameOver:
		if (gameOverScene->IsFinished()) {
			// シーン更新
			scene = Scene::kTitle;
			// 旧シーンの解放
			delete gameOverScene;
			gameOverScene = nullptr;
			// 新シーンの生成と初期化
			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	case Scene::kClear:
		clearScene->Update();
		break;
	case Scene::kGameOver:
		gameOverScene->Update();
		break;
	}
}

void DrawScene() {
	switch (scene) {

	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	case Scene::kClear:
		clearScene->Draw();
		break;
	case Scene::kGameOver:
		gameOverScene->Draw();
		break;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	//エンジンの初期化
	KamataEngine::Initialize();

	

	//最初のシーンの初期化
	scene = Scene::kTitle;
	//gameScene = new GameScene;
	///gameScene->Initialize();
	titleScene = new TitleScene;
	titleScene->Initialize();
	
	//DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//メインループ
	while (true)
	{
		//エンジンの更新
		if (KamataEngine::Update())
		{
			break;
		}

		//シーン切り替え
		ChangeScene();
		//現在シーン更新
		UpdateScene();

		//描画開始
		dxCommon->PreDraw();

		//現在シーンの描画
		DrawScene();

		//描画終了
		dxCommon->PostDraw();

	}

	delete titleScene;
	delete gameScene;
	delete clearScene;
	delete gameOverScene;


	//エンジンの終了処理
	KamataEngine::Finalize();
	
	return 0;
}
