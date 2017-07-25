#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_MATERIAL_MATTE_H
#define VALLEY_MATERIAL_MATTE_H

#include"valley.h"
#include"material.h"

namespace valley
{

//�ƹ����
class Matte : public Material
{
public:
	// MatteMaterial Public Methods
	Matte(const std::shared_ptr<Texture<Spectrum>>& kd,
		  const std::shared_ptr<Texture<Float>>& sigma,   //�ֲڶ�
		  const std::shared_ptr<Texture<Float>>& bumpMap = nullptr);

	void compute_scattering(SurfaceInteraction* si, TransportMode mode,
		bool allowMultipleLobes) const override;

private:
	std::shared_ptr<Texture<Spectrum>> kd;
	std::shared_ptr<Texture<Float>> sigma;	//�ֲڶ�
	std::shared_ptr<Texture<Float>> bumpMap;
};

}	//namespace valley


#endif //VALLEY_MATERIAL_MATTE_H
