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

	virtual Spectrum We(const Ray& ray, Point2f* pRaster2 = nullptr) const = 0;
	virtual void pdf_We(const Ray& ray, Float* pdfPos, Float* pdfDir) const = 0;
	virtual Spectrum sample_Wi(const Interaction& ref, const Point2f& u,
		Vector3f* wi, Float* pdf, Point2f* pRaster, Visibility* vis) const = 0;

public:
	std::shared_ptr<Film> film;
};

}	//namespace valley


#endif //VALLEY_CORE_CAMERA_H
