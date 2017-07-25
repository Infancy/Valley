#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_INTEGRATOR_DIRECTLIGHT_H
#define VALLEY_INTEGRATOR_DIRECTLIGHT_H

#include"valley.h"
#include"integrator.h"

namespace valley
{

class DirectLight : public SamplerIntegrator
{
public:
	DirectLight(std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler, uint32_t maxDepth = 2,
			bool sampler_all_light = true) :
		SamplerIntegrator(camera, sampler, maxDepth), sampler_all_light(sampler_all_light) {}

	virtual void preprocess(const Scene& scene, Sampler& sampler) 
	{
		if (sampler_all_light) 
		{
			// Compute number of samples to use for each light
			for (const auto &light : scene.lights)
				nLightSamples.push_back(light->nSamples);							
		}
	}

	virtual Spectrum Li(const Ray& ray, const Scene &scene,
		Sampler &sampler, int depth = 0) const
	{
		Spectrum L(0.f);
		// Find closest ray intersection or return background radiance
		SurfaceInteraction isect;

		//���δ���ڵ���������Դ���ӵ��ֱ�ӷ����
		if (!scene.intersect(ray, &isect)) 
		{
			for (const auto& light : scene.lights) L += light->Le(ray);
			return L;
		}

		//�����������������ӵ㷽����ķ����
		// Compute scattering functions for surface Interaction
		isect.compute_scattering(ray);
		if (!isect.bsdf)
			return Li(isect.generate_ray(ray.d), scene, sampler, depth);
		Vector3f wo = isect.wo;

		// Compute emitted light if ray hit an area light source
		//�����ϵ������Դ
		L += isect.Le(wo);
		//scene�еĹ�Դ
		if (scene.lights.size() > 0) 
		{
			// Compute direct lighting for _DirectLightingIntegrator_ integrator
			if (sampler_all_light)
				L += uniform_sample_all_lights(isect, scene, sampler, nLightSamples);
			else
				L += uniform_sample_one_light(isect, scene, sampler);
		}
		//��ֱ�ӹ����м��㾵�淴�������
		if (depth + 1 < maxDepth)
		{
			Vector3f wi;
			// Trace rays for specular reflection and refraction
			L += specular_reflect(ray, isect, scene, sampler, depth);
			L += specular_transmit(ray, isect, scene, sampler, depth);
		}
		return L;
	}

private:
	bool sampler_all_light;
	std::vector<int> nLightSamples;	//��¼��ÿ��light�Ĳ�������
};

}	//namespace valley


#endif //VALLEY_INTEGRATOR_DIRECTLIGHT_H