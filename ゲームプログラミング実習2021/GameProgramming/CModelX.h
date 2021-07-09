#ifndef CMODELX_H
#define CMODELX_H

#define MODEL_FILE "sample.blend.x" //入力ファイル名


//領域解放をマクロ化
#define SAFE_DELETE_ARRAY(a) { if(a) delete[] a; a = 0;}

#include <vector> //vectorクラスのインクルード(動的配列)
#include "CMatrix.h" //マトリクスクラスのインクルード
#include "CVector.h"

//配列サイズ取得をマクロ化
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

class CModelX; //CModelXクラスの宣言
class CMaterial; //クラスの宣言

/*
CSkinWeights
スキンウェイトクラス
*/
class CSkinWeights{
public:
	char *mpFrameName; //フレーム名
	int mFrameIndex; //フレーム番号
	int mIndexNum; //頂点番号数
	int *mpIndex; //頂点番号配列
	float *mpWeight; //頂点ウェイト配列
	CMatrix mOffset; //オフセットマトリックス

	CSkinWeights(CModelX *model);

	~CSkinWeights(){
		SAFE_DELETE_ARRAY(mpFrameName);
		SAFE_DELETE_ARRAY(mpIndex);
		SAFE_DELETE_ARRAY(mpWeight);
	}
};

//CMeshクラスの定義
class CMesh {
public:
	int mVertexNum; //頂点数
	CVector *mpVertex; //頂点データ
	int mFaceNum; //面数
	int *mpVertexIndex; //面を構成する頂点番号
	int mNormalNum; //法線数
	CVector *mpNormal; //法線データ
	int mMaterialNum; //マテリアル数
	int mMaterialIndexNum; //マテリアル番号数（面数）
	int *mpMaterialIndex; //マテリアル番号
	std::vector<CMaterial*> mMaterial; //マテリアルデータ
	//スキンウェイト
	std::vector<CSkinWeights*> mSkinWeights;
	//コンストラクタ
	CMesh()
		: mVertexNum(0)
		, mpVertex(0)
		, mFaceNum(0)
		, mpVertexIndex(nullptr)
		, mNormalNum(0)
		, mpNormal(nullptr)
		, mMaterialNum(0)
		, mMaterialIndexNum(0)
		, mpMaterialIndex(nullptr)
	{}
	//デストラクタ
	~CMesh(){
		SAFE_DELETE_ARRAY(mpVertex);
		SAFE_DELETE_ARRAY(mpVertexIndex);
		SAFE_DELETE_ARRAY(mpNormal);
		SAFE_DELETE_ARRAY(mpMaterialIndex);
		//スキンウェイトの削除
		for (int i = 0; i < mSkinWeights.size(); i++){
			delete mSkinWeights[i];
		}
	}
	//読み込み処理
	void Init(CModelX *model);

	void Render();
};

//CModelXFrameクラスの定義
class CModelXFrame{
public:
	std::vector<CModelXFrame*> mChild; //子フレームの配列
	CMatrix mTransformMatrix; //変換行列
	CMesh mMesh; //Meshデータ
	char* mpName; //フレーム名前
	int mIndex; //フレーム番号
	//コンストラクタ
	CModelXFrame(CModelX* model);
	//デストラクタ
	~CModelXFrame(){
		//子フレームを全て解放する
		std::vector<CModelXFrame*>::iterator itr;
		for (itr = mChild.begin(); itr != mChild.end(); itr++){
			delete *itr;
		}
		//名前のエリアを解放する
		SAFE_DELETE_ARRAY(mpName);
	}
	void Render();
};

/*
CAnimationKey
アニメーションクラス
*/
class CAnimationKey{
public:
	//時間
	float mTime;
	//行列
	CMatrix mMatrix;
};


/*
CAnimaton
アニメーションクラス
*/
class CAnimation{
public:
	char *mpFrameName;//フレーム名
	int mFrameIndex; //フレーム番号

	CAnimation(CModelX *model);

	~CAnimation(){
		SAFE_DELETE_ARRAY(mpFrameName);
		SAFE_DELETE_ARRAY(mpKey)
	}

	int mKeyNum; //キー数(時間数)
	CAnimationKey *mpKey; //キーの配列
};

/*
CAnimation
アニメーションセット
*/
class CAnimationSet{
public:
	//アニメーションセット
	char *mpName;

	CAnimationSet(CModelX *model);
	//アニメーション
	std::vector<CAnimation*> mAnimation;

