#include "DxLib.h"
#include "Geometry.h"
#include <vector>

//画面サイズ
int screensizex = 800;
int screensizey = 600;

//スフィア
std::vector<Sphere> sp;
//床
Plane plane;

//初期化
void Init(){
	//ボールの初期化
	float r = 100;
	Position3 pos = Position3(-120, 50, 0);
	Vector3 albedo = { 0.5f,0.8f,0.8f };
	sp.push_back(Sphere(r, pos, albedo));

	pos = Position3(120, 50, 0); 
	albedo = { 0.8f,0.5f,0.8f };
	sp.push_back(Sphere(r, pos, albedo));

	//床の初期化
	plane.normal = Vector3(0, 1, 0);
	plane.offset = -100;
}

//Draw時に255に直す
void DrawPixelWithFloat(int x, int y, Vector3 color) {
	DrawPixel(x, y, GetColor(color.x * 0xff, color.y * 0xff, color.z * 0xff));
}

//反射ベクトル生成
Vector3 ReflectVector(const Vector3& inVec, const Vector3& normalVec) {
	//反射ベクトル
	Vector3 reflect = inVec - normalVec * ((Dot(inVec, normalVec)) * 2.0);
	return reflect;
}

//１～０に収める
Vector3 Clamp(const Vector3& value, const float minvalue = 0.0f, const float maxvalue = 1.0f) {
	return Vector3(max(min(value.x, maxvalue), minvalue),
		max(min(value.y, maxvalue), minvalue),
		max(min(value.z, maxvalue), minvalue)
	);
}

float Clamp(const float& value, const float minvalue = 0.0f, const float maxvalue = 1.0f) {
	return float(max(min(value, maxvalue), minvalue));
}

//色の合成
Vector3 CaliculateColor(const Vector3& albedo, const float diffuse, const float specular, const float ambient) {
	Vector3 color = Clamp(albedo * (diffuse + ambient) + Vector3(specular, specular, specular));
	return color;
}

//オブジェクト描画判定
bool HitRay(Vector3 eye, int ballnum, Vector3 ray, float &bright, Vector3 &n, Vector3 &p, float &spe) {
	
	//中心までのray
	Vector3 center = sp[ballnum].pos - eye;

	//接線
	Vector3 circleray = ray * Dot(center, ray);

	//垂線
	auto raycirclemag = (center - circleray).Magnitude();

	//垂線を下したところから接点までの距離
	auto w = sqrt(sp[ballnum].radius * sp[ballnum].radius - raycirclemag * raycirclemag);

	//接点
	p = eye + ray * (Dot(center, ray) - w);

	//法線
	n = sp[ballnum].pos - p;

	n.Normalize();

	//光
	Vector3 light(1, -2, 2);

	light.Normalize();

	//輝き
	bright = Dot(light, n);

	//反射ベクトル
	auto lightray = ReflectVector(light, n);

	//スペキュラー
	spe = pow(max(min(Dot(lightray, -ray), 1), 0), 20);

	//クランプ
	bright = max(min(bright, 1), 0);

	//垂線が半径以内に収まってたらtrue
	if (sp[ballnum].radius >= raycirclemag) {
		return true;
	}
	return false;
}

//その他オブジェクトとの当たり判定
bool HitObjeectRay(Vector3 eye, int ballnum, Vector3 ray, float &bright) {
	//中心までのray
	Vector3 center = sp[ballnum].pos - eye;

	//接線
	Vector3 circleray = ray * Dot(center, ray);

	//垂線
	auto raycirclemag = (center - circleray).Magnitude();

	//垂線を下したところから接点までの距離
	auto w = sqrt(sp[ballnum].radius * sp[ballnum].radius - raycirclemag * raycirclemag);

	//接点
	auto p = eye + ray * (Dot(center, ray) - w);

	//法線
	auto n = sp[ballnum].pos - p;

	n.Normalize();

	//光
	Vector3 light(1, -2, 2);

	light.Normalize();

	//輝き
	bright = Dot(light, n);

	//クランプ
	bright = max(min(bright, 1), 0);

	//垂線が半径以内に収まってたらtrue
	if (sp[ballnum].radius >= raycirclemag) {
		return true;
	}
	return false;
}

//床描画判定
bool HitFloorRay(Vector3 ray, Plane plane, Vector3& point) {

	//視点
	Vector3 eye(0, 0, -300);

	plane.normal.Normalize();

	//レイと法線ででた高さ
	auto d = Dot(ray, plane.normal);

	//視線と法線ででた高さ
	auto h = Dot(eye, plane.normal);

	//床の法線からなる高さの比較
	auto distance = (plane.offset - h) / d;
	
	if (distance > 0) {
		//床に当たった点
		point = eye + ray * distance;
		return true;
	}
	else {
		return false;
	}
}

