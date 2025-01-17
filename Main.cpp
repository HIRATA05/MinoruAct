// ３Ｄアクション基本

#include "DxLib.h"
#include <math.h>


// 固定値定義 -------------------------------------------------------------------------------------

// キャラクター共通の情報
#define CHARA_PLAY_ANIM_SPEED		0.5f		// アニメーション速度
#define CHARA_MOVE_SPEED			2.0f		// 移動速度
#define CHARA_ANIM_BLEND_SPEED		0.1f		// アニメーションのブレンド率変化速度
#define CHARA_ANGLE_SPEED			0.2f		// 角度変化速度
#define CHARA_JUMP_POWER			30.0f		// ジャンプ力
#define CHARA_FALL_UP_POWER			10.0f		// 足を踏み外した時のジャンプ力
#define CHARA_GRAVITY				2.0f		// 重力
#define CHARA_MAX_HITCOLL			2048		// 処理するコリジョンポリゴンの最大数
#define CHARA_ENUM_DEFAULT_SIZE		800.0f		// 周囲のポリゴン検出に使用する球の初期サイズ
#define CHARA_HIT_WIDTH				200.0f		// 当たり判定カプセルの半径
#define CHARA_HIT_HEIGHT			700.0f		// 当たり判定カプセルの高さ
#define CHARA_HIT_TRYNUM			16//30		// 壁押し出し処理の最大試行回数
#define CHARA_HIT_SLIDE_LENGTH		5.0f		// 一度の壁押し出し処理でスライドさせる距離
#define CHARA_HIT_PUSH_POWER		40.0f		// キャラクター同士で当たったときの押し出される力
#define CHARA_SHADOW_SIZE			200.0f		// 影の大きさ
#define CHARA_SHADOW_HEIGHT			700.0f		// 影が落ちる高さ

// カメラ関係の定義
#define CAMERA_ANGLE_SPEED			0.05f		// 旋回速度
#define CAMERA_PLAYER_TARGET_HEIGHT		400.0f		// プレイヤー座標からどれだけ高い位置を注視点とするか
#define CAMERA_PLAYER_LENGTH			2000.0f		// プレイヤーとの距離
#define CAMERA_COLLISION_SIZE			50.0f		// カメラの当たり判定サイズ

// プレイヤーキャラ以外キャラの情報
#define NOTPLAYER_NUM				4			// プレイヤー以外のキャラの数
#define NOTPLAYER_MOVETIME			120			// 一つの方向に移動する時間
#define NOTPLAYER_JUMPRATIO			250			// プレイヤー以外のキャラがジャンプする確率


// 構造体定義 -------------------------------------------------------------------------------------

// 入力情報構造体
struct PADINPUT
{
	int		NowInput;				// 現在の入力
	int		EdgeInput;				// 現在のフレームで押されたボタンのみビットが立っている入力値
};

// キャラクター情報
struct CHARA
{
	VECTOR		Position;				// 座標
	VECTOR		TargetMoveDirection;			// モデルが向くべき方向のベクトル
	float		Angle;					// モデルが向いている方向の角度
	float		JumpPower;				// Ｙ軸方向の速度
	int		ModelHandle;				// モデルハンドル
	int		State;					// 状態( 0:立ち止まり  1:走り  2:ジャンプ )

	int		PlayAnim1;				// 再生しているアニメーション１のアタッチ番号( -1:何もアニメーションがアタッチされていない )
	float		AnimPlayCount1;			// 再生しているアニメーション１の再生時間
	int		PlayAnim2;				// 再生しているアニメーション２のアタッチ番号( -1:何もアニメーションがアタッチされていない )
	float		AnimPlayCount2;			// 再生しているアニメーション２の再生時間
	float		AnimBlendRate;				// 再生しているアニメーション１と２のブレンド率
};

// プレイヤー情報構造体
struct PLAYER
{
	CHARA	CharaInfo;					// キャラクター情報
};

// プレイヤー以外キャラ情報構造体
struct NOTPLAYER
{
	CHARA	CharaInfo;					// キャラクター情報
	int			MoveTime;				// 移動時間
	float		MoveAngle;				// 移動方向
};

// キャラクター共通情報
struct CHARA_COMMON
{
	int		BaseModelHandle;		// 共通の派生元モデルハンドル
	int		ShadowHandle;			// 影描画用のグラフィックハンドル
};
/*
// ステージ情報構造体
struct STAGE
{
	int		ModelHandle;				// モデルハンドル
};
*/
// カメラ情報構造体
struct CAMERA
{
	float		AngleH;				// 水平角度
	float		AngleV;				// 垂直角度
	VECTOR		Eye;					// カメラ座標
	VECTOR		Target;				// 注視点座標
};

//
const int stageNum = 5;
// ステージ情報構造体
struct STAGE
{
	//ステージのモデルハンドル配列
	int ModelHandle_StageFloor;

	VECTOR StagePosition = VGet(0.0f, 0.0f, 0.0f);

	//クリアフラグ
	bool clearflg = false;
	//既に触れたか
	bool checkflg = false;
};
float floor_x = 0.0f, floor_y = 0.0f, floor_z = 0.0f;
float blockSize = 4000.0f;  // ブロックのサイズ
VECTOR StagePosition;
int clearNum = 0;

//落下制限数値
float rakkaY = -1000.0f;

//ゲームクリアフラグ
bool GameClearFlg = false;
int clearScene_time = 0;

//音声
int BGMSound;
int ClearBGMSound;
int ScreamSESound;
//

// 関数プロトタイプ宣言 ---------------------------------------------------------------------------

void Input_Process(void);			// 入力処理

void Chara_Initialize(CHARA* ch, VECTOR Position);						// キャラクターの初期化
void Chara_Terminate(CHARA* ch);											// キャラクターの後始末
void Chara_Process(CHARA* ch, VECTOR MoveVec, int JumpFlag);				// キャラクターの処理
//void Chara_Move(CHARA* ch, VECTOR MoveVector);							// キャラクターの移動処理
//void Chara_Move(CHARA* ch, VECTOR MoveVector, STAGE stg);							// キャラクターの移動処理
void Chara_Move(CHARA* ch, VECTOR MoveVector, int num1, int num2);							// キャラクターの移動処理
void Chara_Collision(CHARA* ch, VECTOR* ch_MoveVec, CHARA* chk_ch);		// キャラクターに当たっていたら押し出す処理を行う( chk_ch に ch が当たっていたら ch が離れる )
void Chara_AngleProcess(CHARA* ch);										// キャラクターの向きを変える処理
void Chara_PlayAnim(CHARA* ch, int PlayAnim);							// キャラクターに新たなアニメーションを再生する
void Chara_AnimProcess(CHARA* ch);										// キャラクターのアニメーション処理
void Chara_ShadowRender(CHARA* ch);										// キャラクターの影を描画

void Player_Initialize(void);		// プレイヤーの初期化
void Player_Terminate(void);			// プレイヤーの後始末
void Player_Process(void);			// プレイヤーの処理

void Stage_Initialize(void);			// ステージの初期化処理
void Stage_Terminate(void);			// ステージの後始末処理

void Camera_Initialize(void);		// カメラの初期化処理
void Camera_Process(void);			// カメラの処理

void Render_Process(void);			// 描画処理

//
void Clear_Check(void);			//クリア判定処理
void GameClearScene(void);			//クリアシーン処理
void Sound_Initialize(void);		// 音声の初期化
//

// 実体宣言 ---------------------------------------------------------------------------------------

PADINPUT inp;		// 入力情報の実体宣言
PLAYER pl;		// プレイヤー情報の実体宣言
NOTPLAYER npl[NOTPLAYER_NUM];	// プレイヤーではないキャラの実体宣言
CHARA_COMMON chcmn;		// キャラクターの共通情報の実体宣言
//STAGE stg;		// ステージ情報の実体宣言
CAMERA cam;		// カメラ情報の実体宣言

//
STAGE stg[stageNum][stageNum];		// ステージ情報の実体宣言
//


// 関数実体 ---------------------------------------------------------------------------------------

