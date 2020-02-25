#include "DxLib.h"
#include "Geometry.h"
#include <vector>

//��ʃT�C�Y
int screensizex = 800;
int screensizey = 600;

//�X�t�B�A
std::vector<Sphere> sp;
//��
Plane plane;

//������
void Init(){
	//�{�[���̏�����
	float r = 100;
	Position3 pos = Position3(-120, 50, 0);
	Vector3 albedo = { 0.5f,0.8f,0.8f };
	sp.push_back(Sphere(r, pos, albedo));

	pos = Position3(120, 50, 0); 
	albedo = { 0.8f,0.5f,0.8f };
	sp.push_back(Sphere(r, pos, albedo));

	//���̏�����
	plane.normal = Vector3(0, 1, 0);
	plane.offset = -100;
}

//Draw����255�ɒ���
void DrawPixelWithFloat(int x, int y, Vector3 color) {
	DrawPixel(x, y, GetColor(color.x * 0xff, color.y * 0xff, color.z * 0xff));
}

//���˃x�N�g������
Vector3 ReflectVector(const Vector3& inVec, const Vector3& normalVec) {
	//���˃x�N�g��
	Vector3 reflect = inVec - normalVec * ((Dot(inVec, normalVec)) * 2.0);
	return reflect;
}

//�P�`�O�Ɏ��߂�
Vector3 Clamp(const Vector3& value, const float minvalue = 0.0f, const float maxvalue = 1.0f) {
	return Vector3(max(min(value.x, maxvalue), minvalue),
		max(min(value.y, maxvalue), minvalue),
		max(min(value.z, maxvalue), minvalue)
	);
}

float Clamp(const float& value, const float minvalue = 0.0f, const float maxvalue = 1.0f) {
	return float(max(min(value, maxvalue), minvalue));
}

//�F�̍���
Vector3 CaliculateColor(const Vector3& albedo, const float diffuse, const float specular, const float ambient) {
	Vector3 color = Clamp(albedo * (diffuse + ambient) + Vector3(specular, specular, specular));
	return color;
}

//�I�u�W�F�N�g�`�攻��
bool HitRay(Vector3 eye, int ballnum, Vector3 ray, float &bright, Vector3 &n, Vector3 &p, float &spe) {
	
	//���S�܂ł�ray
	Vector3 center = sp[ballnum].pos - eye;

	//�ڐ�
	Vector3 circleray = ray * Dot(center, ray);

	//����
	auto raycirclemag = (center - circleray).Magnitude();

	//�������������Ƃ��납��ړ_�܂ł̋���
	auto w = sqrt(sp[ballnum].radius * sp[ballnum].radius - raycirclemag * raycirclemag);

	//�ړ_
	p = eye + ray * (Dot(center, ray) - w);

	//�@��
	n = sp[ballnum].pos - p;

	n.Normalize();

	//��
	Vector3 light(1, -2, 2);

	light.Normalize();

	//�P��
	bright = Dot(light, n);

	//���˃x�N�g��
	auto lightray = ReflectVector(light, n);

	//�X�y�L�����[
	spe = pow(max(min(Dot(lightray, -ray), 1), 0), 20);

	//�N�����v
	bright = max(min(bright, 1), 0);

	//���������a�ȓ��Ɏ��܂��Ă���true
	if (sp[ballnum].radius >= raycirclemag) {
		return true;
	}
	return false;
}

//���̑��I�u�W�F�N�g�Ƃ̓����蔻��
bool HitObjeectRay(Vector3 eye, int ballnum, Vector3 ray, float &bright) {
	//���S�܂ł�ray
	Vector3 center = sp[ballnum].pos - eye;

	//�ڐ�
	Vector3 circleray = ray * Dot(center, ray);

	//����
	auto raycirclemag = (center - circleray).Magnitude();

	//�������������Ƃ��납��ړ_�܂ł̋���
	auto w = sqrt(sp[ballnum].radius * sp[ballnum].radius - raycirclemag * raycirclemag);

	//�ړ_
	auto p = eye + ray * (Dot(center, ray) - w);

	//�@��
	auto n = sp[ballnum].pos - p;

	n.Normalize();

	//��
	Vector3 light(1, -2, 2);

	light.Normalize();

	//�P��
	bright = Dot(light, n);

	//�N�����v
	bright = max(min(bright, 1), 0);

	//���������a�ȓ��Ɏ��܂��Ă���true
	if (sp[ballnum].radius >= raycirclemag) {
		return true;
	}
	return false;
}

