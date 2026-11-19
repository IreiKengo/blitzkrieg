#include "ModelManager.h"

using namespace KamataEngine;

ModelManager* ModelManager::GetInstance()
{

	static ModelManager instance;
	return &instance;

}

KamataEngine::Model* ModelManager::GetModel(const std::string& filePath, bool smoothing)
{
	//読み込み済みならreturn
	auto it = models_.find(filePath);
	if (it != models_.end()) {
		return it->second.get();

	}

	// 未読み込みなら新しく生成して登録
	std::unique_ptr<Model> newModel(Model::CreateFromOBJ(filePath, smoothing));
	models_[filePath] = std::move(newModel);

	return models_[filePath].get();

}

void ModelManager::Finalize()
{

	models_.clear();

}