// 入力処理
void Input_Process(void)
{
	int Old;

	// ひとつ前のフレームの入力を変数にとっておく
	Old = inp.NowInput;

	// 現在の入力状態を取得
	inp.NowInput = GetJoypadInputState(DX_INPUT_KEY_PAD1);

	// 今のフレームで新たに押されたボタンのビットだけ立っている値を EdgeInput に代入する
	inp.EdgeInput = inp.NowInput & ~Old;
}

// キャラクターの共通情報を初期化
void CharaCommon_Initialize(void)
{
	// モデルの読み込み
	chcmn.BaseModelHandle = MV1LoadModel("Joni/VJoni.pmx");

	// 影描画用の画像の読み込み
	chcmn.ShadowHandle = LoadGraph("Shadow.tga");
}

// キャラクター共通情報の後始末
void CharaCommon_Terminate(void)
{
	// モデルの削除
	MV1DeleteModel(chcmn.BaseModelHandle);

	// 影用画像の削除
	DeleteGraph(chcmn.ShadowHandle);
}

// キャラクターの初期化
void Chara_Initialize(CHARA* ch, VECTOR Position)
{
	// 初期座標は原点
	ch->Position = Position;

	// 回転値は０
	ch->Angle = 0.0f;

	// ジャンプ力は初期状態では０
	ch->JumpPower = 0.0f;

	// モデルハンドルの作成
	ch->ModelHandle = MV1DuplicateModel(chcmn.BaseModelHandle);

	MV1SetScale(ch->ModelHandle, VGet(40.0f, 40.0f, 40.0f));

	// 初期状態では「立ち止り」状態
	ch->State = 0;

	// 初期状態はＸ軸方向
	ch->TargetMoveDirection = VGet(1.0f, 0.0f, 0.0f);

	// アニメーションのブレンド率を初期化
	ch->AnimBlendRate = 1.0f;

	// 初期状態ではアニメーションはアタッチされていないにする
	ch->PlayAnim1 = -1;
	ch->PlayAnim2 = -1;

	// ただ立っているアニメーションを再生
	Chara_PlayAnim(ch, 4);
}

// キャラクターの後始末
void Chara_Terminate(CHARA* ch)
{
	// モデルの削除
	MV1DeleteModel(ch->ModelHandle);
}

// キャラクターの処理
void Chara_Process(CHARA* ch, VECTOR MoveVec, int JumpFlag)
{
	int MoveFlag;			// 移動したかどうかのフラグ( 1:移動した  0:移動していない )

	// ルートフレームのＺ軸方向の移動パラメータを無効にする
	{
		MATRIX LocalMatrix;

		// ユーザー行列を解除する
		MV1ResetFrameUserLocalMatrix(ch->ModelHandle, 2);

		// 現在のルートフレームの行列を取得する
		LocalMatrix = MV1GetFrameLocalMatrix(ch->ModelHandle, 2);

		// Ｚ軸方向の平行移動成分を無効にする
		LocalMatrix.m[3][2] = 0.0f;

		// ユーザー行列として平行移動成分を無効にした行列をルートフレームにセットする
		MV1SetFrameUserLocalMatrix(ch->ModelHandle, 2, LocalMatrix);
	}

	// 移動したかどうかのフラグをセット、少しでも移動していたら「移動している」を表す１にする
	MoveFlag = 0;
	if (MoveVec.x < -0.001f || MoveVec.x > 0.001f ||
		MoveVec.y < -0.001f || MoveVec.y > 0.001f ||
		MoveVec.z < -0.001f || MoveVec.z > 0.001f)
	{
		MoveFlag = 1;
	}

	// キャラクターの状態が「ジャンプ」ではなく、且つジャンプフラグが立っていたらジャンプする
	if (ch->State != 2 && JumpFlag == 1)
	{
		// 状態を「ジャンプ」にする
		ch->State = 2;

		// Ｙ軸方向の速度をセット
		ch->JumpPower = CHARA_JUMP_POWER;

		// ジャンプアニメーションの再生
		Chara_PlayAnim(ch, 2);
	}

	// 移動ボタンが押されたかどうかで処理を分岐
	if (MoveFlag)
	{
		// 移動ベクトルを正規化したものをキャラクターが向くべき方向として保存
		ch->TargetMoveDirection = VNorm(MoveVec);

		// もし今まで「立ち止まり」状態だったら
		if (ch->State == 0)
		{
			// 走りアニメーションを再生する
			Chara_PlayAnim(ch, 1);

			// 状態を「走り」にする
			ch->State = 1;
		}
	}
	else
	{
		// このフレームで移動していなくて、且つ状態が「走り」だったら
		if (ch->State == 1)
		{
			// 立ち止りアニメーションを再生する
			Chara_PlayAnim(ch, 4);

			// 状態を「立ち止り」にする
			ch->State = 0;
		}
	}

	// 状態が「ジャンプ」の場合は
	if (ch->State == 2)
	{
		// Ｙ軸方向の速度を重力分減算する
		ch->JumpPower -= CHARA_GRAVITY;

		// もし落下していて且つ再生されているアニメーションが上昇中用のものだった場合は
		if (ch->JumpPower < 0.0f && MV1GetAttachAnim(ch->ModelHandle, ch->PlayAnim1) == 2)
		{
			// 落下中ようのアニメーションを再生する
			Chara_PlayAnim(ch, 3);
		}

		// 移動ベクトルのＹ成分をＹ軸方向の速度にする
		MoveVec.y = ch->JumpPower;
	}

	// キャラクターの移動方向にモデルの方向を近づける
	Chara_AngleProcess(ch);

	// 移動ベクトルを元にコリジョンを考慮しつつキャラクターを移動
	//Chara_Move(ch, MoveVec);

	//
	for (int i = 0; i <= stageNum - 1; i++) {

		for (int j = 0; j <= stageNum - 1; j++)
		{
			// 移動ベクトルを元にコリジョンを考慮しつつキャラクターを移動
			//Chara_Move(ch, MoveVec, stg[i][j]);
			Chara_Move(ch, MoveVec, i, j);
		}
	}
	//

	// アニメーション処理
	Chara_AnimProcess(ch);
}

