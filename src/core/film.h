#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_FILM_H
#define VALLEY_CORE_FILM_H

#include"valley.h"
#include"color.h"
#include"geometry.h"
#include"image.h"

namespace valley
{

class Film
{
public:
	Film(int width = 800, int height = 600, Float resolution = 1.f) 
		: width(width), height(height), resolution(resolution), pixels(new Color4f[width * height]) {}
	~Film() { save_ppm("tt.ppm", pixels.get(), width, height); }

	Color4f& operator()(int x, int y)
	{
		DCHECK(0 <= x && x < width && 0 <= y && y < height);
		return pixels[y * width + x];
	}
	Color4f operator()(int x, int y) const
	{
		DCHECK(0 <= x && x < width && 0 <= y && y < height);
		return pixels[y * width + x];
	}

public:
	int width, height;
	Float resolution;	////�ֱ���Ϊ��λ������ϵ���������

private:
	std::unique_ptr<Color4f[]> pixels;
};


}	//namespace valley


#endif //VALLEY_CORE_FILM_H
