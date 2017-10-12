#include"distance.h"
#include"scene.h"
#include"sampling.h"

namespace valley
{

// DistantLight Method Definitions
DistantLight::DistantLight(const Transform &LightToWorld, const Spectrum& L,
	const Vector3f &wLight)
	: Light((int)LightType::DeltaDirection, LightToWorld /*MediumInterface()*/),
	L(L),
	wLight(Normalize(LightToWorld(wLight))) {}

Spectrum DistantLight::power() const
{
	return L * Pi * worldRadius * worldRadius;	//����������ʵ�ʣ��������㷨Ҫ��
}

void DistantLight::preprocess(const Scene& scene)
{
	scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
}

Float DistantLight::pdf_Li(const Interaction&, const Vector3f&) const
{
	return 0.f;
}

void DistantLight::pdf_Le(const Ray&, const Normal3f&, Float* pdfPos,
	Float* pdfDir) const
{
	*pdfPos = 1 / (Pi * worldRadius * worldRadius);
	*pdfDir = 0;	// �������������ĸ���Ϊ 0
}

Spectrum DistantLight::sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi,
	Float* pdf, Visibility* vis) const
{
	*wi = wLight;
	*pdf = 1;
	Point3f pOutside = ref.p + wLight * (2 * worldRadius);	//��������������뾶
	*vis =
		Visibility(ref, Interaction(pOutside));
	return L;
}

Spectrum DistantLight::sample_Le(const Point2f& u1, const Point2f& u2, Ray* ray,
	Normal3f* nLight, Float* pdfPos, Float* pdfDir) const
{
	// Choose point on disk oriented toward infinite light direction
	Vector3f v1, v2;
	CoordinateSystem(wLight, &v1, &v2);
	Point2f cd = concentric_sample_disk(u1);
	//����һ���������Χ��һ�����Բ��
	Point3f pDisk = worldCenter + worldRadius * (cd.x * v1 + cd.y * v2);
	//Point3f pDisk = worldCenter + worldRadius * (cd.z * v1 + cd.x * v2);

	// Set ray origin and direction for infinite light ray
	*ray = Ray(pDisk + worldRadius * wLight, -wLight, Infinity);
	*nLight = (Normal3f)ray->d;
	*pdfPos = 1 / (Pi * worldRadius * worldRadius);
	*pdfDir = 1;
	return L;
}

}	//namespace valley
