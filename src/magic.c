#include <stdio.h>
#include <AR/ar.h>
#include <AR/param.h>
#include <AR/video.h>
#include <AR/gsub.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "ARMetaseq.h"


/*=========================================================================
【設定項目】ユーザが任意で設定
=========================================================================*/
// ビデオデバイスの設定ファイル
char *vconf_name = (char *)"-dev=/dev/video0";
// カメラパラメータファイル
char *cparam_name = (char *)"Data/camera_para.dat";
// パターンファイル
char *patt_name = (char *)"Data/patt.sample1";
// MQOファイル
char *seq_name = (char *)"Model1/default_%05d.mqo";
// フレーム数
int n_frame = 45;
// 連続描画するときのフレーム数
int e_frame = 9;
// フレームリセットの閾値
int resetcap = 5;
// パターンの中心座標
double patt_center[2] = {0.0, 0.0};
// パターンのサイズ(単位：mm)
double patt_width = 80.0;
// 2値化の閾値
int thresh = 100;


/*=========================================================================
【設定】グローバル変数
=========================================================================*/
int capflag = 0;		// フレームリセットの判定用
int count = 0;			// フレームカウント用
int patt_id;			// パターンのID
double patt_trans[3][4];	// 座標変換行列
MQO_SEQUENCE mqo_seq;		// シーケンス


/*=========================================================================
【プロトタイプ宣言】
=========================================================================*/
void MainLoop(void);
void DrawObject(void);
void MouseEvent(int button, int state, int x, int y);
void KeyEvent(unsigned char key, int x, int y);
void Cleanup(void);
void mySetLight(void);


/*=========================================================================
【関数】 main関数
=========================================================================*/
int main(int argc, char **argv)
{
	ARParam cparam;		// カメラパラメータ
	ARParam wparam;		// カメラパラメータ (作業用変数)
	int xsize, ysize;	// 画像サイズ

	// GLUTの初期化
	glutInit(&argc, argv);

	// ビデオデバイスの設定
	if (arVideoOpen(vconf_name) < 0) {
		printf("ビデオデバイスのエラー\n");
		return -1;
	}

	// カメラパラメータの設定
	if (arVideoInqSize(&xsize, &ysize) < 0) {
		printf("画像サイズを取得できませんでした\n");
		return -1;
	}
	if (arParamLoad(cparam_name, 1, &wparam) < 0) {
		printf("カメラパラメータの読み込みに失敗しました\n");
		return -1;
	}
	arParamChangeSize(&wparam, xsize, ysize, &cparam);
	arInitCparam(&cparam);

	// パターンファイルのロード
	if ((patt_id = arLoadPatt(patt_name)) < 0) {
		printf("パターンファイルの読み込みに失敗しました\n");
		return -1;
	}

	// ウィンドウの設定
	argInit(&cparam, 1.0, 0, 0, 0, 0);

	// GLMetaseqの初期化
	GLMetaseqInitialize();

	// シーケンスの読み込み
	printf("シーケンスの読み込み中...\n");
	mqo_seq = mqoCreateSequence(seq_name, n_frame, 1.0);
	if (mqo_seq.n_frame <= 0) {
		printf("シーケンスの読み込みに失敗しました\n");
		return -1;
	}
	printf("完了\n");

	glClearColor(0.0, 0.0, 0.0, 0.0);
	// ビデオキャプチャの開始
	arVideoCapStart();

	// メインループの開始
	argMainLoop(MouseEvent, KeyEvent, MainLoop);

	return 0;
}


