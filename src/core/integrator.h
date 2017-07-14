#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_INTEGTATOR_H
#define VALLEY_CORE_INTEGTATOR_H

#include"valley.h"
#include"color.h"
#include"geometry.h"
#include"intersection.h"
#include"lightdistrib.h"
#include"camera.h"
#include"sampler.h"
#include"scene.h"

namespace valley
{

//���Ȳ��������Դ��radiance
Color uniform_sample_all_lights(const Isect& it, const Scene &scene, Sampler& sampler,
								const std::vector<int>& nLightSamples, bool handleMedia = false);

//����������Դ��������power_pdf���õ����Ʋ��������Դ�Ľ��
Color uniform_sample_one_light(const Isect& it, const Scene& scene, Sampler& sampler,
							   bool handleMedia = false, const Distribution1D* lightDistrib = nullptr);

//ѡ��һ��Isect��һ��Light��ʹ��˫����Ҫ�Բ���������ֱ�ӹ��յĹ���
Color estimate_direct(const Isect& it,    const Point2f& uScattering,
					  const Light& light, const Point2f& uLight, 
					  const Scene& scene, Sampler& sampler,
					  bool handleMedia = false, bool has_specular = false);

class Integrator 
{
public:
	Integrator(std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler,
		uint32_t maxDepth) : camera(camera), sampler(sampler), maxDepth(maxDepth) {}
	virtual ~Integrator() {}

	//interactive(ray, scene){Li();}
	//�����ع��ߵķ����
	virtual Color Li(const Ray& ray, const Scene &scene,
		Sampler &sampler, int depth = 0) const
	{
		std::cerr << "error,should't call Li() in Integrator_base_class\n";
		return Color();
	}

	virtual void preprocess(const Scene& scene, Sampler& sampler) {}
	virtual void render(const Scene &scene)
	{
		std::cerr << "error,should't call render() in Integrator_base_class\n";
	}

public:
	uint32_t maxDepth;

	std::shared_ptr<Sampler> sampler;
	std::shared_ptr<Camera> camera;
};

class SamplerIntegrator : public Integrator
{
public:
	SamplerIntegrator(std::shared_ptr<Camera> camera,  std::shared_ptr<Sampler> sampler,
			   uint32_t maxDepth) : Integrator(camera, sampler, maxDepth) {}
	virtual ~SamplerIntegrator() {}

	virtual void render(const Scene& scene) override;

	Color specular_reflect(const Ray& ray, const SurfaceIsect& isect,
						  const Scene &scene, Sampler &sampler, int depth) const;
	Color specular_transmit(const Ray& ray, const SurfaceIsect& isect,  
						   const Scene &scene, Sampler &sampler, int depth) const;
};

}	//namespace valley


#endif //VALLEY_CORE_INTEGTATOR_H
