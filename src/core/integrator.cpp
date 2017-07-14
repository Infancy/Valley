#include"integrator.h"
#include"sampler.h"

namespace valley
{

// Integrator Utility Functions
Color uniform_sample_all_lights(const Isect& it, const Scene &scene, Sampler &sampler,
	const std::vector<int> &nLightSamples, bool handleMedia)
{
	Color L(0.f);
	for (size_t i = 0; i < scene.lights.size(); ++i) 
	{
		// Accumulate contribution of _j_th light to _L_
		const std::shared_ptr<Light>& light = scene.lights[i];
		int nSamples = nLightSamples[i];

		Color Ld(0.f);
		for (int k = 0; k < nSamples; ++k)
			Ld += estimate_direct(it, sampler.get_2D(), *light, sampler.get_2D(), 
				scene, sampler, handleMedia);
		L += Ld / nSamples;	//�ڹ�Դ��ȡn�������㣬����ƽ�������
	}
	return L;
}

Color uniform_sample_one_light(const Isect& it, const Scene& scene, Sampler& sampler,
	bool handleMedia, const Distribution1D* lightDistrib)
{
	// Randomly choose a single light to sample, _light_
	int nLights = int(scene.lights.size());
	if (nLights == 0) return Color(0.f);
	int lightNum;
	Float lightPdf;

	if (lightDistrib)
	{
		//���ѡȡһ��light
		lightNum = lightDistrib->sample_discrete(sampler.get(), &lightPdf);
		if (lightPdf == 0) return Color(0.f);
	}
	else
	{
		lightNum = std::min((int)(sampler.get() * nLights), nLights - 1);
		lightPdf = Float(1) / nLights;
	}
	const std::shared_ptr<Light> &light = scene.lights[lightNum];
	//Point2f uLight = sampler.get_2D();
	//Point2f uScattering = sampler.get_2D();

	//������lightPdf���õ�һ���Ŵ��Ч���������ڶ����еƹ���в���
	return estimate_direct(it, sampler.get_2D(), *light, sampler.get_2D(),
		scene, sampler, handleMedia) / lightPdf;
}

Color estimate_direct(const Isect& it, const Point2f &uScattering, const Light &light,
	const Point2f &uLight, const Scene &scene, Sampler &sampler,
	bool handleMedia, bool has_specular)
{
	// Sample light source with multiple importance sampling
	BxDF_type bsdfFlags = has_specular ? BxDF_type::All : BxDF_type::NonSpecular;
	Color Ld(0.f);
	Vector3f wi;
	Float lightPdf = 0, scatteringPdf = 0;
	Visibility visibility;

	Color Li = light.sample_Li(it, uLight, &wi, &lightPdf, &visibility);
	//VLOG(2) << "EstimateDirect uLight:" << uLight << " -> Li: " << Li << ", wi: "
	//	<< wi << ", pdf: " << lightPdf;

	if (lightPdf > 0 && !Li.is_black()) 
	{
		// Compute BSDF or phase function's value for light sample
		Color f;
		if (it.in_surface()) 
		{
			// Evaluate BSDF for light sampling strategy
			const SurfaceIsect& isect = (const SurfaceIsect&)it;
			//f = isect.bsdf->f(isect.wo, wi, bsdfFlags) * AbsDot(wi, isect.shading.n);
			f = isect.bsdf->f(isect.wo, wi, bsdfFlags);
			f *= AbsDot(wi, isect.shading.n);
			scatteringPdf = isect.bsdf->pdf(isect.wo, wi, bsdfFlags);
			//VLOG(2) << "  surf f*dot :" << f << ", scatteringPdf: " << scatteringPdf;
		}
		/*
		else 
		{
			// Evaluate phase function for light sampling strategy
			const MediumInteraction &mi = (const MediumInteraction &)it;
			Float p = mi.phase->p(mi.wo, wi);
			f = Color(p);
			scatteringPdf = p;
			VLOG(2) << "  medium p: " << p;
		}
		*/
		if (!f.is_black()) 
		{
			// Compute effect of visibility for light source sample
			if (handleMedia) 
			{
				Li *= visibility.Tr(scene, sampler);
				//VLOG(2) << "  after Tr, Li: " << Li;
			}
			else 
			{
				if (!visibility.unoccluded(scene))
				{
					//VLOG(2) << "  shadow ray blocked";
					Li = Color(0.f);
				}
				else
					VLOG(2) << "  shadow ray unoccluded";
			}

			// Add light's contribution to reflected radiance
			if (!Li.is_black()) 
			{
				if (is_delta_light(light.flags))
					Ld += f * Li / lightPdf;		//����ͨ��Դ��pdf=1���������Դ��pdf=1/area
				else 
				{
					Float weight =
						power_heuristic(1, lightPdf, 1, scatteringPdf);
					Ld += f * Li * weight / lightPdf;
				}
			}
		}
	}

	// Sample BSDF with multiple importance sampling
	if (!is_delta_light(light.flags)) 
	{
		Color f;
		bool sampledSpecular = false;
		if (it.in_surface()) 
		{
			// Sample scattered direction for surface interactions
			BxDF_type sampledType;
			const SurfaceIsect& isect = (const SurfaceIsect&)it;
			f = isect.bsdf->sample_f(isect.wo, &wi, uScattering, &scatteringPdf,
				bsdfFlags, &sampledType);
			f *= AbsDot(wi, isect.shading.n);
			sampledSpecular = static_cast<int>(sampledType & BxDF_type::Specular) != 0;
		}
		/*
		else 
		{
			// Sample scattered direction for medium interactions
			const MediumInteraction &mi = (const MediumInteraction &)it;
			Float p = mi.phase->Sample_p(mi.wo, &wi, uScattering);
			f = Color(p);
			scatteringPdf = p;
		}
		*/
		//VLOG(2) << "  BSDF / phase sampling f: " << f << ", scatteringPdf: " <<
		//	scatteringPdf;

		if (!f.is_black() && scatteringPdf > 0) 
		{
			// Account for light contributions along sampled direction _wi_
			Float weight = 1;
			if (!sampledSpecular) {
				lightPdf = light.pdf_Li(it, wi);
				if (lightPdf == 0) return Ld;
				weight = power_heuristic(1, scatteringPdf, 1, lightPdf);
			}

			// Find intersection and compute transmittance
			SurfaceIsect lightIsect;
			Ray ray = it.generate_ray(wi);
			Color Tr(1.f);
			bool foundSurfaceIsect =
				handleMedia ? scene.intersectTr(ray, sampler, &lightIsect, &Tr)
				: scene.intersect(ray, &lightIsect);

			// Add light contribution from material sampling
			Color Li(0.f);
			if (foundSurfaceIsect) 
			{
				if (lightIsect.primitive->get_AreaLight() == &light)
					Li = lightIsect.Le(-wi);
			}
			else
				Li = light.Le(ray);
			if (!Li.is_black()) Ld += f * Li * Tr * weight / scatteringPdf;
		}
	}
	return Ld;
}

void check_radiance(int x, int y, Color& L)
{
	// Issue warning if unexpected radiance value returned
	if (L.isnan())
	{
		LOG(ERROR) <<
			"Not-a-number radiance value returned "
			"for pixel( " << x << " , " << y << " ). Setting to black.";
		L = Color(0.f);
	}
	else if (L.luminance() < -1e-5)
	{
		LOG(ERROR) <<
			"Negative luminance value returned "
			"for pixel( " << x << " , " << y << " ). Setting to black.";
		L = Color(0.f);
	}
	else if (std::isinf(L.luminance()))
	{
		LOG(ERROR) <<
			"Infinite luminance value returned "
			"for pixel( " << x << " , " << y << " ). Setting to black.";
		L = Color(0.f);
	}
}

// SamplerIntegrator Method Definitions
void SamplerIntegrator::render(const Scene &scene)
{
	preprocess(scene, *sampler);

	for (int y = 0; y < camera->film->height; ++y)
		for (int x = 0; x < camera->film->width; ++x)
		{
			for(int count = 0; count < sampler->samples_PerPixel; ++count)
			{ 
				Ray ray;
				camera->generate_ray(sampler->get_CameraSample(x, y), &ray);

				Color L = Li(ray, scene, Sampler());

				check_radiance(x, y, L);
				camera->film->add(x, y, L);
			}
		}

	//��һ�����ز���n�Σ��ɷ����Ϊ��ʱ���ع�
	return camera->film->scale(1.f / sampler->samples_PerPixel);	//filter��flush
}

Color SamplerIntegrator::specular_reflect(const Ray& ray, const SurfaceIsect& isect,
	const Scene &scene, Sampler &sampler, int depth) const
{
	// Compute specular reflection direction _wi_ and BSDF value
	Vector3f wo = isect.wo, wi;
	Float pdf;
	BxDF_type type = BxDF_type::Reflection | BxDF_type::Specular;
	Color f = isect.bsdf->sample_f(wo, &wi, sampler.get_2D(), &pdf, type);

	// Return contribution of specular reflection
	const Normal3f &ns = isect.shading.n;
	if (pdf > 0.f && !f.is_black() && AbsDot(wi, ns) != 0.f) 
	{
		// Compute ray differential _rd_ for specular reflection
		Ray rd = isect.generate_ray(wi);
		/*
		if (ray.hasDifferentials) 
		{
			rd.hasDifferentials = true;
			rd.rxOrigin = isect.p + isect.dpdx;
			rd.ryOrigin = isect.p + isect.dpdy;
			// Compute differential reflected directions
			Normal3f dndx = isect.shading.dndu * isect.dudx +
				isect.shading.dndv * isect.dvdx;
			Normal3f dndy = isect.shading.dndu * isect.dudy +
				isect.shading.dndv * isect.dvdy;
			Vector3f dwodx = -ray.rxDirection - wo,
				dwody = -ray.ryDirection - wo;
			Float dDNdx = Dot(dwodx, ns) + Dot(wo, dndx);
			Float dDNdy = Dot(dwody, ns) + Dot(wo, dndy);
			rd.rxDirection =
				wi - dwodx + 2.f * Vector3f(Dot(wo, ns) * dndx + dDNdx * ns);
			rd.ryDirection =
				wi - dwody + 2.f * Vector3f(Dot(wo, ns) * dndy + dDNdy * ns);
		}
		*/
		return f * Li(rd, scene, sampler, depth + 1) * AbsDot(wi, ns) /
			pdf;
	}
	else
		return Color(0.f);
}

Color SamplerIntegrator::specular_transmit(const Ray& ray, const SurfaceIsect& isect,
	const Scene &scene, Sampler &sampler, int depth) const
{
	Vector3f wo = isect.wo, wi;
	Float pdf;
	const Point3f &p = isect.p;
	const Normal3f &ns = isect.shading.n;
	const BSDF &bsdf = *isect.bsdf;
	BxDF_type type = BxDF_type::Transmission | BxDF_type::Specular;

	Color f = bsdf.sample_f(wo, &wi, sampler.get_2D(), &pdf, type);
	Color L = Color(0.f);

	if (pdf > 0.f && !f.is_black() && AbsDot(wi, ns) != 0.f) 
	{
		// Compute ray differential _rd_ for specular transmission
		Ray rd = isect.generate_ray(wi);
		/*
		if (ray.hasDifferentials) 
		{
			rd.hasDifferentials = true;
			rd.rxOrigin = p + isect.dpdx;
			rd.ryOrigin = p + isect.dpdy;

			Float eta = bsdf.eta;
			Vector3f w = -wo;
			if (Dot(wo, ns) < 0) eta = 1.f / eta;

			Normal3f dndx = isect.shading.dndu * isect.dudx +
				isect.shading.dndv * isect.dvdx;
			Normal3f dndy = isect.shading.dndu * isect.dudy +
				isect.shading.dndv * isect.dvdy;

			Vector3f dwodx = -ray.rxDirection - wo,
				dwody = -ray.ryDirection - wo;
			Float dDNdx = Dot(dwodx, ns) + Dot(wo, dndx);
			Float dDNdy = Dot(dwody, ns) + Dot(wo, dndy);

			Float mu = eta * Dot(w, ns) - Dot(wi, ns);
			Float dmudx =
				(eta - (eta * eta * Dot(w, ns)) / Dot(wi, ns)) * dDNdx;
			Float dmudy =
				(eta - (eta * eta * Dot(w, ns)) / Dot(wi, ns)) * dDNdy;

			rd.rxDirection =
				wi + eta * dwodx - Vector3f(mu * dndx + dmudx * ns);
			rd.ryDirection =
				wi + eta * dwody - Vector3f(mu * dndy + dmudy * ns);
		}
		*/
		L = f * Li(rd, scene, sampler, depth + 1) * AbsDot(wi, ns) / pdf;
	}
	return L;
}

}	//namespace valley