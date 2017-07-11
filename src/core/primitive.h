#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_PRIMITIVE_H
#define VALLEY_CORE_PRIMITIVE_H

#include"valley.h"
#include"geometry.h"
#include"material.h"

namespace valley
{

class Primitive 
{
public:
	virtual ~Primitive() {}

	virtual Bounds3f world_bound() const = 0;
	virtual bool intersect(const Ray &r, SurfaceIsect*) const = 0;
	virtual bool intersectP(const Ray &r) const = 0;
	virtual const AreaLight* get_AreaLight() const = 0;
	virtual const Material* get_material() const = 0;
	virtual void compute_scattering(SurfaceIsect* isect, 
								//	MemoryArena &arena,
									TransportMode mode, 
									bool allowMultipleLobes) const = 0;
};

class GeometricPrimitive : public Primitive 
{
public:
	GeometricPrimitive(const std::shared_ptr<Shape>& shape,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<AreaLight>& areaLight = nullptr
		//,const MediumInterface &mediumInterface
		) : 
		shape(shape),
		material(material),
		areaLight(areaLight) 
	  //mediumInterface(mediumInterface)
	{}

	virtual Bounds3f world_bound() const override;

	virtual bool intersect(const Ray& r, SurfaceIsect* isect) const override;
	virtual bool intersectP(const Ray& r) const override;

	const AreaLight* get_AreaLight() const override;
	const Material* get_material() const override;

	void compute_scattering(SurfaceIsect* isect,
						//	MemoryArena &arena,
							TransportMode mode,
							bool allowMultipleLobes) const override;

private:
	std::shared_ptr<Shape> shape;
	std::shared_ptr<Material> material;
	std::shared_ptr<AreaLight> areaLight;
	//MediumInterface mediumInterface;
};

//���������ͬʵ��
/*
class TransformedPrimitive : public Primitive
{
public:
	// TransformedPrimitive Public Methods
	TransformedPrimitive(std::shared_ptr<Primitive> &primitive,
		const AnimatedTransform &PrimitiveToWorld)
		: primitive(primitive), PrimitiveToWorld(PrimitiveToWorld) {}
	bool Intersect(const Ray &r, SurfaceIsect *in) const;
	bool IntersectP(const Ray &r) const;
	const AreaLight *GetAreaLight() const { return nullptr; }
	const Material *GetMaterial() const { return nullptr; }
	void compute_scattering(SurfaceIsect *isect,
		//MemoryArena &arena, 
		TransportMode mode,
		bool allowMultipleLobes) const {
		LOG(FATAL) <<
			"TransformedPrimitive::ComputeScatteringFunctions() shouldn't be "
			"called";
	}
	Bounds3f world_bound() const {
		return PrimitiveToWorld.MotionBounds(primitive->world_bound());
	}

private:
	// TransformedPrimitive Private Data
	std::shared_ptr<Primitive> primitive;
	const AnimatedTransform PrimitiveToWorld;
};
*/

class Accelerator : public Primitive
{
public:
	Accelerator(std::vector<std::shared_ptr<Primitive>>& primitives) :
		primitives(primitives)
	{
		//����������İ�Χ��
		for (auto& p : primitives)
			bounds = Union(bounds, p->world_bound());
	}
	virtual Bounds3f world_bound() const override { return bounds; }

	const AreaLight* get_AreaLight() const override;
	const Material* get_material() const override;

	void compute_scattering(SurfaceIsect* isect, 
						//	MemoryArena &arena,
							TransportMode mode,
							bool allowMultipleLobes) const override;

	virtual bool intersect(const Ray& r, SurfaceIsect* isect) const override;
	virtual bool intersectP(const Ray& r) const override;

private:
	std::vector<std::shared_ptr<Primitive>> primitives;
	Bounds3f bounds;
};

}	//namespace valley


#endif //VALLEY_CORE_PRIMITIVE_H