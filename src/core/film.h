#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_FILM_H
#define VALLEY_CORE_FILM_H

#include"valley.h"
#include"spectrum.h"
#include"geometry.h"
#include"image.h"
#include"filter.h"

namespace valley
{

struct Pixel
{
	Pixel() { FilterWeightSum = 0.0; }
	Spectrum color;
	Float FilterWeightSum;
	Spectrum splat;
	Float pad;	//ռλ
};

/*
(0,0)_ _ _ _ _ _x ����
	|
	|
	|	film ������ϵ
	|	
	|           .(x,y)
  y ����
*/

class Film	
{
public:
	Film(int width = 800, int height = 600, Filter* filter = nullptr, 
		 const std::string& filename = std::string("C:\\Users\\wyh32\\Desktop\\valley\\"),
		 bool save_type = false) 
		: width(width), height(height), 
		bounds(Point2i(0, 0), Point2i(width, height)),
		pixels(new Pixel[width * height]),
		filter(filter), FilterRadius(filter->radius), 
		invFilterRadius(1. / FilterRadius.x, 1. / FilterRadius.y),
		filename(filename), 
		save_type(save_type) 
	{
		// Precompute filter weight table
		int offset = 0;
		for (int y = 0; y < kernelWidth; ++y)
			for (int x = 0; x < kernelWidth; ++x, ++offset)
			{
				Point2f p;
				p.x = (x + 0.5f) * filter->radius.x / kernelWidth;
				p.y = (y + 0.5f) * filter->radius.y / kernelWidth;
				kernel[offset] = filter->evaluate(p);
			}
	}

	~Film() {}

	Pixel& operator()(int y, int x)
	{
		DCHECK(0 <= x && x < width && 0 <= y && y < height);
		return pixels[y * width + x];
	}
	Pixel operator()(int y, int x) const
	{
		DCHECK(0 <= x && x < width && 0 <= y && y < height);
		return pixels[y * width + x];
	}

	void scale(Float factor)
	{
		for (int i = 0; i < width * height; ++i)
			pixels[i].color *= factor;
	}

	Bounds2i get_sample_bounds() const 
	{
		Bounds2f floatBounds(
			Floor(Point2f(bounds.pMin) + Vector2f(0.5f, 0.5f) - filter->radius),
			 Ceil(Point2f(bounds.pMax) - Vector2f(0.5f, 0.5f) + filter->radius));
		return (Bounds2i)floatBounds;
	}

	void add(const Point2f pFilm, Spectrum L, Float sampleWeight = 1.0)
	{
		// Compute sample's raster bounds
		Point2f pFilmDiscrete = pFilm - Vector2f(0.5f, 0.5f);
		Point2i p0 = (Point2i)Ceil(pFilmDiscrete - filter->radius);
		Point2i p1 = (Point2i)Floor(pFilmDiscrete + filter->radius) + Point2i(1, 1);
		//����ɢ�� pFilmDiscrete Ϊ���ģ������˲��ķ�Χ
		p0 = Max(p0, bounds.pMin);
		p1 = Min(p1, bounds.pMax);

		//if (!(InsideExclusive(p0, bounds) && InsideExclusive(p1, bounds))) 
		if(!(p0.x < p1.x && p0.y < p1.y))
			return;

		// Loop over filter support and add sample to pixel arrays
		VLOG(2) << "add_Li p0: " << p0.x << ' ' << p0.y << " p1: " << p1.x << ' ' << p1.y
			<< " pFilm: " << pFilm.x << ' ' << pFilm.y;

		// Precompute $x$ and $y$ filter table offsets
		//Ԥ�ȼ������е�ƫ����
		std::vector<int> ifx(p1.x - p0.x);
		for (int x = p0.x; x < p1.x; ++x)
		{
			Float fx = std::abs((x - pFilmDiscrete.x) * invFilterRadius.x *
				kernelWidth);
			ifx[x - p0.x] = std::min((int)std::floor(fx), kernelWidth - 1);
		}

		std::vector<int> ify(p1.y - p0.y);
		for (int y = p0.y; y < p1.y; ++y)
		{
			Float fy = std::abs((y - pFilmDiscrete.y) * invFilterRadius.y *
				kernelWidth);
			ify[y - p0.y] = std::min((int)std::floor(fy), kernelWidth - 1);
		}

		for (int y = p0.y; y < p1.y; ++y)
			for (int x = p0.x; x < p1.x; ++x) 
			{
				// Evaluate filter value at $(x,y)$ pixel
				int offset = ify[y - p0.y] * kernelWidth + ifx[x - p0.x];

				Float filterWeight = kernel[offset];

				// Update pixel values with filtered sample contribution
				//���������˲����̷ֱ������Ӻͷ�ĸ������󱣴�ͼ���ʱ����г�������
				Pixel& pixel = (*this)(y, x);
				pixel.color += L * sampleWeight * filterWeight;    //������ += ��
				pixel.FilterWeightSum += filterWeight;

				//VLOG(1) << " x: " << x << " y: " << y << " L: " << L << " color: " << pixel.c 
				//	<< " weight: " << pixel.FilterWeightSum;
			}
	}

	void add_splat(const Point2f &p, Spectrum L)
	{
		if (L.isnan() || L.luminance() < 0 || std::isinf(L.luminance())) 
		{
			LOG(ERROR) << "Ignoring splatted spectrum values at " << p.x << " " << p.y;
			return;
		}

		if (!InsideExclusive((Point2i)p, bounds)) 
			return;

		Pixel& pixel = (*this)(p.y, p.x);
		pixel.splat = L;
	}

	void flush()
	{
		if (!filename.empty())
		{
			std::string time;
			long t = clock();
			while (t != 0)
			{
				char c = '0';
				time += (c + t % 10);
				t /= 10;
			}
			std::reverse(time.begin(), time.end());
			filename += time;
			filename += ".ppm";
		}

		//ִ�������˲������һ����������
		std::unique_ptr<Spectrum[]> colors(new Spectrum[width * height]);

		for(int i = 0; i < width * height; ++i)
		{ 
			Pixel& p = pixels[i];
			Spectrum& c = colors[i];

			if (p.FilterWeightSum != 0)
				c = p.color / p.FilterWeightSum;
			c += p.splat;
			//VLOG(1) << "i: " << i << " color: " << p.c << " weight: " << p.FilterWeightSum
			//	<< " final: " << c;
		}

		save_ppm(filename, colors.get(), width, height);
	}

public:
	int width, height;
	Bounds2i bounds;
	//Float resolution; �ֱ���Ϊ��λ������ϵ���������

private:
	std::unique_ptr<Pixel[]> pixels;

	std::unique_ptr<Filter> filter;
	Vector2f FilterRadius, invFilterRadius;
	static const int kernelWidth = 16;
	Float kernel[kernelWidth * kernelWidth];		//�����

	std::string filename;
	bool save_type;
};


}	//namespace valley


#endif //VALLEY_CORE_FILM_H
