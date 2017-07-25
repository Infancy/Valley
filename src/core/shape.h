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

#ifndef VALLEY_CORE_SHAPE_H
#define VALLEY_CORE_SHAPE_H

#include"valley.h"
#include"geometry.h"
#include"interaction.h"
#include"transform.h"

namespace valley
{

class Shape 
{
public:
	Shape(const Transform* ObjectToWorld, const Transform* WorldToObject,
		bool reverseOrientation);
	virtual ~Shape();

	virtual Bounds3f object_bound() const = 0;
	virtual Bounds3f world_bound() const;

	virtual bool intersect(const Ray &ray, 
						   SurfaceInteraction* isect /*bool testAlphaTexture*/) const = 0;
	virtual bool intersectP(const Ray &ray,
						    bool testAlphaTexture = true) const;

	virtual Float area() const = 0;
	virtual Float pdf(const Interaction &) const { return 1 / area(); }

	//����һ��[0,1]��Χ�Ĳ����㣬�ڼ�����������һ��Interaction��������Ӧ��pdf(1/area())
	virtual Interaction sample(const Point2f &u, Float *pdf) const = 0;

	// Sample a point on the shape given a reference point |ref| and
	// return the PDF with respect to solid angle from |ref|.
	virtual Interaction sample(const Interaction &ref, const Point2f &u, Float *pdf) const;

	//����һ��isect��isect�ϵķ���wi����isect->wi->shape���������Ӧ��pdf
	virtual Float pdf(const Interaction &ref, const Vector3f &wi) const;

	// Returns the solid angle subtended by the shape w.r.t. the reference
	// point p, given in world space. Some shapes compute this value in
	// closed-form, while the default implementation uses Monte Carlo
	// integration; the nSamples parameter determines how many samples are
	// used in this case.
	virtual Float solid_angle(const Point3f &p, int nSamples = 512) const;

public:
	std::unique_ptr<const Transform> ObjectToWorld, WorldToObject;
	const bool reverseOrientation;			//�Ƿ�ת�����ߣ�����
	const bool transformSwapsHandedness;    //ת������ϵ������or���֣���
};

}	//namespace valley


#endif //VALLEY_CORE_SHAPE_H