	~CAnimationSet(){
		SAFE_DELETE_ARRAY(mpName);
		//アニメーション要素の削除
		for (int i = 0; i < mAnimation.size(); i++){
			delete mAnimation[i];
		}
	}
	float mTime; //現在時間
	float mWeight; //重み
	float mMaxTime; //最大時間
};

/*
CModelX
Xファイル形式の3Dモデルデータをプログラムで認識する
*/
class CModelX{
public:
	char* mpPointer; //読み込み位置
	char mToken[1024]; //取り出した単語の領域
	std::vector<CModelXFrame*> mFrame; //フレームの配列
	//アニメーションセットの配列
	std::vector<CAnimationSet*> mAnimationSet;

	CModelX()
		: mpPointer(0)
	{}

	~CModelX(){
		if (mFrame.size() > 0)
		{
			delete mFrame[0];
		}
		for (int i = 0; i < mAnimationSet.size(); i++){
			delete mAnimationSet[i];
		}
	}

	//ファイル読み込み
	void Load(char* file);
	//単語の取り出し
	void GetToken();
	//ノードの読み飛ばし
	void SkipNode();
	//浮動小数点データの取得
	float GetFloatToken();
	//整数データの取得
	int GetIntToken();

	void Render();

	//フレーム名に該当するフレームのアドレスを返す
	//CModelXFrame* FindFrame(char* name);

	/*
	FindFrame
	フレーム名に該当するフレームのアドレスを返す
	*/
	CModelXFrame* CModelX::FindFrame(char* name){
		//イテレータの作成
		std::vector<CModelXFrame*>::iterator itr;
		//先頭から最後まで繰り返す
		for (itr = mFrame.begin(); itr != mFrame.end(); itr++){
			//名前が一致したか？
			if (strcmp(name, (*itr)->mpName) == 0){
				//一致したらそのアドレスを返す
				return *itr;
			}
		}
		//一致するフレーム無い場合はNULLを返す
		return NULL;
	}

	/*
	AnimateFrame
	フレームの変換行列をアニメーションデータで更新する
	*/
	void CModelX::AnimateFrame(){
		//アニメーションで適用されるフレームの
		//変換行列をゼロクリアする
		for (int i = 0; i < mAnimationSet.size(); i++){
			CAnimationSet* anim = mAnimationSet[i];
			//重みが０は飛ばす
			if (anim->mWeight == 0)continue;
			//フレーム分(Animation分)繰り返す
			for (int j = 0; j < anim->mAnimation.size(); j++){
				CAnimation* animation = anim->mAnimation[j];
				//該当するフレームの変換行列をゼロクリアする
				memset(&mFrame[animation->mFrameIndex]->mTransformMatrix, 0, sizeof(CMatrix));
			}
		}
		//アニメーションも該当するフレームの変換行列を
		//アニメーションのデータで設定する
		for (int i = 0; i < mAnimationSet.size(); i++){
			CAnimationSet* anim = mAnimationSet[i];
			//重みが0は飛ばす
			if (anim->mWeight == 0)continue;
			//フレーム分(Animation分)繰り返す
			for (int j = 0; j < anim->mAnimation.size(); j++){
				//フレームを取得する
				CAnimation* animation = anim->mAnimation[j];
				CModelXFrame* frame = mFrame[animation->mFrameIndex];
				//キーがない場合は飛ばず
				if (animation->mpKey == 0)continue;
				//時間を取得
				float time = anim->mTime;
				//最初の時間より小さい場合
				if (time < animation->mpKey[0].mTime){
					//変換行列を0コマ目の行列で更新
					frame->mTransformMatrix += animation->mpKey[0].mMatrix  * anim->mWeight;
				}
				//最後の時間より大きい場合
				else if (time >= animation->mpKey[animation->mKeyNum - 1].mTime){
					//変換行列を最後のコマの行列で更新
					frame->mTransformMatrix += animation->mpKey[animation->mKeyNum - 1].mMatrix * anim->mWeight;
				}
				else{
					//時間の途中の場合
					for (int k = 1; k < animation->mKeyNum; k++){
						//変換行列を、線形補間にて更新
						if (time < animation->mpKey[k].mTime &&
							animation->mpKey[k - 1].mTime != animation->mpKey[k].mTime){
							float r = (animation->mpKey[k].mTime - time) /
								(animation->mpKey[k].mTime - animation->mpKey[k - 1].mTime);
							frame->mTransformMatrix +=
								(animation->mpKey[k - 1].mMatrix * r + animation->mpKey[k].mMatrix * (1 - r)) * anim->mWeight;
						}
					}
				}
				printf("Frame.%s\n", frame->mpName);
				frame->mTransformMatrix.Print();
			}
		}
	}
};


#endif