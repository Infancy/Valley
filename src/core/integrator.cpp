#include"integrator.h"
#include"sampler.h"

namespace rainy
{

// Integrator Utility Functions
//�����й�Դ����uniform�Ĳ���
Spectrum uniform_sample_all_lights(const Interaction& it, const Scene &scene, Sampler &sampler,
	const std::vector<int> &nLightSamples, bool handleMedia)
{
	Spectrum L(0.f);

	// �������� light ���� it �� it.wo ����Ĺ���
	for (size_t i = 0; i < scene.lights.size(); ++i) // for(const auto& light : scene.lights)
	{
		const auto& light = scene.lights[i];
		int nSamples = nLightSamples[i];

		Spectrum Ld(0.f);
		for (int k = 0; k < nSamples; ++k)
			Ld += estimate_direct(it, sampler.get_2D(), *light, sampler.get_2D(), 
				scene, sampler, handleMedia);
		L += Ld / nSamples;	// �ڹ�Դ��ȡ n �������㣬����ƽ�������
	}
	return L;
}

Spectrum uniform_sample_one_light(const Interaction& it, const Scene& scene, Sampler& sampler,
	bool handleMedia, const Distribution1D* lightDistrib)
{
	int nLights = int(scene.lights.size());
	if (nLights == 0) return Spectrum(0.f);
	int lightNum;
	Float lightPdf;

	if (lightDistrib)
	{
		lightNum = lightDistrib->sample_discrete(sampler.get_1D(), &lightPdf);
		if (lightPdf == 0) return Spectrum(0.f);
	}
	else
	{
		lightNum = std::min((int)(sampler.get_1D() * nLights), nLights - 1);
		lightPdf = Float(1) / nLights;
	}
	const auto& light = scene.lights[lightNum];

	// ����������Դ���������书�ʵĸ����ܶȣ�power_pdf�����õ����Ʋ������й�Դ�Ľ��
	return estimate_direct(it, sampler.get_2D(), *light, sampler.get_2D(),
		scene, sampler, handleMedia) / lightPdf;
}

//	���㵥�� light ���� isect �� wo ������ķ����
//
//	prev_isect  light
//		  ----  ----
//		   ^      ^
//	        \    /
//	      wo \  / wi
//			  \/
//			------
//			isect
//
//	�� bsdf �ʾ���״̬����Դ�ֲ��Ϲ�ʱ�� bsdf ������Ϊ��Ч
//	�� bsdf ��������ֲ�״̬����Դ��Сʱ�ӹ�Դ��������Ч
//	���ʹ�ö�����Ҫ�Բ�����MIS���ֱ�� light �� bsdf ���в���
//	�ڹ�Դ�ϲ���һ�� p ���� Le���� bsdf �ϲ���һ���� wi ���� Li����� Ld = MIS(Le, Li)
Spectrum estimate_direct(const Interaction& it, const Point2f &uScattering, const Light &light,
	const Point2f &uLight, const Scene &scene, Sampler &sampler,
	bool handleMedia, bool has_specular)
{
	BxDFType bsdfFlags = has_specular ? BxDFType::All : BxDFType::NonSpecular;
	Spectrum Ld(0.f);

	// �ӹ�Դ���ֽ��ж�����Ҫ�Բ���
	Vector3f wi;
	Float lightPdf = 0, scatteringPdf = 0;
	Visibility visibility;
	// ���뽻�㣬�ڹ�Դ��ѡȡһ�㣬����ѡ���õ�ĸ����ܶȣ��õ㵽����ķ��� wi���������� Li ���ɼ���
	Spectrum Li = light.sample_Li(it, uLight, &wi, &lightPdf, &visibility);
	VLOG(2) << "EstimateDirect uLight:" << uLight << " -> Li: " << Li << ", wi: "
		<< wi << ", pdf: " << lightPdf;

	if (lightPdf > 0 && !Li.is_black()) 
	{
		// ���� BSDF����������� surface �У� �� ��λ��������������� medium �У���ֵ
		Spectrum f;
		if (it.on_surface()) 
		{
			// Evaluate BSDF for light sampling strategy
			const SurfaceInteraction& isect = (const SurfaceInteraction&)it;
			// ���� f(p, wo, wi) * cos(eta_light_isect) ��
			//f = isect.bsdf->f(isect.wo, wi, bsdfFlags) * AbsDot(wi, isect.shading.n);
			f = isect.bsdf->f(isect.wo, wi, bsdfFlags);
			f *= AbsDot(wi, isect.shading.n);
			scatteringPdf = isect.bsdf->pdf(isect.wo, wi, bsdfFlags);

			VLOG(2) << "  surf f*dot :" << f << ", scatteringPdf: " << scatteringPdf;
		}
		/*
		else 
		{
			// Evaluate phase function for light sampling strategy
			const MediumInteraction &mi = (const MediumInteraction &)it;
			Float p = mi.phase->p(mi.wo, wi);
			f = Spectrum(p);
			scatteringPdf = p;
			VLOG(2) << "  medium p: " << p;
		}
		*/
		if (!f.is_black()) 
		{
			// ���ǿɼ�������
			if (handleMedia) 
			{
				Li *= visibility.Tr(scene, sampler);
				VLOG(2) << "  after Tr, Li: " << Li;
			}
			else 
			{
				if (!visibility.unoccluded(scene))
				{
					VLOG(2) << "  shadow ray blocked";
					Li = Spectrum(0.f);
				}
				else
					VLOG(2) << "  shadow ray unoccluded";
			}

			// Add light's contribution to reflected radiance
			if (!Li.is_black()) 
			{
				if (is_DeltaLight(light.flags))	// �����delta��Դ������ʹ��MIS
					Ld += f * Li / lightPdf;		// ����ͨ��Դ��pdf=1���������Դ��pdf=1/area
				else 
				{
					Float weight =
						power_heuristic(1, lightPdf, 1, scatteringPdf);	// ʹ�ù�������ʽ��������Ȩ��
					Ld += f * Li * weight / lightPdf;
				}
			}
		}
	}

	//��bsdf���ֽ��в���
	if (!is_DeltaLight(light.flags)) 
	{
		Spectrum f;
		bool sampledSpecular = false;
		if (it.on_surface()) 
		{
			// Sample scattered direction for surface interactions
			BxDFType sampledType;
			const SurfaceInteraction& isect = (const SurfaceInteraction&)it;
			// �ڽ�����������ѡȡһ�㣬������Ӧ�� wi��scatteringPdf �� sampledType
			// ������ f(p, wo, wi) ��
			f = isect.bsdf->sample_f(isect.wo, &wi, uScattering, &scatteringPdf,
				bsdfFlags, &sampledType);
			// ���� cos(eta_light_isect) ��
			f *= AbsDot(wi, isect.shading.n);
			sampledSpecular = static_cast<int>(sampledType & BxDFType::Specular) != 0;	// has_specular(sampledType);
		}
		/*
		else 
		{
			// Sample scattered direction for medium interactions
			const MediumInteraction &mi = (const MediumInteraction &)it;
			Float p = mi.phase->Sample_p(mi.wo, &wi, uScattering);
			f = Spectrum(p);
			scatteringPdf = p;
		}
		*/
		VLOG(2) << "  BSDF / phase sampling f: " << f << ", scatteringPdf: " <<
			scatteringPdf;

		if (!f.is_black() && scatteringPdf > 0) 
		{
			// Account for light contributions along sampled direction _wi_
			// ����wi�ķ�������Դ����

			Float weight = 1;
			if (!sampledSpecular) 
			{
				lightPdf = light.pdf_Li(it, wi);	// it->wi->light�������� wi ��������� light �ĸ���
				if (lightPdf == 0) return Ld;		// ������ܲ����� light�� ��� bsdf ����ʧ�ܣ�����Ld
				weight = power_heuristic(1, scatteringPdf, 1, lightPdf);
			}

			// ������ wi �����Ƿ��루�������Դ�ཻ
			SurfaceInteraction lightInteraction;
			Ray ray = it.generate_ray(wi);
			Spectrum Tr(1.f);
			bool foundSurfaceInteraction =	// �Ƿ����͸�����Դ�������޹�
				handleMedia ? scene.intersectTr(ray, sampler, &lightInteraction, &Tr)
				: scene.intersect(ray, &lightInteraction);

			// ��� wi �������Դ�ཻ������Ҫ��һ����������ֱ�Ӽ��� light �� ray ������ķ����
			Spectrum Li(0.f);
			if (foundSurfaceInteraction) 
			{
				if (lightInteraction.primitive->get_AreaLight() == &light)
					Li = lightInteraction.Le(-wi);	
			}
			else
				Li = light.Le(ray);
			if (!Li.is_black()) Ld += f * Li * Tr * weight / scatteringPdf;
		}
	}
	return Ld;
}

static void check_radiance(int x, int y, Spectrum& L)
{
	// Issue warning if unexpected radiance value returned
	if (L.isnan())
	{
		LOG(ERROR) <<
			"Not-a-number radiance value returned "
			"for pixel( " << x << " , " << y << " ). Setting to black.";
		L = Spectrum(0.f);
	}
	else if (L.luminance() < -1e-5)
	{
		LOG(ERROR) <<
			"Negative luminance value returned "
			"for pixel( " << x << " , " << y << " ). Setting to black.";
		L = Spectrum(0.f);
	}
	else if (std::isinf(L.luminance()))
	{
		LOG(ERROR) <<
			"Infinite luminance value returned "
			"for pixel( " << x << " , " << y << " ). Setting to black.";
		L = Spectrum(0.f);
	}
}

// SamplerIntegrator Method Definitions
void SamplerIntegrator::render(const Scene &scene)
{
	for (int y = 0; y < camera->film->height; ++y)
		for (int x = 0; x < camera->film->width; ++x)
		{
			for(int count = 0; count < sampler->samples_PerPixel; ++count)
			{ 
				Ray ray;
				CameraSample cs = sampler->get_CameraSample(x, y);
				camera->generate_ray(cs, &ray);

				Spectrum L = Li(ray, scene, *sampler);

				check_radiance(x, y, L);
				camera->film->add(cs.pFilm, L);
			}
		}
	camera->film->flush();
	//��һ�����ز���n�Σ��ɷ����Ϊ��ʱ���ع�
	//return camera->film->scale(1.f / sampler->samples_PerPixel);	//filter��flush
}

Spectrum SamplerIntegrator::specular_reflect(const Ray& ray, const SurfaceInteraction& isect,
	const Scene &scene, Sampler &sampler, int depth) const
{
	// Compute specular reflection direction _wi_ and BSDF value
	Vector3f wo = isect.wo, wi;
	Float pdf;
	BxDFType type = BxDFType::Reflection | BxDFType::Specular;
	Spectrum f = isect.bsdf->sample_f(wo, &wi, sampler.get_2D(), &pdf, type);

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
		return Spectrum(0.f);
}

Spectrum SamplerIntegrator::specular_transmit(const Ray& ray, const SurfaceInteraction& isect,
	const Scene &scene, Sampler &sampler, int depth) const
{
	Vector3f wo = isect.wo, wi;
	Float pdf;
	const Point3f &p = isect.p;
	const Normal3f &ns = isect.shading.n;
	const BSDF &bsdf = *isect.bsdf;
	BxDFType type = BxDFType::Transmission | BxDFType::Specular;

	Spectrum f = bsdf.sample_f(wo, &wi, sampler.get_2D(), &pdf, type);
	Spectrum L = Spectrum(0.f);

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

}	//namespace rainy
