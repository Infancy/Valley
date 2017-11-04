#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_RECTANGLE_H
#define RAINY_CORE_RECTANGLE_H

#include"shape.h"

namespace rainy
{

class Rectangle : public Shape
{
public:
	Rectangle(Transform* o2w,
		bool reverseOrientation, Float Xaxis, Float Yaxis) : 
		Shape(o2w, new Transform(o2w->GetInverseMatrix(), o2w->GetMatrix()), 
		reverseOrientation), point(-Xaxis / 2.f, -Yaxis / 2.f, 0), 
		first(Xaxis, 0, 0), second(0, Yaxis, 0),
		normal(Normalize(Cross(first, second))) {}	//��������������ϵ��

	Bounds3f object_bound() const;

	bool intersect(const Ray &ray, SurfaceInteraction* isect
		/*bool testAlphaTexture*/) const;
	bool intersectP(const Ray &ray, bool testAlphaTexture) const;

	Float area() const;

	Interaction sample(const Point2f &u, Float *pdf) const;

private:
	Point3f 		point; //����ͨ���任����������λ�ã���point���Ƕ����
	Vector3f		first, second;
	Normal3f        normal;
};

}	//namespace rainy


#endif //RAINY_CORE_RECTANGLE_H