/*=========================================================================
【関数】 メインループ関数
=========================================================================*/
void MainLoop(void)
{
	ARUint8 *image;
	ARMarkerInfo *marker_info;
	int marker_num;
	int j, k;

	// カメラ画像の取得
	if ((image = arVideoGetImage()) == NULL) {
		arUtilSleep(2);
		return;
	}

	// カメラ画像の描画
	argDrawMode2D();
	argDispImage(image, 0, 0);

	// マーカの検出と認識
	if (arDetectMarker(image, thresh, &marker_info, &marker_num) < 0) {
		Cleanup();
		exit(0);
	}

	// 次の画像のキャプチャ指示
	arVideoCapNext();

	// マーカの信頼度の比較
	k = -1;
	for(j = 0; j < marker_num; j++) {
		if (patt_id == marker_info[j].id) {
			if (k == -1) k = j;
			else if (marker_info[k].cf < marker_info[j].cf) k = j;
		}
	}

	if (k != -1) {
		count = 0;
		// マーカの位置・姿勢（座標変換行列）の計算
		arGetTransMat(&marker_info[k], patt_center, patt_width, patt_trans);
		// 3Dオブジェクトの描画
		DrawObject();
	} else {
		count++;
		// マーカーが連続でキャプチャできなければフレームをリセット
		if(count > resetcap) {
			count = 0;
			capflag = 1;
		}
	}

	// バッファの内容を画面に表示
	argSwapBuffers();
}


/*=========================================================================
【関数】 光源の設定を行う関数
=========================================================================*/
void mySetLight(void)
{
	GLfloat light_diffuse[] = { 0.9, 0.9, 0.9, 1.0 };		// 拡散反射光
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };		// 鏡面反射光
	GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 0.1 };		// 環境光
	GLfloat light_position[] = { 100.0, -200.0, 200.0, 0.0 };	// 位置と種類

	// 光源の設定
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);	// 拡散反射光の設定
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);	// 鏡面反射光の設定
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);	// 環境光の設定
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);	// 位置と種類の設定

	glShadeModel(GL_SMOOTH);		// シェーディングの種類の設定
	glEnable(GL_LIGHT0);			// 光源の有効化
}


/*=========================================================================
【関数】 3Dオブジェクトの描画を行う関数
=========================================================================*/
void DrawObject(void)
{
	static int k = 0;			// 描画するフレームの番号
	double gl_para[16];

	// キャプチャの連続を判定
	if(capflag) {
		capflag = 0;
		k = 0;
	}

	// 3Dオブジェクトを描画するための準備
	argDrawMode3D();
	argDraw3dCamera(0, 0);

	// 座標変換行列の適用
	argConvGlpara(patt_trans, gl_para);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(gl_para);

	// 3Dオブジェクトの描画
	glClear(GL_DEPTH_BUFFER_BIT);		// Zバッファの初期化
	glEnable(GL_DEPTH_TEST);		// 隠面処理の適用
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.5);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mySetLight();				// 光源の設定
	glEnable(GL_LIGHTING);			// 光源の適用

	glPushMatrix();
	glRotatef(90.0, 1.0, 0.0, 0.0);		// モデルを立たせる
	mqoCallSequence(mqo_seq, k);		// 指定フレームの描画
	glPopMatrix();

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	// フレーム番号のカウント
	k++;
	if (k >= n_frame) {
		k = e_frame;
	}
}


/*=========================================================================
【関数】 マウス入力処理関数
=========================================================================*/
void MouseEvent(int button, int state, int x, int y)
{
	// 入力状態を表示
	printf("ボタン:%d 状態:%d 座標:(x,y)=(%d,%d) \n", button, state, x, y);
}


/*=========================================================================
【関数】 キーボード入力処理関数
=========================================================================*/
void KeyEvent(unsigned char key, int x, int y)
{
	// ESCキーを入力したらアプリケーション終了
	if (key == 0x1b) {
		Cleanup();
		exit(0);
	}
}


/*=========================================================================
【関数】 終了処理関数
=========================================================================*/
void Cleanup(void)
{
	arVideoCapStop();		// ビデオキャプチャの停止
	arVideoClose();			// ビデオデバイスの終了
	argCleanup();			// グラフィック処理の終了

	mqoDeleteSequence(mqo_seq);	// シーケンスの削除
	GLMetaseqClear();		// 終了処理
}
