#include "TSR_US_imageOperation.h"



void cvText(IplImage* img, const char* text, int x, int y, CvScalar textColor, double hscale, double vscale, int linewidth)
{
	CvFont font;



	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, hscale, vscale, 0, linewidth, 8);
	CvPoint textPos = cvPoint(x, y);

	cvPutText(img, text, textPos, &font, textColor);
}
/****************************************************************************************
** NAME: createImage
** CALLED BY: Callback_Func, detectObjects
** PARAMETER: int width, int height, MyImage *image
** RETURN VALUE: none
** DESCRIPTION: This function allocates the memory space of type "uint8_t", and use
** that to save an image. Return the pointer of the first element.
** Input:
** "int32_t width" indicates the width of the creating image.
** "int32_t height" indicates the height of the creating image.
** "MyImage * image" is the MyImage type of structure that saves all the information of
** one image.
****************************************************************************************/
void createImage(int32_t width, int32_t height, TSR_Image * image)
{
	image->width = width;
	image->height = height;
	image->data = (uint8_t *)malloc(sizeof(uint8_t)*(height*width));
}
//
/**************************************************************************************************
** NAME: createSumImage
** CALLED BY: detectObjects
** PARAMETER: int width, int height, MyImage *image
** RETURN VALUE: none
** DESCRIPTION: This function allocates the memory space of type "int", and use
** that to save an image. Return the pointer of the first element.
** Input:
** "int32_t width" indicates the width of the creating integral image.
** "int32_t height" indicates the height of the creating integral image.
** "MyIntImage * image" image" is the MyIntImage type of structure that saves all the information of
** one integral image.
***************************************************************************************************/
void createSumImage(int32_t width, int32_t height, TSR_IntImage * image)
{
	image->width = width;
	image->height = height;
	image->data = (int32_t *)malloc(sizeof(int32_t)*(height*width));
}
//
/***************************************************************************************************
** NAME: freeImage
** CALLED BY: detectObjects
** PARAMETER: MyImage *image
** RETURN VALUE: int
** DESCRIPTION: This function release the memory space of the "MyImage" type image. If the memory
** be released successfully, return 0 , otherwise return -1.
** Input:
** "MyImage * image" is the image you want to free the memory.
** Output:
** "int32_t" is 0 if the memory of image be released successfully, otherwise return -1.
****************************************************************************************************/
int32_t freeImage(TSR_Image * image)
{
		free(image->data);
		return 0;
}
//
/**************************************************************************************************
** NAME: freeSumImage
** CALLED BY: detectObjects
** PARAMETER: MyIntImage* image
** RETURN VALUE: int
** DESCRIPTION: This function allocates the memory space of type "int", and use
** that to save an image. Return the pointer of the first element.
** Input:
** "MyIntImage * image" is the integral image you want to free the memory.
** Output:
** "int32_t" is 0 if the memory of integral image be released successfully, otherwise return -1.
***************************************************************************************************/
int32_t freeSumImage(TSR_IntImage * image)
{

		free(image->data);
		return 0;

}
//
//
/****************************************************************************************************************
** NAME: nearestNeighbor
** CALLED BY: detectObjects
** PARAMETER: MyImage *src, MyImage *dst
** RETURN VALUE: none
** DESCRIPTION: This function downsample an image using nearest neighbor. It is used to build the image pyramid.
** Input:
** "MyImage * src" is the original image, all the other small image are downsampled from this one.
** "MyImage * dst" is the output image, which is downsampled by using nearest neighbor.
*****************************************************************************************************************/
void nearestNeighbor(TSR_Image * src, TSR_Image * dst)
{

	int32_t y;
	int32_t j;
	int32_t x;
	int32_t i;
	uint8_t * t;
	uint8_t * p;
	int32_t w1 = src->width;
	int32_t h1 = src->height;
	int32_t w2 = dst->width;
	int32_t h2 = dst->height;
	uint32_t rat = 0;
	uint8_t * src_data = src->data;
	uint8_t * dst_data = dst->data;
	int32_t x_ratio = (int32_t)((w1 << 16) / w2) + 1;
	int32_t y_ratio = (int32_t)((h1 << 16) / h2) + 1;

	for (i = 0; i < h2; i++)
	{
		t = dst_data + i * w2;
		y = ((i * y_ratio) >> 16);
		p = src_data + y * w1;
		rat = 0;
		for (j = 0; j < w2; j++)
		{
			x = (rat >> 16);
			*t = p[x];
			t++;
			rat += x_ratio;
		}
	}
}
//
//
/****************************************************************************************
** NAME: integralImages
** CALLED BY: detectObjects
** PARAMETER: MyImage* _img, MyIntImage* _sum
** RETURN VALUE: none
** DESCRIPTION: At each scale of the image pyramid, compute a new integral image.
** "MyIntImage * _sum" is the structure that save integral image.
** "MyImage * _img" is the structure that save each image pyramid.
** you can find more information: http://en.wikipedia.org/wiki/Summed_area_table
****************************************************************************************/
void integralImages(TSR_Image * src, TSR_IntImage * sum)
{
	int32_t x, y;
	uint8_t it;
	int32_t height = src->height;
	int32_t width = src->width;
	uint8_t * data = src->data;
	int32_t * sumData = (int32_t*)sum->data, t, s;

	for (y = 0; y < height; y++)
	{
		s = 0;
		for (x = 0; x < width; x++)
		{
			it = data[y*width + x];
			s += (int32_t)it;
			t = s;
			if (y != 0)
			{
				t += sumData[(y - 1)*width + x];
			}
			sumData[y*width + x] = t;
		}
	}
}

// Copy certain part of memory to other location
void subImageCrop(TSR_Image * src, TSR_Image * dst, TSR_Point * p)
{	
	unsigned char * s;
	unsigned char * d;
	for (int i = 0; i < dst->height; i++)
	{
		s = src->data + (p->y + i) * src->width + p->x;
		d = dst->data + i * dst->width;
		for (int j = 0; j < dst->width; j++)
		{
			*d = *s;
			d++;
			s++;
		}
	}
}