#include"material/matte.h"
//#include"bsdf.h"
#include"bsdf/lambertian.h"

namespace valley
{

Matte::Matte(const std::shared_ptr<Texture<Spectrum>>& kd,
	const std::shared_ptr<Texture<Float>>& sigma,   //�ֲڶ�
	const std::shared_ptr<Texture<Float>>& bumpMap)
	: kd(kd), sigma(sigma), bumpMap(bumpMap) {}

void Matte::compute_scattering(SurfaceInteraction* si, TransportMode mode,
							   bool allowMultipleLobes) const 
{
	//���㰼͹����
	//if (bumpMap) bump(bumpMap, is);

	// Evaluate textures for _MatteMaterial_ material and allocate BRDF
	si->bsdf.reset(new BSDF(*si));
	Spectrum r = kd->evaluate(*si).clamp();
	Float sig = Clamp(sigma->evaluate(*si), 0, 90);
	if (!r.is_black())
	{
		//	if (sig == 0)   //����ֲڶ�Ϊ0��Ϊ��ȫ������
		si->bsdf->add_BxDF(new LambertianReflection(r));
		//	else
		//	si->bsdf->add_BxDF(ARENA_ALLOC(arena, OrenNayar)(r, sig));
	}
}

}	//namespace valley