#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SPHERE_H
#define VALLEY_CORE_SPHERE_H

#include"shape.h"

namespace valley
{

class Sphere : public Shape
{
public:
	Sphere(Transform* o2w, bool reverseOrientation, Float radius) : 
		Shape(o2w, new Transform(o2w->GetInverseMatrix(), o2w->GetMatrix()), 
		reverseOrientation), radius(radius) {}
	/*
		thetaMin(std::acos(Clamp(std::min(zMin, zMax) / radius, -1, 1))),
        thetaMax(std::acos(Clamp(std::max(zMin, zMax) / radius, -1, 1))),
		phiMax(Radians(Clamp(phiMax, 0, 360))
	*/

	Bounds3f object_bound() const override;

	bool intersect(const Ray &ray, SurfaceInteraction* isect
		/*bool testAlphaTexture*/) const override;
	bool intersectP(const Ray &ray, bool testAlphaTexture) const override;
	Float area() const override;

	Interaction sample(const Point2f &u, Float *pdf) const override;
	Interaction sample(const Interaction& ref, const Point2f &u,
				 Float *pdf) const override;

	Float pdf(const Interaction& ref, const Vector3f &wi) const override;
	Float solid_angle(const Point3f &p, int nSamples) const override;

private:
	//Point3f center; ͨ��������ת������ľֲ������У�ʹcenter�ܵ���(0,0,0)
	Float radius;
};

}	//namespace valley


#endif //VALLEY_CORE_SPHERE_H
