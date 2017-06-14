#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_CAMERA_H
#define VALLEY_CORE_CAMERA_H

#include"valley.h"
#include"geometry.h"
#include"transform.h"
#include"film.h"

namespace valley
{

struct CameraSample	//���ɹ�������Ĳ���ֵ
{
	Point2f pFilm;
	Point2f pLens;
};


class Camera
{
public:
	Camera(Film* film = nullptr) : film(film) {}
	virtual ~Camera() {}

	//���ݲ���������������ռ���ߣ�������һ������������ͼ���ϵ�Ȩֵ
	virtual Float generate_ray(const CameraSample& sample, Ray* ray) const = 0;

public:
	Film* film;
};


//class ProjectiveCamera : public Camera


class Pinhole : public Camera	//PerspectiveCamera
{
public:
	Pinhole(const Point3f eye, const Point3f target, const Vector3f up,
		   Float imageDistance, Film* film)
		: Camera(film), eye(eye), imageDistance(imageDistance)
	{
		//����camera_to_world����
		forward = Normalize(target - eye);
		right   = Normalize(Cross(up, forward));	//���ֲ��
		upward  = Cross(forward, right);

		//�����ֳ�
		//Float pixelSize = 1.f / std::sqrt(film->resolution);
		Float a = std::sqrt(film->resolution);
		//Float pixelSize = 1.f / a;
		pixelSize = 1.f / a;
	}

	Float generate_ray(const CameraSample& sample, Ray* ray) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);	
		Point3f pCamera(pixelSize * (pFilm.x - film->width / 2),
						pixelSize * (film->height / 2 - pFilm.y),
						imageDistance);
		//Vec3 dir = p.x * u + p.y * v + vpd * w;
		Vector3f dir = forward * pCamera.z + right * pCamera.x + upward * pCamera.y;
		*ray = Ray(eye, Normalize(dir));

		//*ray = camera_to_world(*ray);
		return 1;
	}

private:
	Point3f eye;
	Vector3f forward, right, upward;

	Float imageDistance, pixelSize;	//ͼ��ƽ�浽��׵ľ��롢����ʵ�ʳߴ�
};

}	//namespace valley


#endif //VALLEY_CORE_CAMERA_H
