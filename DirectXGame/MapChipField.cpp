#include "MapChipField.h"
#include <fstream>
#include <map>
#include <sstream>
#include <cassert>

using namespace KamataEngine;

namespace {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
};

}

void MapChipField::ResetMapChipData() {
	// マップチップデータをリセット
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	// マップチップデータをリセット
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// マップチップCSV
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();

	std::string line;

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {

		getline(mapChipCsv, line);

		// １行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			getline(line_stream, word, ',');

			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const{
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}
	if (yIndex < 0 || kNumBlockVirtical - 1 < yIndex) {
		return MapChipType::kBlock;
	}

	return mapChipData_.data[yIndex][xIndex];
}


Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex)const { return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0); }


MapChipField::IndexSet
MapChipField::GetMapChipIndexSetByPosition(const Vector3& position)const {

	IndexSet indexSet = {};

	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2.0f) / kBlockWidth);
	indexSet.yIndex = kNumBlockVirtical - 1 - static_cast<uint32_t>((position.y + kBlockHeight / 2.0f) / kBlockHeight);

	return indexSet;
}

MapChipField::Rect 
MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) const{ 
	//指定ブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);
	
	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;
	
	return rect; 
}

MapChipField::Rect MapChipField::GetBlockBoundsWorld() const 
{

	uint32_t minX = kNumBlockHorizontal;
	uint32_t maxX = 0;
	uint32_t minY = kNumBlockVirtical;
	uint32_t maxY = 0;
	bool foundBlock = false;

	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			if (GetMapChipTypeByIndex(x, y) == MapChipType::kBlock) {
				foundBlock = true;
				minX = (std::min)(minX, x);
				maxX = (std::max)(maxX, x);
				minY = (std::min)(minY, y);
				maxY = (std::max)(maxY, y);
			}
		}
	}

	Rect rect;
	if (!foundBlock) {
		// ブロックがない場合のデフォルト値
		return Rect{0.0f, 100.0f, 0.0f, 20.0f};
	}

	// インデックスからワールド座標へ変換
	Vector3 minPos = GetMapChipPositionByIndex(minX, maxY);
	Vector3 maxPos = GetMapChipPositionByIndex(maxX, minY);

	rect.left = minPos.x;
	rect.right = maxPos.x;
	rect.bottom = minPos.y;
	rect.top = maxPos.y;

	return rect;

}

float MapChipField::GetBlockCenterYWorld() const 
{
	
	Rect bounds = GetBlockBoundsWorld();
	return (bounds.bottom + bounds.top) * 0.5f;

}
