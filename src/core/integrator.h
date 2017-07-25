#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_INTEGTATOR_H
#define VALLEY_CORE_INTEGTATOR_H

#include"valley.h"
#include"spectrum.h"
#include"geometry.h"
#include"interaction.h"
#include"lightdistrib.h"
#include"camera.h"
#include"sampler.h"
#include"scene.h"

namespace valley
{

//���Ȳ��������Դ��radiance
Spectrum uniform_sample_all_lights(const Interaction& it, const Scene &scene, Sampler& sampler,
								const std::vector<int>& nLightSamples, bool handleMedia = false);

//����������Դ��������power_pdf���õ����Ʋ��������Դ�Ľ��
Spectrum uniform_sample_one_light(const Interaction& it, const Scene& scene, Sampler& sampler,
							   bool handleMedia = false, const Distribution1D* lightDistrib = nullptr);

//ѡ��һ��Interaction��һ��Light��ʹ��˫����Ҫ�Բ���������ֱ�ӹ��յĹ���
Spectrum estimate_direct(const Interaction& it,    const Point2f& uScattering,
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
	virtual Spectrum Li(const Ray& ray, const Scene &scene,
		Sampler &sampler, int depth = 0) const
	{
		std::cerr << "error,should't call Li() in Integrator_base_class\n";
		return Spectrum();
	}

	virtual void preprocess(const Scene& scene, Sampler& sampler) {}
	virtual void render(const Scene &scene) = 0;

public:
	int maxDepth;

	std::shared_ptr<Sampler> sampler;
	std::shared_ptr<Camera> camera;
};

class SamplerIntegrator : public Integrator
{
public:
	SamplerIntegrator(std::shared_ptr<Camera> camera,  std::shared_ptr<Sampler> sampler,
			   int maxDepth) : Integrator(camera, sampler, maxDepth) {}
	virtual ~SamplerIntegrator() {}

	virtual void render(const Scene& scene) override;

	Spectrum specular_reflect(const Ray& ray, const SurfaceInteraction& isect,
						  const Scene &scene, Sampler &sampler, int depth) const;
	Spectrum specular_transmit(const Ray& ray, const SurfaceInteraction& isect,  
						   const Scene &scene, Sampler &sampler, int depth) const;
};

}	//namespace valley


#endif //VALLEY_CORE_INTEGTATOR_H
