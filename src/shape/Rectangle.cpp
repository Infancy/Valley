#include"Rectangle.h"
#include"sampling.h"

namespace valley
{

// Rectangle Method Definitions
Bounds3f Rectangle::object_bound() const
{
	return Bounds3f(point, point + first + second);
}

bool Rectangle::intersect(const Ray &r, SurfaceIsect* isect
	/*bool testAlphaTexture*/) const
{
	// Transform _Ray_ to object space
	Ray ray = (*WorldToObject)(r);

	ray.d = Normalize(ray.d);	//�費��Ҫ��һ����

	Vector3f vec = point - ray.o;
	const Normal3f& n = normal;
	//tHit = (point - ray.o) * normal / (ray.d * normal); 
	Float tmpa = (vec.x * n.x + vec.y * n.y + vec.z * n.z);
	Float tmpb = (ray.d.x * n.x + ray.d.y * n.y + ray.d.z * n.z);
	if (tmpb == 0.f)
		return false;		//ƽ��
	Float t = tmpa / tmpb;

	if (t <= 0.f || t >= ray.tMax)
		return false;

	Point3f pHit = ray.o + t * ray.d;
	Vector3f d = pHit - point;

	Float dDotfirst = d * first;

	if (dDotfirst < 0.0 || dDotfirst > first * first)
		return false;

	Float dDotsecond = d * second;

	if (dDotsecond < 0.0 || dDotsecond > second * second)
		return false;

	//u��v from 0 to 1
	Float u = d * first / first.Length();
	Float v = d * second / second.Length();
	
	Vector3f dpdu(0, 0, first.Length()), dpdv(second.Length(), 0, 0);
	Normal3f dndu(0, 0, 0), dndv(0, 0, 0);

	Vector3f pError(0, 0, 0);

	//�½�һ��SI��ֵ��isect
	*isect = (*ObjectToWorld)(SurfaceIsect(pHit, pError, Point2f(u, v),
		-ray.d, dpdu, dpdv, dndu, dndv, this));

	r.tMax = t;	//��¼�ཻ����
	return true;
}

bool Rectangle::intersectP(const Ray &r, bool testAlphaTexture) const
{
	// Transform _Ray_ to object space
	Ray ray = (*WorldToObject)(r);

	ray.d = Normalize(ray.d);	//�費��Ҫ��һ����

	Vector3f vec = point - ray.o;
	const Normal3f& n = normal;
	//tHit = (point - ray.o) * normal / (ray.d * normal); 
	Float tmpa = (vec.x * n.x + vec.y * n.y + vec.z * n.z);
	Float tmpb = (ray.d.x * n.x + ray.d.y * n.y + ray.d.z * n.z);
	if (tmpb == 0.f)
		return false;		//ƽ��
	Float t = tmpa / tmpb;

	if (t <= 0.f || t >= ray.tMax)
		return false;

	Point3f pHit = ray.o + t * ray.d;
	Vector3f d = pHit - point;

	Float dDotfirst = d * first;

	if (dDotfirst < 0.0 || dDotfirst > first * first)
		return false;

	Float dDotsecond = d * second;

	if (dDotsecond < 0.0 || dDotsecond > second * second)
		return false;

	return true;
}

Float Rectangle::area() const { return first.Length() * second.Length(); }

//����һ��[0,1]^2��Χ�ڵ������,����һ���������Ӧ��pdf
Isect Rectangle::sample(const Point2f &u, Float *pdf) const
{
	//Point3f pObj(u.x * first.Length(), u.y * second.Length(), 0);
	Point3f pObj = Point3f(u.y * second.Length(), 0, u.x * first.Length()) + point;
	Isect it;
	//it.n = Normalize((*ObjectToWorld)(Normal3f(0, 0, 1)));
	it.n = Normalize((*ObjectToWorld)(Normal3f(0, 1, 0)));
	if (reverseOrientation) it.n *= -1;
	it.p = (*ObjectToWorld)(pObj, Vector3f(0, 0, 0), &it.pError);
	*pdf = 1 / area();
	return it;
}

}	//namespace valley