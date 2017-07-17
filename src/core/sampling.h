#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SAMPLING_H
#define VALLEY_CORE_SAMPLING_H

#include"valley.h"
#include"geometry.h"

namespace valley
{

Vector3f uniform_sample_sphere(const Point2f &u);
Vector3f uniform_sample_hemisphere(const Point2f& u);

inline Float uniform_sphere_pdf()     { return Inv4Pi; }
inline Float uniform_hemisphere_pdf() { return Inv2Pi; }
inline Float uniform_cone_pdf(Float thetaMax) { return 1 / (2 * Pi * (1 - thetaMax)); }

//��[0,1]^2�ϵĲ�����ӳ�䵽disk��
Point2f concentric_sample_disk(const Point2f &u);

inline Vector3f cosine_sample_hemisphere(const Point2f &u)
{
	Point2f d = concentric_sample_disk(u);
	Float z = std::sqrt(std::max((Float)0, 1 - d.x * d.x - d.y * d.y));
	return Vector3f(d.x, d.y, z);
}
inline Float    cosine_hemisphere_pdf(Float cosTheta) { return cosTheta * InvPi; }

inline Float balance_heuristic(int nf, Float fPdf, int ng, Float gPdf) 
{
	return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}
//Veach���ݾ���ָ����beta=2����Ϊһ���Ϻõ�ֵ����Ӳ���뵽ʵ����
inline Float power_heuristic(int nf, Float fPdf, int ng, Float gPdf) 
{
	Float f = nf * fPdf, g = ng * gPdf;
	return (f * f) / (f * f + g * g);
}

class Distribution1D
{
public:
	//����light_power[n]
	Distribution1D(const Float* f, int n) : func(f, f + n), cdf(n + 1)
	{
		// Compute integral of step function at $x_i$
		cdf[0] = 0;
		for (int i = 1; i < n + 1; ++i)
			cdf[i] = cdf[i - 1] + func[i - 1] / n;

		// Transform step function integral into CDF
		funcInt = cdf[n];
		if (funcInt == 0)
			for (int i = 1; i < n + 1; ++i)
				cdf[i] = Float(i) / Float(n);
		else
			for (int i = 1; i < n + 1; ++i)
				cdf[i] /= funcInt;
		/*
		cdf[0] = 0;
		for (int i = 1; i < n + 1; ++i)
		cdf[i] = cdf[i - 1] + func[i - 1];

		// Transform step function integral into CDF
		total_power = cdf[n];
		if (funcInt == 0)
		for (int i = 1; i < n + 1; ++i)
		cdf[i] = Float(i) / Float(n);
		else
		for (int i = 1; i < n + 1; ++i)
		cdf[i] /= total_power;
		//cdf[i] from 0 to 1
		*/
	}

	int count() const { return func.size(); }

	//��任�㷨
	//�������Ȳ��������ֵu��������Ӧ�ֲ��µ�pdf�������$x_i$
	Float sample_continuous(Float u, Float *pdf, int *off = nullptr) const
	{
		// Find surrounding CDF segments and _offset_
		//0.f < u < 1.f,����u�����ĸ�cdf[i]
		int offset = FindInterval(cdf.size(),
			[&](int index) { return cdf[index] <= u; });
		if (off) *off = offset;

		// Compute offset along CDF segment
		//����u��cdf[offset]~cdf[offset+1]�ľ���du
		Float du = u - cdf[offset];
		if ((cdf[offset + 1] - cdf[offset]) > 0)
		{
			CHECK_GT(cdf[offset + 1], cdf[offset]);
			du /= (cdf[offset + 1] - cdf[offset]);
		}
		DCHECK(!std::isnan(du));

		// Compute PDF for sampled offset
		if (pdf) *pdf = (funcInt > 0) ? func[offset] / funcInt : 0;

		// Return $x\in{}[0,1)$ corresponding to sample
		//����ֵ$x_i$����[0,1]
		return (offset + du) / count();
	}

	//���ص�i��light����light_power / total_power
	int sample_discrete(Float u, Float* pdf = nullptr, Float* uRemapped = nullptr) const
	{
		// Find surrounding CDF segments and _offset_
		int offset = FindInterval(cdf.size(),
			[&](int index) { return cdf[index] <= u; });

		if (pdf) *pdf = (funcInt > 0) ? func[offset] / (funcInt * count()) : 0;

		if (uRemapped)
			*uRemapped = (u - cdf[offset]) / (cdf[offset + 1] - cdf[offset]);
		if (uRemapped)
			CHECK(*uRemapped >= 0.f && *uRemapped <= 1.f);
		return offset;
	}

	Float discrete_pdf(int index) const
	{
		CHECK(index >= 0 && index < count());
		return func[index] / (funcInt * count());
	}

public:
	std::vector<Float> func, cdf;
	Float funcInt;	//��������ֵ
};

}	//namespace valley


#endif //VALLEY_CORE_SAMPLING_H
