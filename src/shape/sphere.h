#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SPHERE_H
#define VALLEY_CORE_SPHERE_H

#include"valley.h"
#include"geometry.h"
#include"intersection.h"

namespace valley
{

class Sphere
{
protected:
	Point3f center;
	Float radius;

	const Float kEpsilon = 0.001f;   //for shadows and secondary rays

public:
	Sphere(Point3f c, Float r) : center(c), radius(r) {}
	
	bool intersect(const Ray& ray, Isect& isect) const
	{
		Float t;
		Vector3f v = ray.o - center;
		//���t^2 + 2bt + c = 0;		��Ϊdirection�ǵ�λ����������a = d * d = 1
		Float b = ray.d * v;
		Float c = v * v - radius * radius;
		Float discr = b * b - c;		

		if (discr > 0.0)
		{
			Float e = sqrt(discr);

			t = -b - e;
			if (t > kEpsilon)
			{
				//tmin = t;										//����ԭ���������ڲ�ʱӦ��ת����
				isect.normal = (v + t * ray.d) / radius;			//���صĲ����ǵ�λ����
				isect.p = ray.o + t * ray.d;
				return true;
			}

			t = -b + e;
			if (t > kEpsilon)
			{
				//tmin = t;
				isect.normal = (v + t * ray.d) / radius;
				isect.p = ray.o + t * ray.d;
				return true;
			}
			return false;	//��������·�������ص���˼
		}
		else
			return false;
	}
};

}	//namespace valley


#endif //VALLEY_CORE_SPHERE_H