//���`�攻��
bool HitFloorRay(Vector3 ray, Plane plane, Vector3& point) {

	//���_
	Vector3 eye(0, 0, -300);

	plane.normal.Normalize();

	//���C�Ɩ@���łł�����
	auto d = Dot(ray, plane.normal);

	//�����Ɩ@���łł�����
	auto h = Dot(eye, plane.normal);

	//���̖@������Ȃ鍂���̔�r
	auto distance = (plane.offset - h) / d;
	
	if (distance > 0) {
		//���ɓ��������_
		point = eye + ray * distance;
		return true;
	}
	else {
		return false;
	}
}

//�e�`�攻��
bool HitFloorShadow(Vector3 point, int ballnum) {

	//��
	Vector3 light(1, -2, 2);

	light.Normalize();

	//�����p�ϐ�������
	float bright = 0;
	float spe = 0;
	Vector3 normal = Vector3();
	Vector3 hitpos = Vector3();

	//���ɓ��������_���烉�C�g�Ɍ����Ẵ��C�ƃI�u�W�F�N�g�̔���
	if (HitRay(point, ballnum, -light, bright, normal, hitpos, spe)) {
		return true;
	}
	else {
		return false;
	}
}

////////////////////////////////////////////////////////////////////////
//�`��
////////////////////////////////////////////////////////////////////////
void TraceOn() {
	for (int y = 0; y < screensizey; y++) {
		for (int x = 0; x < screensizex; x++) {

			//���W��^�񒆂ɕϊ�(y���])
			Vector3 screenpos = Vector3(x - screensizex / 2, -y + screensizey / 2, 0);

			//���_
			Vector3 eye(0, 0, -300);

			//���C���ł���
			Vector3 ray = screenpos - eye;

			//���K��
			ray.Normalize();

			//�P��
			float bright = 0;

			//specular
			float specular = 0;

			//�n�ʂ̓����������W
			Vector3 point = Vector3();

			//�F
			Vector3 albedo = Vector3();

			//�@��
			Vector3 normal = Vector3();

			//�{�[���Ƃ̐ړ_
			Vector3 hitballpos = Vector3();

			//�w�i
			if (ray.y >= 0) {
				DrawPixel(x, y, GetColor(0, 225, 255));
			}

			////////////////////////////////////////////////////////////////////////
			//���̕`��
			////////////////////////////////////////////////////////////////////////

			if (HitFloorRay(ray, plane , point)) {
				//�s���͗l
				if (sin(point.x / 50) * cos(point.z / 50) >= 0) {

					for (int i = 0; i < sp.size(); ++i) {
						//�e�`��
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
						//�e�`��
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
			//�{�[���̕`��
			////////////////////////////////////////////////////////////////////////

			for (int i = 0; i < sp.size(); ++i) {

				if (HitRay(eye, i, ray, bright,normal,hitballpos, specular)) {

					////////////////////////////////////////////////////////////////////////
					//���ʔ���
					////////////////////////////////////////////////////////////////////////

					auto ref = ReflectVector(ray, normal);
					auto b = 0.0f;

					auto ballray = sp[(i + 1) % sp.size()].pos - sp[i].pos;

					//������
					if (HitFloorRay(ref, plane, point)) {

						//���C��`��
						if (sin(point.x / 50) * cos(point.z / 50) >= 0) {
							//���˂��Ă���F����Z
							auto c = Vector3(0.5, 0.5, 0.5);
							albedo = sp[i].albedo * c;
						}
						else {
							//���˂��Ă���F����Z
							auto c = Vector3(0.1, 0.1, 0.5);
							albedo = sp[i].albedo * c;
						}
					}
					else {
						albedo = sp[i].albedo;
					}

					//���̑��I�u�W�F�N�g����
					if (HitObjeectRay(point, (i + 1) % sp.size(), ref, b)) {

						if (Dot(ref, ballray) >= 0) {
							auto c = sp[(i + 1) % sp.size()].albedo;
							albedo = sp[i].albedo * c;
							bright = b;
						}
					}

					//���C��`��
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

	//������
	Init();

	//�`��
	TraceOn();

	WaitKey();

	DxLib_End();
}
