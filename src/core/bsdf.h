/*
pbrt source code is Copyright(c) 1998-2016
Matt Pharr, Greg Humphreys, and Wenzel Jakob.

This file is part of pbrt.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_BSDF_H
#define RAINY_CORE_BSDF_H

#include "rainy.h"
#include "geometry.h"
#include "spectrum.h"
//#include "interaction.h"

namespace rainy
{

//
// Utility Function
//

inline Float CosTheta(const Vector3f& w) { return w.z; }
inline Float AbsCosTheta(const Vector3f& w) { return std::abs(w.z); }
inline Float Cos2Theta(const Vector3f& w) { return w.z * w.z; }

inline bool same_hemisphere(const Vector3f& w, const Vector3f& wp) { return w.z * wp.z > 0; }
inline bool same_hemisphere(const Vector3f& w, const Normal3f& wp) { return w.z * wp.z > 0; }

//
// BxDFType
//

enum class BxDFType 
{
	// 每个 type 至少有 Re 或 Tr 之一
	Reflection   = 1 << 0,
	Transmission = 1 << 1,	// 透射，和镜面没什么必然的联系

	Diffuse		 = 1 << 2,	// 漫反/透射
	Glossy		 = 1 << 3,	// 光泽
	Specular	 = 1 << 4,	// 镜面，delta 分布

	NonSpecular  = Reflection | Transmission | Diffuse | Glossy,
	All			 = Reflection | Transmission | Diffuse | Glossy | Specular
};

//
// BxDFType Inline Method Definitions
//

inline BxDFType operator&(BxDFType a, BxDFType b) 
{ 
	return static_cast<BxDFType>(static_cast<int>(a) & static_cast<int>(b)); 
}
inline BxDFType operator|(BxDFType a, BxDFType b) 
{ 
	return static_cast<BxDFType>(static_cast<int>(a) | static_cast<int>(b)); 
}

inline bool has_specular(BxDFType type)		{ return static_cast<bool>(type & BxDFType::Specular); }
inline bool has_reflection(BxDFType type)   { return static_cast<bool>(type & BxDFType::Reflection); }
inline bool has_transmission(BxDFType type) { return static_cast<bool>(type & BxDFType::Transmission); }
// inline bool non_specular(BxDFType type)  { return !has_specular(type); }

//
// BxDF
// 

class BxDF 
{
public:
	BxDF(BxDFType t) : type(t) {}
	virtual ~BxDF() {}
	
	bool match(BxDFType t) const;

	virtual Spectrum f(const Vector3f& wo, const Vector3f& wi) const;

	virtual Spectrum sample_f(const Vector3f& wo, Vector3f* wi, 
							  const Point2f& sample, Float* Pdf, 
							  BxDFType* sampledType = nullptr) const;

	// 针对某些无法通过闭式计算反射率的 BxDF，可用 rho 来估算（使用蒙特卡洛方法）
	// rho_hemisphere_direction
	virtual Spectrum rho(const Vector3f& wo, int nSamples,
						 const Point2f* samples) const;
	// rho_hemisphere_hemisphere
	virtual Spectrum rho(int nSamples, const Point2f* samples1,
					     const Point2f* samples2) const;

	virtual Float pdf(const Vector3f& wo, const Vector3f& wi) const;
	// virtual std::string ToString() const = 0;

	// 某些光线传输算法需要对 BRDF 和 BTDF 进行区分，所以加入 type 成员
	const BxDFType type;
};

//
// BSDF
//

class BSDF 
{
public:
	BSDF(const SurfaceInteraction& si, Float eta = 1);
	~BSDF() {}				// 使用系统的 new 和 delete

	void add_BxDF(BxDF* b);
	int components_num(BxDFType flags = BxDFType::All) const;	// nComponents

	Vector3f world_to_local(const Vector3f& v) const;
	Vector3f local_to_world(const Vector3f& v) const;

	// 针对给定方向返回分布函数值
	Spectrum f(const Vector3f& woW, const Vector3f& wiW,
			  BxDFType flags = BxDFType::All) const;

	// 根据采样值从bxdf[n]中选取一个bxdf（sampleType即为该bxdf），计算sample_f，得到wi，pdf
	// 然后需对pdf进行均值计算以得到平均值，最后计算f(wo,wi)
	Spectrum sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, Float* pdf,
					 BxDFType type = BxDFType::All, BxDFType* sampledType = nullptr) const;

	Spectrum rho(int nSamples, const Point2f* samples1, const Point2f* samples2,
				BxDFType flags = BxDFType::All) const;
	Spectrum rho(const Vector3f& wo, int nSamples, const Point2f* samples,
				BxDFType flags = BxDFType::All) const;

	Float pdf(const Vector3f& wo, const Vector3f& wi,
			  BxDFType flags = BxDFType::All) const;
	//std::string ToString() const;

public:
	const Float eta;	// 表面相对折射率，用于（半）透明物体

private:
	const Normal3f ns, ng;	// shading-normal/geometry-normal
	const Vector3f ss, ts;	// ns,ss,ts构成了着色坐标系

	static constexpr int MaxBxDFs = 8;	// 最大挂载 8 个 BxDF 组件
	int nBxDFs = 0;
	std::unique_ptr<BxDF> bxdfs[MaxBxDFs];
	//friend class MixMaterial;
};

//
// BSDF Inline Method Definitions
//

inline void BSDF::add_BxDF(BxDF* b)
{
	CHECK_LT(nBxDFs, MaxBxDFs);
	bxdfs[nBxDFs++].reset(b);
}

inline Vector3f BSDF::world_to_local(const Vector3f &v) const
{
	return Vector3f(Dot(v, ss), Dot(v, ts), Dot(v, ns));	// s->x,t->y,n->z
}
inline Vector3f BSDF::local_to_world(const Vector3f &v) const
{	
	// 因为 stn 是正交矩阵，其逆矩阵即为转置矩阵
	return Vector3f(ss.x * v.x + ts.x * v.y + ns.x * v.z,
					ss.y * v.x + ts.y * v.y + ns.y * v.z,
					ss.z * v.x + ts.z * v.y + ns.z * v.z);
}

}	// namespace rainy


#endif //RAINY_CORE_BSDF_H
