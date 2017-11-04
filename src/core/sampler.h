#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SAMPLER_H
#define VALLEY_CORE_SAMPLER_H

#include"valley.h"
#include"rng.h"
#include"camera.h"

namespace valley
{

//
// Sampler
//

class Sampler
{
public:
	Sampler(int64_t samples_PerPixel): 
		samples_PerPixel(samples_PerPixel) {}
	~Sampler(){}

	virtual Float   get_1D() = 0;
	virtual Point2f get_2D() = 0;

	CameraSample get_CameraSample(int x, int y);
	CameraSample get_CameraSample(Point2i p);

	// ���� start_pixel ����һ�����������е�����
	virtual void start_pixel(const Point2i& p);
	virtual bool next_sample();
	virtual bool set_SampleIndex(int64_t sampleNum);

	virtual std::unique_ptr<Sampler> clone(int seed) = 0;

public:
	const int64_t samples_PerPixel;

protected:
	Point2i currentPixel;	//��ǰ���ĸ�������
	int64_t currentPixel_SampleIndex;	//�ڸ����صĵڼ�����������

	/*
	//ÿ��������Ĵ�СΪ n * samples_PerPixel��subArraySizes ��¼������� n �Ĵ�С
	vector<vector<Float>>   sampleArray1D;
	vector<vector<Point2f>> sampleArray2D;
	vector<int>  subArraySizes_1D,       subArraySizes_2D;
	size_t current_ArrayOffset_1D, current_ArrayOffset_2D;
	
	const Point2f *Sampler::Get2DArray(int n) {
		...
		if( subArraySizes_2D[current_ArrayOffset_2D] == n && currentPixel_SampleIndex < samples_PerPixel)
			return &sampleArray2D[current_ArrayOffset_2D++][currentPixel_SampleIndex * n];}
	*/
};

//
// ���ز�����������ʱ��ĳ������Ϊ�����ռ䣬��ǰ���ɲ������飬������ø������е�����
//

class PixelSampler : public Sampler
{
public:
	// Ĭ��׼�� 83 ������
	PixelSampler(int64_t samplesPerPixel, int seed = 1234, int nSampledDimensions = 83);
	~PixelSampler() {}

	virtual Float   get_1D() override;
	virtual Point2f get_2D() override;

	// void start_pixel(const Point2i& p) ������ʵ�֣���������ɳ�ʼ����������Ĺ���
	bool next_sample();
	bool set_SampleIndex(int64_t sampleNum);

protected:
	RNG rng;

	std::vector<std::vector<Float>>   sampleArray_1D;
	std::vector<std::vector<Point2f>> sampleArray_2D;
	int current_ArrayOffset_1D = 0, current_ArrayOffset_2D = 0;	  //��ǰ�������±�
};

//
// ȫ�ֲ�����������ʱ������ Film Ϊ�����ռ�
//

class GlobalSampler : public Sampler 
{
public:
	GlobalSampler(int64_t samplesPerPixel) : Sampler(samplesPerPixel) {}
	~GlobalSampler() {}

	virtual Float   get_1D() override;
	virtual Point2f get_2D() override;

	//virtual void start_pixel(const Point2i& p);
	virtual bool next_sample();
	virtual bool set_SampleIndex(int64_t sampleNum);
	
	virtual int64_t get_index_for_sample(int64_t sampleNum) const = 0;
	//��ĳһά���Ͻ��в���
	virtual Float sample_dimension(int64_t index, int dimension) const = 0;

private:
	int dimension;
	int64_t intervalSampleIndex;

	static const int arrayStartDim = 5;
	int arrayEndDim;
};

}	// namespace valley


#endif // VALLEY_CORE_SAMPLER_H

