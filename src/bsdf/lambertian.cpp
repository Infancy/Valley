#include"lambertian.h"
#include"sampling.h"

namespace valley
{

Spectrum LambertianTransmission::sample_f(const Vector3f& wo, Vector3f* wi,
	const Point2f& u, Float* Pdf, BxDFType* sampledType) const 
{
	*wi = cosine_sample_hemisphere(u);
	if (wo.z > 0) wi->z *= -1;	// ����ɫ������°���ѡȡ����
	*Pdf = pdf(wo, *wi);
	return f(wo, *wi);
}

Float LambertianTransmission::pdf(const Vector3f& wo, const Vector3f& wi) const 
{
	return !same_hemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
}

}	//namespace valley