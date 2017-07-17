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

#ifndef VALLEY_CORE_LIGHT_H
#define VALLEY_CORE_LIGHT_H

#include"valley.h"
#include"transform.h"
#include"intersection.h"
#include"color.h"

namespace valley
{

enum class Light_type
{
	DeltaPosition  = 1,
	DeltaDirection = 2,
	Area		   = 4,
	Infinite       = 8
};

inline bool is_delta_light(int flags) 
{
	return flags & static_cast<int>(Light_type::DeltaPosition) ||
		   flags & static_cast<int>(Light_type::DeltaDirection);
}

class Light
{
public:
	Light(
		int flags,				//�����ؿ����㷨����Ҫ���ֹ�Դ������
		const Transform& LightToWorld, 
	  //const MediumInterface &mediumInterface,
		int nSamples = 1);		//���������Դ��������Ӱ���ߣ���������Ӱ
	virtual ~Light();

	//�ܷ��书�ʣ�ֻ��Ҫ�������ֵ����
	virtual Color power() const = 0;

	//�ڿ�ʼ��Ⱦǰ��¼������һЩ��������DistanceLight��¼������Χ��
	virtual void preprocess(const Scene &scene) {}

	//��Դ��-r������ķ����
	//���統�ӵ㷢���Ĺ����볡��������δ�ཻʱ�������Դ���ӵ㷽��ķ����
	virtual Color Le(const RayDifferential &r) const;

	virtual Float pdf_Li(const Isect& ref, const Vector3f &wi) const = 0;
	virtual void pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
						Float *pdfDir) const = 0;

	//����Isect�����ص���õ��incident Radiance����ķ���wi
	//����Դ�������Դʱ�����贫��һ��[0,1]^2��Χ�Ĳ����㣬�Թ�Դ��һ����в�������¼�����ܶ�ֵpdf
	//Visibility���ڼ�¼��Ӱ���ߵ���Ϣ
	virtual Color sample_Li(const Isect& ref, const Point2f& u,
							Vector3f* wi, Float* pdf, Visibility* vis) const = 0;


	virtual Color sample_Le(const Point2f& u1, const Point2f& u2,  Ray* ray, 
							Normal3f* nLight, Float* pdfPos, Float* pdfDir) const = 0;

public:
	const int flags;
	const int nSamples;
	//const MediumInterface mi;

protected:
	const Transform LightToWorld, WorldToLight;
};


class Visibility
{
public:
	Visibility() {}
	// VisibilityTester Public Methods
	Visibility(const Isect& p0, const Isect& p1)
		: p0(p0), p1(p1) {}

	//ĳЩ���ߴ��䷽����Ҫ��������
	const Isect& P0() const { return p0; }
	const Isect& P1() const { return p1; }

	bool unoccluded(const Scene &scene) const;

	//transmittance
	//�����ڽ����еĹ�����Ŀ���֮��ķ����
	Color Tr(const Scene &scene, Sampler &sampler) const;

private:
	//���㡢��Դ�����ڹ�����Ӱ����
	Isect p0, p1;
};

class AreaLight : public Light 
{
public:
	// AreaLight Interface
	AreaLight(const Transform& LightToWorld, 
	  //const MediumInterface &medium,
		int nSamples);

	//���ݹ�Դ����һ������淨�߼�����䷽���ϵķ�������L
	virtual Color L(const Isect& intr, const Vector3f& w) const = 0;
};

}	//namespace valley


#endif //VALLEY_CORE_LIGHT_H
