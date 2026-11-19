#pragma once
#include <KamataEngine.h>
#include <map>
#include <string>
#include <memory>

class ModelManager {
public:
	static ModelManager* GetInstance();

	
	KamataEngine::Model* GetModel(const std::string& filePath, bool smoothing = false);

	//全モデルの解放
	void Finalize();

private:

	//インスタンス作成禁止
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager&) = delete;//コピー禁止
	ModelManager& operator=(const ModelManager&) = delete;//代入演算子禁止

	std::map<std::string, std::unique_ptr<KamataEngine::Model>> models_;
};