// キャラクターの移動処理
//void Chara_Move(CHARA* ch, VECTOR MoveVector)
//void Chara_Move(CHARA* ch, VECTOR MoveVector, STAGE stg)
void Chara_Move(CHARA* ch, VECTOR MoveVector, int num1, int num2)
{
	int i, j, k;						// 汎用カウンタ変数
	int MoveFlag;						// 水平方向に移動したかどうかのフラグ( ０:移動していない  １:移動した )
	int HitFlag;						// ポリゴンに当たったかどうかを記憶しておくのに使う変数( ０:当たっていない  １:当たった )
	MV1_COLL_RESULT_POLY_DIM HitDim;			// キャラクターの周囲にあるポリゴンを検出した結果が代入される当たり判定結果構造体
	int KabeNum;						// 壁ポリゴンと判断されたポリゴンの数
	int YukaNum;						// 床ポリゴンと判断されたポリゴンの数
	MV1_COLL_RESULT_POLY* Kabe[CHARA_MAX_HITCOLL];	// 壁ポリゴンと判断されたポリゴンの構造体のアドレスを保存しておくためのポインタ配列
	MV1_COLL_RESULT_POLY* Yuka[CHARA_MAX_HITCOLL];	// 床ポリゴンと判断されたポリゴンの構造体のアドレスを保存しておくためのポインタ配列
	MV1_COLL_RESULT_POLY* Poly;				// ポリゴンの構造体にアクセスするために使用するポインタ( 使わなくても済ませられますがプログラムが長くなるので・・・ )
	HITRESULT_LINE LineRes;				// 線分とポリゴンとの当たり判定の結果を代入する構造体
	VECTOR OldPos;						// 移動前の座標	
	VECTOR NowPos;						// 移動後の座標
	
	// 移動前の座標を保存
	OldPos = ch->Position;

	// 移動後の座標を算出
	NowPos = VAdd(ch->Position, MoveVector);

	// キャラクターの周囲にあるステージポリゴンを取得する
	// ( 検出する範囲は移動距離も考慮する )
	//HitDim = MV1CollCheck_Sphere(stg.ModelHandle, -1, ch->Position, CHARA_ENUM_DEFAULT_SIZE + VSize(MoveVector));
	HitDim = MV1CollCheck_Sphere(stg[num1][num2].ModelHandle_StageFloor, -1, ch->Position, CHARA_ENUM_DEFAULT_SIZE + VSize(MoveVector));

	// x軸かz軸方向に 0.01f 以上移動した場合は「移動した」フラグを１にする
	if (fabs(MoveVector.x) > 0.01f || fabs(MoveVector.z) > 0.01f)
	{
		MoveFlag = 1;
	}
	else
	{
		MoveFlag = 0;
	}

	// 検出されたポリゴンが壁ポリゴン( ＸＺ平面に垂直なポリゴン )か床ポリゴン( ＸＺ平面に垂直ではないポリゴン )かを判断する
	{
		// 壁ポリゴンと床ポリゴンの数を初期化する
		KabeNum = 0;
		YukaNum = 0;

		// 検出されたポリゴンの数だけ繰り返し
		for (i = 0; i < HitDim.HitNum; i++)
		{
			// ＸＺ平面に垂直かどうかはポリゴンの法線のＹ成分が０に限りなく近いかどうかで判断する
			if (HitDim.Dim[i].Normal.y < 0.000001f && HitDim.Dim[i].Normal.y > -0.000001f)
			{
				// 壁ポリゴンと判断された場合でも、キャラクターのＹ座標＋１．０ｆより高いポリゴンのみ当たり判定を行う
				if (HitDim.Dim[i].Position[0].y > ch->Position.y + 1.0f ||
					HitDim.Dim[i].Position[1].y > ch->Position.y + 1.0f ||
					HitDim.Dim[i].Position[2].y > ch->Position.y + 1.0f)
				{
					// ポリゴンの数が列挙できる限界数に達していなかったらポリゴンを配列に追加
					if (KabeNum < CHARA_MAX_HITCOLL)
					{
						// ポリゴンの構造体のアドレスを壁ポリゴンポインタ配列に保存する
						Kabe[KabeNum] = &HitDim.Dim[i];

						// 壁ポリゴンの数を加算する
						KabeNum++;
					}
				}
			}
			else
			{
				// ポリゴンの数が列挙できる限界数に達していなかったらポリゴンを配列に追加
				if (YukaNum < CHARA_MAX_HITCOLL)
				{
					// ポリゴンの構造体のアドレスを床ポリゴンポインタ配列に保存する
					Yuka[YukaNum] = &HitDim.Dim[i];

					// 床ポリゴンの数を加算する
					YukaNum++;
				}
			}
		}
	}

	// 壁ポリゴンとの当たり判定処理
	if (KabeNum != 0)
	{
		// 壁に当たったかどうかのフラグは初期状態では「当たっていない」にしておく
		HitFlag = 0;

		// 移動したかどうかで処理を分岐
		if (MoveFlag == 1)
		{
			// 壁ポリゴンの数だけ繰り返し
			for (i = 0; i < KabeNum; i++)
			{
				// i番目の壁ポリゴンのアドレスを壁ポリゴンポインタ配列から取得
				Poly = Kabe[i];

				// ポリゴンとキャラクターが当たっていなかったら次のカウントへ
				if (HitCheck_Capsule_Triangle(NowPos, VAdd(NowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, Poly->Position[0], Poly->Position[1], Poly->Position[2]) == FALSE) continue;

				// ここにきたらポリゴンとキャラクターが当たっているということなので、ポリゴンに当たったフラグを立てる
				HitFlag = 1;

				// 壁に当たったら壁に遮られない移動成分分だけ移動する
				{
					VECTOR SlideVec;	// キャラクターをスライドさせるベクトル

					// 進行方向ベクトルと壁ポリゴンの法線ベクトルに垂直なベクトルを算出
					SlideVec = VCross(MoveVector, Poly->Normal);

					// 算出したベクトルと壁ポリゴンの法線ベクトルに垂直なベクトルを算出、これが
					// 元の移動成分から壁方向の移動成分を抜いたベクトル
					SlideVec = VCross(Poly->Normal, SlideVec);

					// それを移動前の座標に足したものを新たな座標とする
					NowPos = VAdd(OldPos, SlideVec);
				}

				// 新たな移動座標で壁ポリゴンと当たっていないかどうかを判定する
				for (j = 0; j < KabeNum; j++)
				{
					// j番目の壁ポリゴンのアドレスを壁ポリゴンポインタ配列から取得
					Poly = Kabe[j];

					// 当たっていたらループから抜ける
					if (HitCheck_Capsule_Triangle(NowPos, VAdd(NowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, Poly->Position[0], Poly->Position[1], Poly->Position[2]) == TRUE) break;
				}

				// j が KabeNum だった場合はどのポリゴンとも当たらなかったということなので
				// 壁に当たったフラグを倒した上でループから抜ける
				if (j == KabeNum)
				{
					HitFlag = 0;
					break;
				}
			}
		}
		else
		{
			// 移動していない場合の処理

			// 壁ポリゴンの数だけ繰り返し
			for (i = 0; i < KabeNum; i++)
			{
				// i番目の壁ポリゴンのアドレスを壁ポリゴンポインタ配列から取得
				Poly = Kabe[i];

				// ポリゴンに当たっていたら当たったフラグを立てた上でループから抜ける
				if (HitCheck_Capsule_Triangle(NowPos, VAdd(NowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, Poly->Position[0], Poly->Position[1], Poly->Position[2]) == TRUE)
				{
					HitFlag = 1;
					break;
				}
			}
		}

		// 壁に当たっていたら壁から押し出す処理を行う
		if (HitFlag == 1)
		{
			// 壁からの押し出し処理を試みる最大数だけ繰り返し
			for (k = 0; k < CHARA_HIT_TRYNUM; k++)
			{
				// 壁ポリゴンの数だけ繰り返し
				for (i = 0; i < KabeNum; i++)
				{
					// i番目の壁ポリゴンのアドレスを壁ポリゴンポインタ配列から取得
					Poly = Kabe[i];

					// キャラクターと当たっているかを判定
					if (HitCheck_Capsule_Triangle(NowPos, VAdd(NowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, Poly->Position[0], Poly->Position[1], Poly->Position[2]) == FALSE) continue;

					// 当たっていたら規定距離分キャラクターを壁の法線方向に移動させる
					NowPos = VAdd(NowPos, VScale(Poly->Normal, CHARA_HIT_SLIDE_LENGTH));

					// 移動した上で壁ポリゴンと接触しているかどうかを判定
					for (j = 0; j < KabeNum; j++)
					{
						// 当たっていたらループを抜ける
						Poly = Kabe[j];
						if (HitCheck_Capsule_Triangle(NowPos, VAdd(NowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, Poly->Position[0], Poly->Position[1], Poly->Position[2]) == TRUE) break;
					}

					// 全てのポリゴンと当たっていなかったらここでループ終了
					if (j == KabeNum) break;
				}

				// i が KabeNum ではない場合は全部のポリゴンで押し出しを試みる前に全ての壁ポリゴンと接触しなくなったということなのでループから抜ける
				if (i != KabeNum) break;
			}
		}
	}

	// 床ポリゴンとの当たり判定
	if (YukaNum != 0)
	{
		// ジャンプ中且つ上昇中の場合は処理を分岐
		if (ch->State == 2 && ch->JumpPower > 0.0f)
		{
			float MinY;

			// 天井に頭をぶつける処理を行う

			// 一番低い天井にぶつける為の判定用変数を初期化
			MinY = 0.0f;

			// 当たったかどうかのフラグを当たっていないを意味する０にしておく
			HitFlag = 0;

			// 床ポリゴンの数だけ繰り返し
			for (i = 0; i < YukaNum; i++)
			{
				// i番目の床ポリゴンのアドレスを床ポリゴンポインタ配列から取得
				Poly = Yuka[i];

				// 足先から頭の高さまでの間でポリゴンと接触しているかどうかを判定
				LineRes = HitCheck_Line_Triangle(NowPos, VAdd(NowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), Poly->Position[0], Poly->Position[1], Poly->Position[2]);

				// 接触していなかったら何もしない
				if (LineRes.HitFlag == FALSE) continue;

				// 既にポリゴンに当たっていて、且つ今まで検出した天井ポリゴンより高い場合は何もしない
				if (HitFlag == 1 && MinY < LineRes.Position.y) continue;

				// ポリゴンに当たったフラグを立てる
				HitFlag = 1;

				// 接触したＹ座標を保存する
				MinY = LineRes.Position.y;
			}

			// 接触したポリゴンがあったかどうかで処理を分岐
			if (HitFlag == 1)
			{
				// 接触した場合はキャラクターのＹ座標を接触座標を元に更新
				NowPos.y = MinY - CHARA_HIT_HEIGHT;

				// Ｙ軸方向の速度は反転
				ch->JumpPower = -ch->JumpPower;
			}
		}
		else
		{
			float MaxY;

			// 下降中かジャンプ中ではない場合の処理

			// 床ポリゴンに当たったかどうかのフラグを倒しておく
			HitFlag = 0;

			// 一番高い床ポリゴンにぶつける為の判定用変数を初期化
			MaxY = 0.0f;

			// 床ポリゴンの数だけ繰り返し
			for (i = 0; i < YukaNum; i++)
			{
				// i番目の床ポリゴンのアドレスを床ポリゴンポインタ配列から取得
				Poly = Yuka[i];

				// ジャンプ中かどうかで処理を分岐
				if (ch->State == 2)
				{
					// ジャンプ中の場合は頭の先から足先より少し低い位置の間で当たっているかを判定
					LineRes = HitCheck_Line_Triangle(VAdd(NowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), VAdd(NowPos, VGet(0.0f, -1.0f, 0.0f)), Poly->Position[0], Poly->Position[1], Poly->Position[2]);
				}
				else
				{
					// 走っている場合は頭の先からそこそこ低い位置の間で当たっているかを判定( 傾斜で落下状態に移行してしまわない為 )
					LineRes = HitCheck_Line_Triangle(VAdd(NowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), VAdd(NowPos, VGet(0.0f, -40.0f, 0.0f)), Poly->Position[0], Poly->Position[1], Poly->Position[2]);
				}

				// 当たっていなかったら何もしない
				if (LineRes.HitFlag == FALSE) continue;

				// 既に当たったポリゴンがあり、且つ今まで検出した床ポリゴンより低い場合は何もしない
				if (HitFlag == 1 && MaxY > LineRes.Position.y) continue;

				// ポリゴンに当たったフラグを立てる
				HitFlag = 1;
				
				// 接触したＹ座標を保存する
				MaxY = LineRes.Position.y;
			}

			// 床ポリゴンに当たったかどうかで処理を分岐
			if (HitFlag == 1)
			{
				// 当たった場合

				//
				//足場に触れたフラグをTrue
				stg[num1][num2].clearflg = true;
				//stg.checkflg = true;
				//

				// 接触したポリゴンで一番高いＹ座標をキャラクターのＹ座標にする
				NowPos.y = MaxY;

				// Ｙ軸方向の移動速度は０に
				ch->JumpPower = 0.0f;

				// もしジャンプ中だった場合は着地状態にする
				if (ch->State == 2)
				{
					// 移動していたかどうかで着地後の状態と再生するアニメーションを分岐する
					if (MoveFlag)
					{
						// 移動している場合は走り状態に
						Chara_PlayAnim(ch, 1);
						ch->State = 1;
					}
					else
					{
						// 移動していない場合は立ち止り状態に
						Chara_PlayAnim(ch, 4);
						ch->State = 0;
					}

					// 着地時はアニメーションのブレンドは行わない
					ch->AnimBlendRate = 1.0f;
				}
			}
			else
			{
				
				// 床コリジョンに当たっていなくて且つジャンプ状態ではなかった場合は
				if (ch->State != 2)
				{
					// ジャンプ中にする
					ch->State = 2;

					// ちょっとだけジャンプする
					//ch->JumpPower = CHARA_FALL_UP_POWER;

					// アニメーションは落下中のものにする
					Chara_PlayAnim(ch, 3);
				}
				
			}
		}
	}

	// 新しい座標を保存する
	ch->Position = NowPos;

	// キャラクターのモデルの座標を更新する
	MV1SetPosition(ch->ModelHandle, ch->Position);

	// 検出したキャラクターの周囲のポリゴン情報を開放する
	MV1CollResultPolyDimTerminate(HitDim);
	
}

// キャラクターに当たっていたら押し出す処理を行う( chk_ch に ch が当たっていたら ch が離れる )
void Chara_Collision(CHARA* ch, VECTOR* ch_MoveVec, CHARA* chk_ch)
{
	VECTOR ChkChToChVec;
	VECTOR PushVec;
	VECTOR ChPosition;
	float Length;

	// 移動後の ch の座標を算出
	ChPosition = VAdd(ch->Position, *ch_MoveVec);

	// 当たっていなかったら何もしない
	if (HitCheck_Capsule_Capsule(
		ChPosition, VAdd(ChPosition, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH,
		chk_ch->Position, VAdd(chk_ch->Position, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH) == TRUE)
	{
		// 当たっていたら ch が chk から離れる処理をする

		// chk_ch から ch へのベクトルを算出
		ChkChToChVec = VSub(ChPosition, chk_ch->Position);

		// Ｙ軸は見ない
		ChkChToChVec.y = 0.0f;

		// 二人の距離を算出
		Length = VSize(ChkChToChVec);

		// chk_ch から ch へのベクトルを正規化( ベクトルの長さを 1.0f にする )
		PushVec = VScale(ChkChToChVec, 1.0f / Length);

		// 押し出す距離を算出、もし二人の距離から二人の大きさを引いた値に押し出し力を足して離れてしまう場合は、ぴったりくっつく距離に移動する
		if (Length - CHARA_HIT_WIDTH * 2.0f + CHARA_HIT_PUSH_POWER > 0.0f)
		{
			float TempY;

			TempY = ChPosition.y;
			ChPosition = VAdd(chk_ch->Position, VScale(PushVec, CHARA_HIT_WIDTH * 2.0f));

			// Ｙ座標は変化させない
			ChPosition.y = TempY;
		}
		else
		{
			// 押し出し
			ChPosition = VAdd(ChPosition, VScale(PushVec, CHARA_HIT_PUSH_POWER));
		}
	}

	// 当たり判定処理後の移動ベクトルをセット
	*ch_MoveVec = VSub(ChPosition, ch->Position);
}

// キャラクターの向きを変える処理
void Chara_AngleProcess(CHARA* ch)
{
	float TargetAngle;			// 目標角度
	float SaAngle;				// 目標角度と現在の角度との差

	// 目標の方向ベクトルから角度値を算出する
	TargetAngle = atan2(ch->TargetMoveDirection.x, ch->TargetMoveDirection.z);

	// 目標の角度と現在の角度との差を割り出す
	{
		// 最初は単純に引き算
		SaAngle = TargetAngle - ch->Angle;

		// ある方向からある方向の差が１８０度以上になることは無いので
		// 差の値が１８０度以上になっていたら修正する
		if (SaAngle < -DX_PI_F)
		{
			SaAngle += DX_TWO_PI_F;
		}
		else
			if (SaAngle > DX_PI_F)
			{
				SaAngle -= DX_TWO_PI_F;
			}
	}

	// 角度の差が０に近づける
	if (SaAngle > 0.0f)
	{
		// 差がプラスの場合は引く
		SaAngle -= CHARA_ANGLE_SPEED;
		if (SaAngle < 0.0f)
		{
			SaAngle = 0.0f;
		}
	}
	else
	{
		// 差がマイナスの場合は足す
		SaAngle += CHARA_ANGLE_SPEED;
		if (SaAngle > 0.0f)
		{
			SaAngle = 0.0f;
		}
	}

	// モデルの角度を更新
	ch->Angle = TargetAngle - SaAngle;
	MV1SetRotationXYZ(ch->ModelHandle, VGet(0.0f, ch->Angle + DX_PI_F, 0.0f));
}

// キャラクターに新たなアニメーションを再生する
void Chara_PlayAnim(CHARA* ch, int PlayAnim)
{
	// 再生中のモーション２が有効だったらデタッチする
	if (ch->PlayAnim2 != -1)
	{
		MV1DetachAnim(ch->ModelHandle, ch->PlayAnim2);
		ch->PlayAnim2 = -1;
	}

	// 今まで再生中のモーション１だったものの情報を２に移動する
	ch->PlayAnim2 = ch->PlayAnim1;
	ch->AnimPlayCount2 = ch->AnimPlayCount1;

	// 新たに指定のモーションをモデルにアタッチして、アタッチ番号を保存する
	ch->PlayAnim1 = MV1AttachAnim(ch->ModelHandle, PlayAnim);
	ch->AnimPlayCount1 = 0.0f;

	// ブレンド率は再生中のモーション２が有効ではない場合は１．０ｆ( 再生中のモーション１が１００％の状態 )にする
	ch->AnimBlendRate = ch->PlayAnim2 == -1 ? 1.0f : 0.0f;
}

// キャラクターのアニメーション処理
void Chara_AnimProcess(CHARA* ch)
{
	float AnimTotalTime;		// 再生しているアニメーションの総時間

	// ブレンド率が１以下の場合は１に近づける
	if (ch->AnimBlendRate < 1.0f)
	{
		ch->AnimBlendRate += CHARA_ANIM_BLEND_SPEED;
		if (ch->AnimBlendRate > 1.0f)
		{
			ch->AnimBlendRate = 1.0f;
		}
	}

	// 再生しているアニメーション１の処理
	if (ch->PlayAnim1 != -1)
	{
		// アニメーションの総時間を取得
		AnimTotalTime = MV1GetAttachAnimTotalTime(ch->ModelHandle, ch->PlayAnim1);

		// 再生時間を進める
		ch->AnimPlayCount1 += CHARA_PLAY_ANIM_SPEED;

		// 再生時間が総時間に到達していたら再生時間をループさせる
		if (ch->AnimPlayCount1 >= AnimTotalTime)
		{
			ch->AnimPlayCount1 = fmod(ch->AnimPlayCount1, AnimTotalTime);
		}

		// 変更した再生時間をモデルに反映させる
		MV1SetAttachAnimTime(ch->ModelHandle, ch->PlayAnim1, ch->AnimPlayCount1);

		// アニメーション１のモデルに対する反映率をセット
		MV1SetAttachAnimBlendRate(ch->ModelHandle, ch->PlayAnim1, ch->AnimBlendRate);
	}

	// 再生しているアニメーション２の処理
	if (ch->PlayAnim2 != -1)
	{
		// アニメーションの総時間を取得
		AnimTotalTime = MV1GetAttachAnimTotalTime(ch->ModelHandle, ch->PlayAnim2);

		// 再生時間を進める
		ch->AnimPlayCount2 += CHARA_PLAY_ANIM_SPEED;

		// 再生時間が総時間に到達していたら再生時間をループさせる
		if (ch->AnimPlayCount2 > AnimTotalTime)
		{
			ch->AnimPlayCount2 = fmod(ch->AnimPlayCount2, AnimTotalTime);
		}

		// 変更した再生時間をモデルに反映させる
		MV1SetAttachAnimTime(ch->ModelHandle, ch->PlayAnim2, ch->AnimPlayCount2);

		// アニメーション２のモデルに対する反映率をセット
		MV1SetAttachAnimBlendRate(ch->ModelHandle, ch->PlayAnim2, 1.0f - ch->AnimBlendRate);
	}
}

// キャラクターの影を描画
void Chara_ShadowRender(CHARA* ch)
{
	int i;
	MV1_COLL_RESULT_POLY_DIM HitResDim;
	MV1_COLL_RESULT_POLY* HitRes;
	VERTEX3D Vertex[3];
	VECTOR SlideVec;

	// ライティングを無効にする
	SetUseLighting(FALSE);

	// Ｚバッファを有効にする
	SetUseZBuffer3D(TRUE);

	// テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);
	/*
	// ライティングを無効にする
	SetUseLighting(FALSE);

	// Ｚバッファを有効にする
	SetUseZBuffer3D(TRUE);

	// テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	// キャラクターの直下に存在する地面のポリゴンを取得
	HitResDim = MV1CollCheck_Capsule(stg.ModelHandle, -1, ch->Position, VAdd(ch->Position, VGet(0.0f, -CHARA_SHADOW_HEIGHT, 0.0f)), CHARA_SHADOW_SIZE);

	// 頂点データで変化が無い部分をセット
	Vertex[0].dif = GetColorU8(255, 255, 255, 255);
	Vertex[0].spc = GetColorU8(0, 0, 0, 0);
	Vertex[0].su = 0.0f;
	Vertex[0].sv = 0.0f;
	Vertex[1] = Vertex[0];
	Vertex[2] = Vertex[0];

	// 球の直下に存在するポリゴンの数だけ繰り返し
	HitRes = HitResDim.Dim;
	for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
	{
		// ポリゴンの座標は地面ポリゴンの座標
		Vertex[0].pos = HitRes->Position[0];
		Vertex[1].pos = HitRes->Position[1];
		Vertex[2].pos = HitRes->Position[2];

		// ちょっと持ち上げて重ならないようにする
		SlideVec = VScale(HitRes->Normal, 0.5f);
		Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
		Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
		Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

		// ポリゴンの不透明度を設定する
		Vertex[0].dif.a = 0;
		Vertex[1].dif.a = 0;
		Vertex[2].dif.a = 0;
		if (HitRes->Position[0].y > ch->Position.y - CHARA_SHADOW_HEIGHT)
			Vertex[0].dif.a = 128 * (1.0f - fabs(HitRes->Position[0].y - ch->Position.y) / CHARA_SHADOW_HEIGHT);

		if (HitRes->Position[1].y > ch->Position.y - CHARA_SHADOW_HEIGHT)
			Vertex[1].dif.a = 128 * (1.0f - fabs(HitRes->Position[1].y - ch->Position.y) / CHARA_SHADOW_HEIGHT);

		if (HitRes->Position[2].y > ch->Position.y - CHARA_SHADOW_HEIGHT)
			Vertex[2].dif.a = 128 * (1.0f - fabs(HitRes->Position[2].y - ch->Position.y) / CHARA_SHADOW_HEIGHT);

		// ＵＶ値は地面ポリゴンとキャラクターの相対座標から割り出す
		Vertex[0].u = (HitRes->Position[0].x - ch->Position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		Vertex[0].v = (HitRes->Position[0].z - ch->Position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		Vertex[1].u = (HitRes->Position[1].x - ch->Position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		Vertex[1].v = (HitRes->Position[1].z - ch->Position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		Vertex[2].u = (HitRes->Position[2].x - ch->Position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		Vertex[2].v = (HitRes->Position[2].z - ch->Position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;

		// 影ポリゴンを描画
		DrawPolygon3D(Vertex, 1, chcmn.ShadowHandle, TRUE);
	}

	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(HitResDim);

	// ライティングを有効にする
	SetUseLighting(TRUE);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(FALSE);
	*/

	//
	for (int i = 0; i <= stageNum - 1; i++) {

		for (int j = 0; j <= stageNum - 1; j++)
		{
			// キャラクターの直下に存在する地面のポリゴンを取得
			HitResDim = MV1CollCheck_Capsule(stg[i][j].ModelHandle_StageFloor, -1, ch->Position, VAdd(ch->Position, VGet(0.0f, -CHARA_SHADOW_HEIGHT, 0.0f)), CHARA_SHADOW_SIZE);

			// 頂点データで変化が無い部分をセット
			Vertex[0].dif = GetColorU8(255, 255, 255, 255);
			Vertex[0].spc = GetColorU8(0, 0, 0, 0);
			Vertex[0].su = 0.0f;
			Vertex[0].sv = 0.0f;
			Vertex[1] = Vertex[0];
			Vertex[2] = Vertex[0];

			// 球の直下に存在するポリゴンの数だけ繰り返し
			HitRes = HitResDim.Dim;
			for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
			{
				// ポリゴンの座標は地面ポリゴンの座標
				Vertex[0].pos = HitRes->Position[0];
				Vertex[1].pos = HitRes->Position[1];
				Vertex[2].pos = HitRes->Position[2];

				// ちょっと持ち上げて重ならないようにする
				SlideVec = VScale(HitRes->Normal, 0.5f);
				Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
				Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
				Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

				// ポリゴンの不透明度を設定する
				Vertex[0].dif.a = 0;
				Vertex[1].dif.a = 0;
				Vertex[2].dif.a = 0;
				if (HitRes->Position[0].y > ch->Position.y - CHARA_SHADOW_HEIGHT)
					Vertex[0].dif.a = 128 * (1.0f - fabs(HitRes->Position[0].y - ch->Position.y) / CHARA_SHADOW_HEIGHT);

				if (HitRes->Position[1].y > ch->Position.y - CHARA_SHADOW_HEIGHT)
					Vertex[1].dif.a = 128 * (1.0f - fabs(HitRes->Position[1].y - ch->Position.y) / CHARA_SHADOW_HEIGHT);

				if (HitRes->Position[2].y > ch->Position.y - CHARA_SHADOW_HEIGHT)
					Vertex[2].dif.a = 128 * (1.0f - fabs(HitRes->Position[2].y - ch->Position.y) / CHARA_SHADOW_HEIGHT);

				// ＵＶ値は地面ポリゴンとキャラクターの相対座標から割り出す
				Vertex[0].u = (HitRes->Position[0].x - ch->Position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
				Vertex[0].v = (HitRes->Position[0].z - ch->Position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
				Vertex[1].u = (HitRes->Position[1].x - ch->Position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
				Vertex[1].v = (HitRes->Position[1].z - ch->Position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
				Vertex[2].u = (HitRes->Position[2].x - ch->Position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
				Vertex[2].v = (HitRes->Position[2].z - ch->Position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;

				// 影ポリゴンを描画
				DrawPolygon3D(Vertex, 1, chcmn.ShadowHandle, TRUE);
			}

			// 検出した地面ポリゴン情報の後始末
			MV1CollResultPolyDimTerminate(HitResDim);

		}
	}
	// ライティングを有効にする
	SetUseLighting(TRUE);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(FALSE);
	//
}


// プレイヤーの初期化
void Player_Initialize(void)
{
	// キャラクター情報を初期化
	Chara_Initialize(&pl.CharaInfo, VGet(0.0f, 0.0f, 0.0f));
}

// プレイヤーの後始末
void Player_Terminate(void)
{
	// キャラクター情報の後始末
	Chara_Terminate(&pl.CharaInfo);
}

// プレイヤーの処理
void Player_Process(void)
{
	VECTOR UpMoveVec;		// 方向ボタン「↑」を入力をしたときのプレイヤーの移動方向ベクトル
	VECTOR LeftMoveVec;		// 方向ボタン「←」を入力をしたときのプレイヤーの移動方向ベクトル
	VECTOR MoveVec;		// このフレームの移動ベクトル
	int JumpFlag;			// ジャンプフラグ
	int i;

	// プレイヤーの移動方向のベクトルを算出
	{
		// 方向ボタン「↑」を押したときのプレイヤーの移動ベクトルはカメラの視線方向からＹ成分を抜いたもの
		UpMoveVec = VSub(cam.Target, cam.Eye);
		UpMoveVec.y = 0.0f;

		// 方向ボタン「←」を押したときのプレイヤーの移動ベクトルは上を押したときの方向ベクトルとＹ軸のプラス方向のベクトルに垂直な方向
		LeftMoveVec = VCross(UpMoveVec, VGet(0.0f, 1.0f, 0.0f));

		// 二つのベクトルを正規化( ベクトルの長さを１．０にすること )
		UpMoveVec = VNorm(UpMoveVec);
		LeftMoveVec = VNorm(LeftMoveVec);
	}

	// このフレームでの移動ベクトルを初期化
	MoveVec = VGet(0.0f, 0.0f, 0.0f);

	// ジャンプフラグを倒す
	JumpFlag = 0;

	// パッドの３ボタンと左シフトがどちらも押されていなかったらプレイヤーの移動処理
	if (CheckHitKey(KEY_INPUT_LSHIFT) == 0 && (inp.NowInput & PAD_INPUT_C) == 0)
	{
		// 方向ボタン「←」が入力されたらカメラの見ている方向から見て左方向に移動する
		if (inp.NowInput & PAD_INPUT_LEFT)
		{
			// 移動ベクトルに「←」が入力された時の移動ベクトルを加算する
			MoveVec = VAdd(MoveVec, LeftMoveVec);
		}
		else
			// 方向ボタン「→」が入力されたらカメラの見ている方向から見て右方向に移動する
			if (inp.NowInput & PAD_INPUT_RIGHT)
			{
				// 移動ベクトルに「←」が入力された時の移動ベクトルを反転したものを加算する
				MoveVec = VAdd(MoveVec, VScale(LeftMoveVec, -1.0f));
			}

		// 方向ボタン「↑」が入力されたらカメラの見ている方向に移動する
		if (inp.NowInput & PAD_INPUT_UP)
		{
			// 移動ベクトルに「↑」が入力された時の移動ベクトルを加算する
			MoveVec = VAdd(MoveVec, UpMoveVec);
		}
		else
			// 方向ボタン「↓」が入力されたらカメラの方向に移動する
			if (inp.NowInput & PAD_INPUT_DOWN)
			{
				// 移動ベクトルに「↑」が入力された時の移動ベクトルを反転したものを加算する
				MoveVec = VAdd(MoveVec, VScale(UpMoveVec, -1.0f));
			}

		// ボタン１が押されていたらジャンプフラグを立てる
		if (inp.EdgeInput & PAD_INPUT_A)
		{
			JumpFlag = 1;
		}
	}

	// 移動方向を移動速度でスケーリングする
	MoveVec = VScale(MoveVec, CHARA_MOVE_SPEED);

	/**/
	// プレイヤーキャラ以外との当たり判定を行う
	for (i = 0; i < NOTPLAYER_NUM; i++)
	{
		Chara_Collision(&pl.CharaInfo, &MoveVec, &npl[i].CharaInfo);
	}
	

	//
	//プレイヤー落下時の位置修正
	if (pl.CharaInfo.Position.y < -5000.0f) {
		//落下したら原点からY100の位置に移動
		pl.CharaInfo.Position = VGet(0.0f, 500.0f, 0.0f);

		//落下時のSE再生
		PlaySoundMem(ScreamSESound, DX_PLAYTYPE_BACK);
	}
	//

	// キャラクターを動作させる処理を行う
	Chara_Process(&pl.CharaInfo, MoveVec, JumpFlag);
}

// プレイヤー以外キャラの初期化
void NotPlayer_Initialize(void)
{
	int i;
	static VECTOR FirstPosition[NOTPLAYER_NUM] =
	{
		{ -3000.0f, 0.0f, 2300.0f },
		{ -2500.0f, 7300.0f, -2500.0f },
		{ -2600.0f, 0.0f, -3100.0f },
		{  2800.0f, 0.0f, 200.0f },
	};

	for (i = 0; i < NOTPLAYER_NUM; i++)
	{
		// キャラクター情報を初期化
		Chara_Initialize(&npl[i].CharaInfo, FirstPosition[i]);

		// 移動時間を初期化
		npl[i].MoveTime = 0;

		// 移動方向を初期化
		npl[i].MoveAngle = GetRand(1000) * DX_PI_F * 2.0f / 1000.0f;
	}
}

// プレイヤー以外キャラの後始末
void NotPlayer_Terminate(void)
{
	int i;

	// キャラクタの数だけ繰り返し
	for (i = 0; i < NOTPLAYER_NUM; i++)
	{
		// キャラクター情報の後始末
		Chara_Terminate(&npl[i].CharaInfo);
	}
}

// プレイヤー以外キャラの処理
void NotPlayer_Process(void)
{
	int i;
	int j;
	VECTOR MoveVec;
	int JumpFlag;

	// キャラクタの数だけ繰り返し
	for (i = 0; i < NOTPLAYER_NUM; i++)
	{
		// ジャンプフラグを倒しておく
		JumpFlag = 0;

		// 一定時間が経過したら移動する方向を変更する
		npl[i].MoveTime++;
		if (npl[i].MoveTime >= NOTPLAYER_MOVETIME)
		{
			npl[i].MoveTime = 0;

			// 新しい方向の決定
			npl[i].MoveAngle = GetRand(1000) * DX_PI_F * 2.0f / 1000.0f;

			// 一定確率でジャンプする
			if (GetRand(1000) < NOTPLAYER_JUMPRATIO)
			{
				JumpFlag = 1;
			}
		}

		// 新しい方向の角度からベクトルを算出
		MoveVec.x = cos(npl[i].MoveAngle) * CHARA_MOVE_SPEED;
		MoveVec.y = 0.0f;
		MoveVec.z = sin(npl[i].MoveAngle) * CHARA_MOVE_SPEED;

		// プレイヤーとの当たり判定を行う
		Chara_Collision(&npl[i].CharaInfo, &MoveVec, &pl.CharaInfo);

		// 自分以外のプレイヤーキャラとの当たり判定を行う
		for (j = 0; j < NOTPLAYER_NUM; j++)
		{
			// 自分との当たり判定はしない
			if (i == j) continue;

			Chara_Collision(&npl[i].CharaInfo, &MoveVec, &npl[j].CharaInfo);
		}

		// 移動処理を行う
		Chara_Process(&npl[i].CharaInfo, MoveVec, JumpFlag);
		
	}
}

// ステージの初期化処理
void Stage_Initialize(void)
{
	/*
	// ステージモデルの読み込み
	stg.ModelHandle = MV1LoadModel("ColTestStage.mqo");

	//stg.ModelHandle = MV1LoadModel("Stage/medieval_castle.x");
	//stg.ModelHandle = MV1LoadModel("StageBlock/GreenCube.mv1");
	MV1SetPosition(stg.ModelHandle, VGet(0.0f, 0.0f, 0.0f));

	// ３ＤモデルのY軸の回転値を９０度にセットする
	//MV1SetRotationXYZ(stg.ModelHandle, VGet(-90.0f * DX_PI_F / 180.0f, 180.0f, 0.0f));
	//MV1SetScale(stg.ModelHandle, VGet(10.0f, 1.0f, 10.0f));

	// モデル全体のコリジョン情報のセットアップ
	MV1SetupCollInfo(stg.ModelHandle, -1);
	*/

	//
	stg[0][0].ModelHandle_StageFloor = MV1LoadModel("StageBlock/GreenCube.mv1");
	
	//配列の数だけ配置
	for (int i = 0; i <= stageNum - 1; i++) {

		for (int j = 0; j <= stageNum - 1; j++)
		{
			// ブロックを描画する座標を計算する
			floor_x = static_cast<float>(j) * blockSize;
			floor_y = -100.0f;
			floor_z = static_cast<float>(i) * blockSize;

			StagePosition = VGet(floor_x, floor_y, floor_z);

			//ステージのブロックを配列で順番に配置 XY200の正方形
			stg[i][j].ModelHandle_StageFloor = MV1DuplicateModel(stg[0][0].ModelHandle_StageFloor);
			
			MV1SetPosition(stg[i][j].ModelHandle_StageFloor, StagePosition);  // 座標を設定

			MV1SetScale(stg[i][j].ModelHandle_StageFloor, VGet(18.0f, 18.0f, 1.0f));

			// ３ＤモデルのY軸の回転値を９０度にセットする
			MV1SetRotationXYZ(stg[i][j].ModelHandle_StageFloor, VGet(-90.0f * DX_PI_F / 180.0f, 0.0f, 0.0f));
			//MV1DrawModel(stg[i][j].ModelHandle_StageFloor);  // 描画
			
			// モデル全体のコリジョン情報のセットアップ
			MV1SetupCollInfo(stg[i][j].ModelHandle_StageFloor, -1);
		}
	}
	//
}

// ステージの後始末処理
void Stage_Terminate(void)
{
	// ステージモデルの後始末
	//MV1DeleteModel(stg.ModelHandle);

	//
	for (int i = 0; i <= stageNum - 1; i++) {

		for (int j = 0; j <= stageNum - 1; j++)
		{
			// ステージモデルの後始末
			MV1DeleteModel(stg[i][j].ModelHandle_StageFloor);
		}
	}
	//
}

// カメラの初期化処理
void Camera_Initialize(void)
{
	// カメラの初期水平角度は１８０度
	cam.AngleH = DX_PI_F;

	// 垂直角度は０度
	cam.AngleV = 0.0f;
}

// カメラの処理
void Camera_Process(void)
{
	// パッドの３ボタンか、シフトキーが押されている場合のみ角度変更操作を行う
	if (CheckHitKey(KEY_INPUT_LSHIFT) || (inp.NowInput & PAD_INPUT_C))
	{
		// 「←」ボタンが押されていたら水平角度をマイナスする
		if (inp.NowInput & PAD_INPUT_LEFT)
		{
			cam.AngleH -= CAMERA_ANGLE_SPEED;

			// −１８０度以下になったら角度値が大きくなりすぎないように３６０度を足す
			if (cam.AngleH < -DX_PI_F)
			{
				cam.AngleH += DX_TWO_PI_F;
			}
		}

		// 「→」ボタンが押されていたら水平角度をプラスする
		if (inp.NowInput & PAD_INPUT_RIGHT)
		{
			cam.AngleH += CAMERA_ANGLE_SPEED;

			// １８０度以上になったら角度値が大きくなりすぎないように３６０度を引く
			if (cam.AngleH > DX_PI_F)
			{
				cam.AngleH -= DX_TWO_PI_F;
			}
		}

		// 「↑」ボタンが押されていたら垂直角度をマイナスする
		if (inp.NowInput & PAD_INPUT_UP)
		{
			cam.AngleV -= CAMERA_ANGLE_SPEED;

			// ある一定角度以下にはならないようにする
			if (cam.AngleV < -DX_PI_F / 2.0f + 0.6f)
			{
				cam.AngleV = -DX_PI_F / 2.0f + 0.6f;
			}
		}

		// 「↓」ボタンが押されていたら垂直角度をプラスする
		if (inp.NowInput & PAD_INPUT_DOWN)
		{
			cam.AngleV += CAMERA_ANGLE_SPEED;

			// ある一定角度以上にはならないようにする
			if (cam.AngleV > DX_PI_F / 2.0f - 0.6f)
			{
				cam.AngleV = DX_PI_F / 2.0f - 0.6f;
			}
		}
	}

	// カメラの注視点はプレイヤー座標から規定値分高い座標
	cam.Target = VAdd(pl.CharaInfo.Position, VGet(0.0f, CAMERA_PLAYER_TARGET_HEIGHT, 0.0f));

	// カメラの座標を決定する
	{
		MATRIX RotZ, RotY;
		float Camera_Player_Length;
		MV1_COLL_RESULT_POLY_DIM HRes;
		int HitNum;

		// 水平方向の回転はＹ軸回転
		RotY = MGetRotY(cam.AngleH);

		// 垂直方向の回転はＺ軸回転 )
		RotZ = MGetRotZ(cam.AngleV);

		// カメラからプレイヤーまでの初期距離をセット
		Camera_Player_Length = CAMERA_PLAYER_LENGTH;

		// カメラの座標を算出
		// Ｘ軸にカメラとプレイヤーとの距離分だけ伸びたベクトルを
		// 垂直方向回転( Ｚ軸回転 )させたあと水平方向回転( Ｙ軸回転 )して更に
		// 注視点の座標を足したものがカメラの座標
		cam.Eye = VAdd(VTransform(VTransform(VGet(-Camera_Player_Length, 0.0f, 0.0f), RotZ), RotY), cam.Target);

		/*
		// 注視点からカメラの座標までの間にステージのポリゴンがあるか調べる
		HRes = MV1CollCheck_Capsule(stg.ModelHandle, -1, cam.Target, cam.Eye, CAMERA_COLLISION_SIZE);
		HitNum = HRes.HitNum;
		MV1CollResultPolyDimTerminate(HRes);
		if (HitNum != 0)
		{
			float NotHitLength;
			float HitLength;
			float TestLength;
			VECTOR TestPosition;

			// あったら無い位置までプレイヤーに近づく

			// ポリゴンに当たらない距離をセット
			NotHitLength = 0.0f;

			// ポリゴンに当たる距離をセット
			HitLength = Camera_Player_Length;
			do
			{
				// 当たるかどうかテストする距離をセット( 当たらない距離と当たる距離の中間 )
				TestLength = NotHitLength + (HitLength - NotHitLength) / 2.0f;

				// テスト用のカメラ座標を算出
				TestPosition = VAdd(VTransform(VTransform(VGet(-TestLength, 0.0f, 0.0f), RotZ), RotY), cam.Target);

				// 新しい座標で壁に当たるかテスト
				HRes = MV1CollCheck_Capsule(stg.ModelHandle, -1, cam.Target, TestPosition, CAMERA_COLLISION_SIZE);
				HitNum = HRes.HitNum;
				MV1CollResultPolyDimTerminate(HRes);
				if (HitNum != 0)
				{
					// 当たったら当たる距離を TestLength に変更する
					HitLength = TestLength;
				}
				else
				{
					// 当たらなかったら当たらない距離を TestLength に変更する
					NotHitLength = TestLength;
				}

				// HitLength と NoHitLength が十分に近づいていなかったらループ
			} while (HitLength - NotHitLength > 0.1f);

			// カメラの座標をセット
			cam.Eye = TestPosition;
		}
		*/

		//
		for (int i = 0; i <= stageNum - 1; i++) {

			for (int j = 0; j <= stageNum - 1; j++)
			{
				// 注視点からカメラの座標までの間にステージのポリゴンがあるか調べる
				HRes = MV1CollCheck_Capsule(stg[i][j].ModelHandle_StageFloor, -1, cam.Target, cam.Eye, CAMERA_COLLISION_SIZE);
				HitNum = HRes.HitNum;
				MV1CollResultPolyDimTerminate(HRes);
				if (HitNum != 0)
				{
					float NotHitLength;
					float HitLength;
					float TestLength;
					VECTOR TestPosition;

					// あったら無い位置までプレイヤーに近づく

					// ポリゴンに当たらない距離をセット
					NotHitLength = 0.0f;

					// ポリゴンに当たる距離をセット
					HitLength = Camera_Player_Length;
					do
					{
						// 当たるかどうかテストする距離をセット( 当たらない距離と当たる距離の中間 )
						TestLength = NotHitLength + (HitLength - NotHitLength) / 2.0f;

						// テスト用のカメラ座標を算出
						TestPosition = VAdd(VTransform(VTransform(VGet(-TestLength, 0.0f, 0.0f), RotZ), RotY), cam.Target);

						// 新しい座標で壁に当たるかテスト
						HRes = MV1CollCheck_Capsule(stg[i][j].ModelHandle_StageFloor, -1, cam.Target, TestPosition, CAMERA_COLLISION_SIZE);
						HitNum = HRes.HitNum;
						MV1CollResultPolyDimTerminate(HRes);
						if (HitNum != 0)
						{
							// 当たったら当たる距離を TestLength に変更する
							HitLength = TestLength;
						}
						else
						{
							// 当たらなかったら当たらない距離を TestLength に変更する
							NotHitLength = TestLength;
						}

						// HitLength と NoHitLength が十分に近づいていなかったらループ
					} while (HitLength - NotHitLength > 0.1f);

					// カメラの座標をセット
					cam.Eye = TestPosition;
				}
			}
		}
		//
	}

	// カメラの情報をライブラリのカメラに反映させる
	SetCameraPositionAndTarget_UpVecY(cam.Eye, cam.Target);
}

// 描画処理
void Render_Process(void)
{
	int i;

	// ステージモデルの描画
	//MV1DrawModel(stg.ModelHandle);

	//
	//配列の数だけブロックを描画
	for (int i = 0; i <= stageNum - 1; i++) {

		for (int j = 0; j <= stageNum - 1; j++)
		{
			MV1DrawModel(stg[i][j].ModelHandle_StageFloor);
		}
	}
	//

	// プレイヤーモデルの描画
	MV1DrawModel(pl.CharaInfo.ModelHandle);

	/*
	// プレイヤー以外キャラモデルの描画
	for (i = 0; i < NOTPLAYER_NUM; i++)
	{
		MV1DrawModel(npl[i].CharaInfo.ModelHandle);
	}
	
	// プレイヤーの影の描画
	Chara_ShadowRender(&pl.CharaInfo);

	// プレイヤー以外キャラの影の描画
	for (i = 0; i < NOTPLAYER_NUM; i++)
	{
		Chara_ShadowRender(&npl[i].CharaInfo);
	}
	*/

	//
	DrawFormatString(0, 100, GetColor(255, 255, 255), "操作\n移動-方向キー\nカメラ回転-Shift+方向キー\nジャンプ-Zキー");
	//
}

//クリア判定の処理
void Clear_Check() {
	//配列の数だけ調べる
	for (int i = 0; i <= stageNum - 1; i++) {

		for (int j = 0; j <= stageNum - 1; j++)
		{
			//クリアチェック
			if (/**/stg[i][j].clearflg  && !stg[i][j].checkflg) {
				clearNum++;
				stg[i][j].checkflg = true;
				//DrawFormatString(200, 200, GetColor(255, 255, 255), "チェック");
			}
		}
	}
	//クリア数がステージ足場数と同じか
	if (clearNum == stageNum * stageNum) {
		GameClearFlg = true;
	}
	DrawFormatString(0, 0, GetColor(255, 255, 255), "すべての床を踏め:%d" ,clearNum);
}
//

//ゲームクリア時の処理
void GameClearScene() {
	//BGMのループ再生
	PlaySoundMem(ClearBGMSound, DX_PLAYTYPE_LOOP);

	DrawFormatString(300, 100, GetColor(255, 255, 255), "ゲームクリア");

	DrawFormatString(200, 240, GetColor(255, 255, 255), "ESCAPEキーを押して終了してください");
}
//

//音声の初期化
void Sound_Initialize() {
	//音声のロード
	BGMSound = LoadSoundMem("Sound/StageBGM.mp3");
	ClearBGMSound = LoadSoundMem("Sound/ClearBGM.mp3");
	ScreamSESound = LoadSoundMem("Sound/MinoruScream.mp3");

	//BGMのループ再生
	PlaySoundMem(BGMSound, DX_PLAYTYPE_LOOP);
}
//

// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ウインドウモードで起動
	ChangeWindowMode(TRUE);

	// ライブラリの初期化
	if (DxLib_Init() < 0)
		return -1;

	//
	//背景の色を変更
	SetBackgroundColor(50, 180, 255);
	//

	// キャラクターの共通情報を初期化
	CharaCommon_Initialize();

	// プレイヤーの初期化
	Player_Initialize();

	// プレイヤー以外キャラの初期化
	NotPlayer_Initialize();

	// ステージの初期化
	Stage_Initialize();

	// カメラの初期化
	Camera_Initialize();

	//音声の初期化
	Sound_Initialize();

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// ＥＳＣキーが押されるか、ウインドウが閉じられるまでループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面をクリア
		ClearDrawScreen();

		
		if (!GameClearFlg) {
			// 入力処理
			Input_Process();

			// プレイヤー以外キャラの処理
			NotPlayer_Process();

			// プレイヤーの処理
			Player_Process();

			// カメラの処理
			Camera_Process();

			// 描画処理
			Render_Process();

			//
			//クリア判定確認
			Clear_Check();
			//
		}
		else if (GameClearFlg) {
			GameClearScene();
		}

		// 裏画面の内容を表画面に反映
		ScreenFlip();
	}

	// プレイヤー以外キャラの後始末
	NotPlayer_Terminate();

	// プレイヤーの後始末
	Player_Terminate();

	// キャラクター共通情報の後始末
	CharaCommon_Terminate();

	// ステージの後始末
	Stage_Terminate();

	// ライブラリの後始末
	DxLib_End();

	// ソフト終了
	return 0;
}
