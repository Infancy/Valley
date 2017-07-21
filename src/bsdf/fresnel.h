#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_BSDF_FOURIER_H
#define VALLEY_BSDF_FOURIER_H

#include"valley.h"
#include"bsdf.h"
#include"color.h"

namespace valley
{

Float FrDielectric(Float cosThetaI, Float etaI, Float etaT);
Color FrConductor(Float cosThetaI, const Color &etaI,
				  const Color &etaT, const Color &k);

class Fresnel 
{
public:
	virtual ~Fresnel() {}
	virtual Color evaluate(Float cosI) const = 0;
	//virtual std::string ToString() const = 0;
};

class FresnelConductor : public Fresnel 
{
public:
	FresnelConductor(const Color &etaI, const Color &etaT, const Color &k)
		: etaI(etaI), etaT(etaT), k(k) {}

	Color evaluate(Float cosThetaI) const override
	{
		return FrConductor(std::abs(cosThetaI), etaI, etaT, k);
	}
	//std::string ToString() const;

private:
	Color etaI, etaT, k;	//etaI��etaTΪ��������Ľ��������ʣ�kΪ����ϵ��
};

class FresnelDielectric : public Fresnel 
{
public:
	FresnelDielectric(Float etaI, Float etaT) : etaI(etaI), etaT(etaT) {}

	Color evaluate(Float cosThetaI) const override
	{
		return FrDielectric(cosThetaI, etaI, etaT);
	}
	//std::string ToString() const;

private:
	Float etaI, etaT;
};

//���� mirror �У���ȫ�������й���
class FresnelNoOp : public Fresnel
{
public:
	Color evaluate(Float) const override { return Color(1.); }
	//std::string ToString() const { return "[ FresnelNoOp ]"; }
};

}	//namespace valley


#endif //VALLEY_BSDF_FOURIER_H
