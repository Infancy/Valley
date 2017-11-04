#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_LIGHT_DISTANCE_H
#define RAINY_LIGHT_DISTANCE_H

#include"rainy.h"
#include"light.h"

namespace rainy
{

// �����Դ
class DistantLight : public Light 
{
public:
	DistantLight(const Transform &LightToWorld, const Spectrum &L,
				 const Vector3f &w);

	Spectrum power() const;

	//�����Դ��һЩ������Ҫ֪��������Χ�еĴ�С
	void preprocess(const Scene &scene);

	Float pdf_Li(const Interaction&, const Vector3f&) const;
	void pdf_Le(const Ray&, const Normal3f&, Float* pdfPos,
				Float* pdfDir) const;

	Spectrum sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi,
					Float* pdf, Visibility* vis) const override;
	Spectrum sample_Le(const Point2f& u1, const Point2f& u2, Ray* ray, 
					Normal3f* nLight, Float* pdfPos, Float* pdfDir) const override;

private:
	const Spectrum L;
	const Vector3f wLight;
	Point3f worldCenter;
	Float worldRadius;
};

}	//namespace rainy


#endif //RAINY_LIGHT_DISTANCE_H
