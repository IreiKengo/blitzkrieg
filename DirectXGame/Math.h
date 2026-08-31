#pragma once
#include "KamataEngine.h"



// 円周率
const float PI = 3.141592654f;

struct AABB {
	KamataEngine::Vector3 min;
	KamataEngine::Vector3 max;
};

// 02_14 29枚目 単項演算子オーバーロード
KamataEngine::Vector3 operator+(const KamataEngine::Vector3& v);
KamataEngine::Vector3 operator-(const KamataEngine::Vector3& v);

// 02_06のCameraControllerのUpdate/Reset関数で必要
const KamataEngine::Vector3 operator+(const KamataEngine::Vector3& lhv, const KamataEngine::Vector3& rhv);
const KamataEngine::Vector3 operator-(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);
const KamataEngine::Vector3 operator*(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);
    // 02_06のスライド24枚目のLerp関数
KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2, float t);

// 02_06 スライド29枚目で追加
const KamataEngine::Vector3 operator*(const KamataEngine::Vector3& v1, const float f);
const KamataEngine::Vector3 operator/(const KamataEngine::Vector3& v1, const float f);

// 代入演算子オーバーロード
KamataEngine::Vector3& operator+=(KamataEngine::Vector3& lhs, const KamataEngine::Vector3& rhv);
KamataEngine::Vector3& operator-=(KamataEngine::Vector3& lhs, const KamataEngine::Vector3& rhv);
KamataEngine::Vector3& operator*=(KamataEngine::Vector3& lhv, const KamataEngine::Vector3& rhv);
KamataEngine::Vector3& operator*=(KamataEngine::Vector3& v, float s);
KamataEngine::Vector3& operator/=(KamataEngine::Vector3& v, float s);

// 単位行列の作成
KamataEngine::Matrix4x4 MakeIdentityMatrix();
// スケーリング行列の作成
KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);
// 回転行列の作成
KamataEngine::Matrix4x4 MakeRotateXMatrix(float theta);
KamataEngine::Matrix4x4 MakeRotateYMatrix(float theta);
KamataEngine::Matrix4x4 MakeRotateZMatrix(float theta);
// 平行移動行列の作成
KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate);
// アフィン変換行列の作成
KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rot, const KamataEngine::Vector3& translate);

// 代入演算子オーバーロード
KamataEngine::Matrix4x4& operator*=(KamataEngine::Matrix4x4& lhm, const KamataEngine::Matrix4x4& rhm);

// 2項演算子オーバーロード
KamataEngine::Matrix4x4 operator*(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

// ワールドトランスフォーム更新(02_03の最後)
void WorldTransformUpdate(KamataEngine::WorldTransform& worldTransform);

float Lerp(float x1, float x2, float t);

float EaseIn(float x1, float x2, float t);

float EaseOut(float x1, float x2, float t);

float EaseInOut(float x1, float x2, float t);

bool IsCollision(const AABB& aabb1, const AABB& aabb2);

KamataEngine::Vector3 Transform(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);

// 02_15 で追加
inline float ToRadians(float degrees) { return degrees * (3.1415f / 180.0f); }
inline float ToDegrees(float radians) { return radians * (180.0f / 3.1415f); }

KamataEngine::Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);