#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CAMERA_PERSPECTIVE_H
#define VALLEY_CAMERA_PERSPECTIVE_H

#include"valley.h"
#include"camera.h"

namespace valley
{

/*								   |
								  *|
Image(��ƽ��)		       *   *   |
|		  Lens      * |	   *	   |
|           |*		  | *		   |
|		  * |      *  |			   |
|		*   |  *      |			   |
|	  *    *|         |          Focal
|	*   *   |         |
| *  *      |         |
|*	    	|		  |
| 		    		  |
					View����ƽ�棩
					
ͨ������͸�����ĵ����Ĺ���ȷ��λ�ã��������������߽�����ɫ
��Ȼ�����߿���δͨ����ƽ�����Ӧ���ص�λ�ã����Ⲣû�й�ϵ
����͸���İ뾶Ϊ 0 ʱ����͸����������������
*/

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(const Point3f eye, const Point3f target, const Vector3f up,
		Float fovy, Film* film, Float lensRadius = 0.f, Float focalDistance = 1e6);

	Float generate_ray(const CameraSample& sample, Ray* ray) const;

	Spectrum We(const Ray& ray, Point2f* pRaster2 = nullptr) const;
	void pdf_We(const Ray& ray, Float* pdfPos, Float* pdfDir) const;
	Spectrum sample_Wi(const Interaction& ref, const Point2f& u,
		Vector3f* wi, Float* pdf, Point2f* pRaster, Visibility* vis) const;

private:
	Transform raster_to_camera, camera_to_world, world_to_raster;

	Float lensRadius, focalDistance;	//͸���뾶������
	Float Area;	//��Ƭʵ�ʴ�С
};

}	//namespace valley


#endif //VALLEY_CAMERA_PERSPECTIVE_H
