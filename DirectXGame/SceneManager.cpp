#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"
#include <KamataEngine.h>

using namespace KamataEngine;

SceneManager::SceneManager()
{

	currentSceneNo_ = TITLE;
	prevSceneNo_ = currentSceneNo_;

	ChangeScene(currentSceneNo_);

}

SceneManager::~SceneManager() {}

int SceneManager::Run() 
{
	
	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		//シーンのチェック
		prevSceneNo_ = currentSceneNo_;
		currentSceneNo_ = scene_->GetSceneNo();

		//シーンの更新チェック
		if (prevSceneNo_ != currentSceneNo_)
		{
			ChangeScene(currentSceneNo_);
		}

		//更新処理
		scene_->Update();


		// 描画開始
		dxCommon->PreDraw();


		//描画処理
		scene_->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	return 0;

}

void SceneManager::ChangeScene(int sceneNo)
{

	switch (sceneNo) {
	case TITLE:
		scene_ = std::make_unique<TitleScene>();
		break;
	case GAME:
		scene_ = std::make_unique<GameScene>();
		break;
	case CLEAR:
		scene_ = std::make_unique<ClearScene>();
		break;
	case OVER:
		scene_ = std::make_unique<GameOverScene>();
		break;
	}

	// 新しいシーンを初期化
	scene_->Initialize();

}
