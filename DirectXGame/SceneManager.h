#pragma once
#include "IScene.h"
#include <memory>

class SceneManager {
public:

	SceneManager();

	~SceneManager();
	int Run();

private:
	// シーンの配列
	std::unique_ptr<IScene> scene_;

	// 現在のシーン
	int currentSceneNo_;
	// 前のシーン
	int prevSceneNo_;


	void ChangeScene(int sceneNo);

};
