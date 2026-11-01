#pragma once
#include <KamataEngine.h>

enum Scene { TITLE, GAME, CLEAR, OVER };

class IScene {

protected:

	static int sceneNo;

	bool isFinished_ = false;

public:
	virtual ~IScene() = default;

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	int GetSceneNo()const { return sceneNo; }
	bool IsFinished() const { return isFinished_; }
};