//影描画判定
bool HitFloorShadow(Vector3 point, int ballnum) {

	//光
	Vector3 light(1, -2, 2);

	light.Normalize();

	//引数用変数初期化
	float bright = 0;
	float spe = 0;
	Vector3 normal = Vector3();
	Vector3 hitpos = Vector3();

	//床に当たった点からライトに向けてのレイとオブジェクトの判定
	if (HitRay(point, ballnum, -light, bright, normal, hitpos, spe)) {
		return true;
	}
	else {
		return false;
	}
}

////////////////////////////////////////////////////////////////////////
//描画
////////////////////////////////////////////////////////////////////////
void TraceOn() {
	for (int y = 0; y < screensizey; y++) {
		for (int x = 0; x < screensizex; x++) {

			//座標を真ん中に変換(y反転)
			Vector3 screenpos = Vector3(x - screensizex / 2, -y + screensizey / 2, 0);

			//視点
			Vector3 eye(0, 0, -300);

			//レイができる
			Vector3 ray = screenpos - eye;

			//正規化
			ray.Normalize();

			//輝き
			float bright = 0;

			//specular
			float specular = 0;

			//地面の当たった座標
			Vector3 point = Vector3();

			//色
			Vector3 albedo = Vector3();

			//法線
			Vector3 normal = Vector3();

			//ボールとの接点
			Vector3 hitballpos = Vector3();

			//背景
			if (ray.y >= 0) {
				DrawPixel(x, y, GetColor(0, 225, 255));
			}

			////////////////////////////////////////////////////////////////////////
			//床の描画
			////////////////////////////////////////////////////////////////////////

			if (HitFloorRay(ray, plane , point)) {
				//市松模様
				if (sin(point.x / 50) * cos(point.z / 50) >= 0) {

					for (int i = 0; i < sp.size(); ++i) {
						//影描画
						if (HitFloorShadow(point, i)) {
							DrawPixel(x, y, GetColor(128 * 0.5f, 128 * 0.5f, 128 * 0.5f));
							break;
						}
						else {
							DrawPixel(x, y, GetColor(128, 128, 128));
						}
					}
				}
				else {
					for (int i = 0; i < sp.size(); ++i) {
						//影描画
						if (HitFloorShadow(point, i)) {
							DrawPixel(x, y, GetColor(25 * 0.5f, 25 * 0.5f, 128 * 0.5f));
							break;
						}
						else {
							DrawPixel(x, y, GetColor(25, 25, 128));
						}
					}
				}
			}

			////////////////////////////////////////////////////////////////////////
			//ボールの描画
			////////////////////////////////////////////////////////////////////////

			for (int i = 0; i < sp.size(); ++i) {

				if (HitRay(eye, i, ray, bright,normal,hitballpos, specular)) {

					////////////////////////////////////////////////////////////////////////
					//鏡面反射
					////////////////////////////////////////////////////////////////////////

					auto ref = ReflectVector(ray, normal);

					auto b = 0.0f;

					auto ballray = sp[(i + 1) % sp.size()].pos - sp[i].pos;

					//その他オブジェクト反射
					if (HitObjeectRay(point, (i + 1) % sp.size(), ref, b)) {
						
						if (Dot(ref, ballray) >= 0) {
							auto c = sp[(i + 1) % sp.size()].albedo;
							albedo = sp[i].albedo * c;
							bright = b;
						}
					}

					//床反射
					if (HitFloorRay(ref, plane, point)) {

						//レイを描画
						if (sin(point.x / 50) * cos(point.z / 50) >= 0) {
							//反射している色を乗算
							auto c = Vector3(0.5, 0.5, 0.5);
							albedo = sp[i].albedo * c;
						}
						else {
							//反射している色を乗算
							auto c = Vector3(0.1, 0.1, 0.5);
							albedo = sp[i].albedo * c;
						}
					}
					else {
						albedo = sp[i].albedo;
					}

					//レイを描画
					DrawPixelWithFloat(x, y, CaliculateColor(albedo, Clamp(bright), specular, 0));
					break;
				}
			}
		}
	}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	ChangeWindowMode(true);
	SetGraphMode(screensizex, screensizey,32);
	SetWindowText("Raytracing");

	DxLib_Init();

	//初期化
	Init();

	//描画
	TraceOn();

	WaitKey();

	DxLib_End();
}
