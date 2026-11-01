#include "SceneManager.h"


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	//エンジンの初期化
	KamataEngine::Initialize();

	SceneManager* sceneManager_ = new SceneManager();

	sceneManager_->Run();

	sceneManager_->~SceneManager();

	//エンジンの終了処理
	KamataEngine::Finalize();
	
	return 0;
}
