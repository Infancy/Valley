#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_INTERSECTION_H
#define VALLEY_CORE_INTERSECTION_H

#include"valley.h"
#include"geometry.h"
#include"material.h"
#include"bsdf.h"

namespace valley
{

//Intersection 
class Isect
{
public:
	Isect(Float maxDist = Infinity) : dist(maxDist) {}
	Isect(const Point3f &p, const Normal3f &n, const Vector3f &pError,
		const Vector3f &wo, Float maxDist = Infinity);

	Ray generate_ray(const Vector3f& d) const
	{
		Point3f o = offset_ray_origin(p, pError, n, d);
		return Ray(o, d, Infinity);
	}

	//����ռ�һ�㣬��������õ��ray
	Ray generate_ray(const Point3f &p2) const 
	{
		Point3f origin = offset_ray_origin(p, pError, n, p2 - p);
		Vector3f d = p2 - p;
		return Ray(origin, d, 1 - ShadowEpsilon);
	}

	Ray generate_ray(const Isect& isect) const 
	{
		Point3f origin = offset_ray_origin(p, pError, n, isect.p - p);
		Point3f target = offset_ray_origin(isect.p, isect.pError, isect.n, origin - isect.p);
		Vector3f d = target - origin;
		return Ray(origin, d, 1 - ShadowEpsilon);
	}

private:
	//�Թ���Դ�����һ������ƫ�ƣ��Ե�����ֵ���ȴ��������
	Point3f offset_ray_origin(const Point3f& p, const Vector3f& pError,
							  const Normal3f& normal, const Vector3f& direct) const
	{
		Float d = Dot(Abs(normal), pError);
	#ifdef VALLEY_FLOAT_AS_DOUBLE
		// We have tons of precision; for now bump up the offset a bunch just
		// to be extra sure that we start on the right side of the surface
		// (In case of any bugs in the epsilons code...)
		d *= 1024.;
	#endif
		Vector3f offset = d * Vector3f(normal);
		if (Dot(direct, normal) < 0) offset = -offset;	//���������ɹ��ߵķ���
		Point3f po = p + offset;
		// Round offset point _po_ away from _p_
		for (int i = 0; i < 3; ++i) {
			if (offset[i] > 0)
				po[i] = NextFloatUp(po[i]);
			else if (offset[i] < 0)
				po[i] = NextFloatDown(po[i]);
		}
		return po;
	}

public:
	Float dist;			//���������ľ���
	Point3f p;			//����
	Normal3f n;    //���㴦�ķ���
	Vector3f wo;		//������ߵķ���
	Vector3f pError;    //�ۻ��ĸ������������  
};

class SurfaceIsect : public Isect 
{
public:
	SurfaceIsect() {}
	SurfaceIsect(const Point3f& p, const Vector3f& pError,
				 const Point2f& uv, const Vector3f& wo,
				 const Vector3f& dpdu, const Vector3f& dpdv,
				 const Normal3f& dndu, const Normal3f& dndv, 
			     const Shape* sh);
	//������ɫ������
	void set_shading_geometry(const Vector3f& dpdu, const Vector3f& dpdv,
							  const Normal3f& dndu, const Normal3f& dndv, 
							  bool orientationIsAuthoritative);

	void compute_scattering(const RayDifferential& ray, 
							TransportMode mode = TransportMode::Radiance, 
							bool allowMultipleLobes = true);

	//void compute_differentials(const RayDifferential &r) const;

	Color Le(const Vector3f& w) const;

public:
	Point2f uv;				//���ڱ����������UV����
	Vector3f dpdu, dpdv;	//����ƫ΢�֣�λ����ƽ���ϣ������ɷ���
	Normal3f dndu, dndv;	//���淨�߱仯�ĵ�ƫ΢��

	const Shape*	 shape     = nullptr;  
	const Primitive* primitive = nullptr;
	
	std::unique_ptr<BSDF>  bsdf	  = nullptr;
	//std::unique_ptr<BSSRDF> bssrdf = nullptr;

	//�洢�ɰ�͹����������������𶥵㷨�߲�ֵ�õ�����ɫ���ߵ�ֵ
	struct
	{
		Normal3f n;			//shadingNormal
		Vector3f dpdu, dpdv;
		Normal3f dndu, dndv;
	} shading;
};

}	//namespace valley


#endif //VALLEY_CORE_INTERSECTION_H
