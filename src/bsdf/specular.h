#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_BSDF_SPECULAR_H
#define VALLEY_BSDF_SPECULAR_H

#include"valley.h"
#include"bsdf.h"
#include"fresnel.h"

namespace valley
{

inline Vector3f Reflect(const Vector3f &wo, const Vector3f &n) {
	return -wo + 2 * Dot(wo, n) * n;
}

inline bool Refract(const Vector3f &wi, const Normal3f &n, Float eta, Vector3f *wt) 
{
	// Compute $\cos \theta_\roman{t}$ using Snell's law
	Float cosThetaI = Dot(n, wi);
	Float sin2ThetaI = std::max(Float(0), Float(1 - cosThetaI * cosThetaI));
	Float sin2ThetaT = eta * eta * sin2ThetaI;

	// Handle total internal reflection for transmission
	//wi�ӽ���ˮƽ���䣬����������
	if (sin2ThetaT >= 1) return false;
	Float cosThetaT = std::sqrt(1 - sin2ThetaT);
	*wt = eta * -wi + (eta * cosThetaI - cosThetaT) * Vector3f(n);
	return true;
}

class SpecularReflection : public BxDF 
{
public:
	SpecularReflection(const Color &R, Fresnel *fresnel)
		: BxDF(BxDF_type::Reflection | BxDF_type::Specular),
		R(R), fresnel(fresnel) {}

	Color f(const Vector3f &wo, const Vector3f &wi) const { return Color(0.f); }

	Color sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
				   Float *pdf, BxDF_type *sampledType) const
	{
		// Compute perfect specular reflection direction
		//*wi = Vector3f(-wo.x, -wo.y, wo.z);
		*wi = Vector3f(-wo.x, wo.y, -wo.z);
		*pdf = 1;

		return fresnel->evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
		//�� mirror �� return 1.f * R / AbsCosTheta(*wi);
	}

	Float pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }

	//std::string ToString() const;

private:
	const Color R;
	const Fresnel* fresnel;
};

class SpecularTransmission : public BxDF 
{
public:
	SpecularTransmission(const Color &T, Float etaA, Float etaB,
		TransportMode mode)
		: BxDF(BxDF_type(BxDF_type::Transmission| BxDF_type::Specular)),
		T(T), etaA(etaA), etaB(etaB),
		fresnel(etaA, etaB), mode(mode) {}

	Color f(const Vector3f &wo, const Vector3f &wi) const { return Color(0.f); }

	Color sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
		Float *pdf, BxDF_type *sampledType) const
	{
		// Figure out which $\eta$ is incident and which is transmitted
		bool entering = CosTheta(wo) > 0;
		Float etaI = entering ? etaA : etaB;
		Float etaT = entering ? etaB : etaA;

		// Compute ray direction for specular transmission
		if (!Refract(wo, Faceforward(Normal3f(0, 1, 0), wo), etaI / etaT, wi))
			return 0;
		*pdf = 1;
		Color ft = T * (Color(1.) - fresnel.evaluate(CosTheta(*wi)));
		// Account for non-symmetry with transmission to different medium
		if (mode == TransportMode::Radiance) ft *= (etaI * etaI) / (etaT * etaT);
		return ft / AbsCosTheta(*wi);
	}

	Float pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }

	//std::string ToString() const;

private:
	const Color T;
	const Float etaA, etaB;
	const FresnelDielectric fresnel;
	const TransportMode mode;
};

class FresnelSpecular : public BxDF
{
public:
	FresnelSpecular(const Color &R, const Color &T, Float etaA,
		Float etaB, TransportMode mode)
		: BxDF(BxDF_type(BxDF_type::Reflection | BxDF_type::Transmission | BxDF_type::Specular)),
		R(R), T(T), etaA(etaA), etaB(etaB), mode(mode) {}

	Color f(const Vector3f &wo, const Vector3f &wi) const { return Color(0.f); }

	Color sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
		Float *pdf, BxDF_type *sampledType) const
	{
		//���ݷ�������ȷ���Ǽ��㷴�仹������
		Float F = FrDielectric(CosTheta(wo), etaA, etaB);
		if (u[0] < F) 
		{
			// Compute specular reflection for _FresnelSpecular_

			// Compute perfect specular reflection direction
			*wi = Vector3f(-wo.x, wo.y, -wo.z);
			if (sampledType)
				*sampledType = BxDF_type::Specular | BxDF_type::Reflection;
			*pdf = F;
			return F * R / AbsCosTheta(*wi);
		}
		else 
		{
			// Compute specular transmission for _FresnelSpecular_

			// Figure out which $\eta$ is incident and which is transmitted
			bool entering = CosTheta(wo) > 0;
			Float etaI = entering ? etaA : etaB;
			Float etaT = entering ? etaB : etaA;

			// Compute ray direction for specular transmission
			if (!Refract(wo, Faceforward(Normal3f(0, 1, 0), wo), etaI / etaT, wi))
				return 0;
			Color ft = T * (1 - F);

			// Account for non-symmetry with transmission to different medium
			if (mode == TransportMode::Radiance)
				ft *= (etaI * etaI) / (etaT * etaT);
			if (sampledType)
				*sampledType = BxDF_type::Specular | BxDF_type::Transmission;
			*pdf = 1 - F;
			return ft / AbsCosTheta(*wi);
		}
	}

	Float pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }

	//std::string ToString() const;

private:
	const Color R, T;
	const Float etaA, etaB;
	const TransportMode mode;
};

}	//namespace valley


#endif //VALLEY_BSDF_SPECULAR_H
