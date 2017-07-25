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

class Interaction
{
public:
	Interaction() {}
	Interaction(const Point3f &p  /*const MediumInterface &mediumInterface*/)
		: p(p) /*mediumInterface(mediumInterface)*/ {}
	Interaction(const Point3f &p, const Vector3f &wo 
		/*const MediumInterface &mediumInterface*/)
		: p(p), wo(wo) /*mediumInterface(mediumInterface)*/ {}
	Interaction(const Point3f &p, const Normal3f &n, const Vector3f &pError,
		const Vector3f &wo, /*const MediumInterface &mediumInterface, */ Float maxDist = Infinity);

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
		//return Ray(origin, d, 1 - ShadowEpsilon);
		return Ray(origin, d, (p - p2).Length() * (1 - ShadowEpsilon));
	}

	Ray generate_ray(const Interaction& isect) const 
	{
		Point3f origin = offset_ray_origin(p, pError, n, isect.p - p);
		Point3f target = offset_ray_origin(isect.p, isect.pError, isect.n, origin - isect.p);
		Vector3f d = target - origin;
		//return Ray(origin, d, 1 - ShadowEpsilon);
		return Ray(origin, d, (p - isect.p).Length() * (1 - ShadowEpsilon));
	}

	bool in_surface() const { return n != Normal3f(0, 0, 0); }
	bool in_medium() const { return !in_surface(); }

public:
	Point3f p;			//����
	Normal3f n;    //���㴦�ķ���
	Vector3f wo;		//������ߵķ���
	Vector3f pError;    //�ۻ��ĸ������������ 
	//MediumInterface mediumInterface;
};

class SurfaceInteraction : public Interaction 
{
public:
	SurfaceInteraction() {}
	SurfaceInteraction(const Point3f& p, const Vector3f& pError,
				 const Point2f& uv, const Vector3f& wo,
				 const Vector3f& dpdu, const Vector3f& dpdv,
				 const Normal3f& dndu, const Normal3f& dndv, 
			     const Shape* sh);
	//������ɫ������
	void set_shading_geometry(const Vector3f& dpdu, const Vector3f& dpdv,
							  const Normal3f& dndu, const Normal3f& dndv, 
							  bool orientationIsAuthoritative);

	void compute_scattering(const RayDifferential& ray, 
							//MemoryArena &arena,
							TransportMode mode = TransportMode::Radiance, 
							bool allowMultipleLobes = true);

	//void compute_differentials(const RayDifferential &r) const;

	Spectrum Le(const Vector3f& w) const;

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

/*
class MediumInteraction : public Interaction
{
public:
	// MediumInteraction Public Methods
	MediumInteraction() : phase(nullptr) {}
	MediumInteraction(const Point3f &p, const Vector3f &wo, Float time,
		const Medium *medium, const PhaseFunction *phase)
		: Interaction(p, wo), phase(phase) {}

	bool is_valid() const { return phase != nullptr; }

	// MediumInteraction Public Data
	const PhaseFunction *phase;
};
*/

//��¼������Դ�ϵĽ���
class EndpointInteraction : public Interaction 
{
public:
	EndpointInteraction() : Interaction(), light(nullptr) {}
	EndpointInteraction(const Interaction &it, const Camera *camera)
		: Interaction(it), camera(camera) {}
	EndpointInteraction(const Camera *camera, const Ray &ray)
		: Interaction(ray.o), camera(camera) {}
	EndpointInteraction(const Light *light, const Ray &r, const Normal3f &nl)
		: Interaction(r.o), light(light) { n = nl; }
	EndpointInteraction(const Interaction &it, const Light *light)
		: Interaction(it), light(light) {}
	EndpointInteraction(const Ray &ray)
		: Interaction(ray(1)), light(nullptr) { n = Normal3f(-ray.d); }

public:
	union
	{
		const Camera *camera;
		const Light *light;
	};
};

}	//namespace valley


#endif //VALLEY_CORE_INTERSECTION_H
