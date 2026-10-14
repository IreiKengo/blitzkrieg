#pragma once

class ITransition
{
public:
	virtual ~ITransition() = default;

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	//演出開始
	virtual void StartOut(float duration = 1.0f) = 0;
	virtual void StartIn(float duration = 1.0f) = 0;
	virtual void Stop() = 0;
	//終了判定
	virtual bool IsFinished() const = 0;
};
