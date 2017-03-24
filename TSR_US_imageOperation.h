#ifndef __TSR_US_imageOperation_H__
#define __TSR_US_imageOperation_H__

#include "TSR_US_type.h"
#include "stdlib.h"
#include "stdio.h"

/****************************
*OpenCV header
*****************************/
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "ml.h"

typedef struct
{
	uint16_t x;
	uint16_t y;
	int32_t scale;
}
TSR_Point;

typedef struct
{
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
	int32_t index_scale;
	float coarseResponse;
	float fineResponse1;
	float fineResponse2;
	float fineResponse3;
}
TSR_Rect;

typedef struct
{
	uint16_t width;
	uint16_t height;
}
TSR_Size;

typedef struct
{
	int32_t width;
	int32_t height;
	uchar* data;
}
TSR_Image;

typedef struct
{
	int32_t width;
	int32_t height;
	int32_t* data;
	int32_t flag;
}
TSR_IntImage;

typedef struct
{
	float32_t scale;
	TSR_Size orig_window_size;
	TSR_Size origin_slidingWindow;
	float32_t factor_array[50];
	float32_t factor;
	uint16_t  winSize[50][2];
	uint16_t  sz[50][2];
	uint8_t RectCount;

	int32_t step_size_H;
	int32_t step_size_W;
	int32_t large_step_H;
	int32_t large_step_W;
	int32_t neighbor;
} TSR_Cascade;

typedef struct
{
	int coarseFeatNum;
	int fine1FeatNum;
	int fine2FeatNum;
	int fine3FeatNum;
	float32_t aspectR;
	float32_t scaleFactor;
	TSR_Size maxSize;
	TSR_Size coarseSlidingWindow;
	TSR_Size fineSlidingWindow1;
	TSR_Size fineSlidingWindow2;
	TSR_Size frameSize;
	float32_t step;
	float32_t large_step;
	float32_t low_response_thresh;
	float32_t high_response_thresh;
	int8_t skip_interval;
	int32_t coarseMnlbpRows;
	int32_t fineMnlbpRows1;
	int32_t fineMnlbpRows2;
	int32_t system1;
	int32_t system2;
	int32_t systemNum;
	float32_t coarse_response_thresh;
	float32_t fine1_response_thresh;
	float32_t fine2_response_thresh;
	float32_t signMergeRadius;
	float fineResponse1;
	TSR_Size recognitionSize;
	int blockRadius;
	int lastFrameThr;
	int fine_step;
	float32_t tracking_coarse_response_thresh;
	//float32_t tracking_fine_response_thresh;
	int svm_type;
	int kernel_type;
	int nr_class;
	int hog_length;
	float32_t rho[3];
	int32_t label[3];
	int32_t nSV[3];

	char coarse_alpha_file[100];
	char coarse_Gwc_index_file[100];
	char coarse_weak_classifier_table_file[100];
	char fine_alpha_file[100];
	char fine_Gwc_index_file[100];
	char fine_weak_classifier_table_file[100];
	char coarse_reject_threshold_file[100];
	char SVM_trained_file[100];
}
UserDefine;

typedef struct
{
	TSR_Rect sign;
	float coarseResponse;
	float fineResponse1;
	float fineResponse2;
	float fineResponse3;
}DetectSign;

typedef struct
{
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
	int32_t pyramidIndex;
	float32_t detection_response;
	float32_t detection_fine1_response;
	float32_t detection_fine2_response;
	float32_t detection_fine3_response;
	int32_t sign_class;
	int32_t detectionTime;
	int32_t blockIndex;
	int32_t newComingSignFlag;

}TrackingObject;





void createImage(int32_t width, int32_t height, TSR_Image *image);

void createSumImage(int32_t width, int32_t height, TSR_IntImage *image);

int32_t freeImage(TSR_Image* image);

int32_t freeSumImage(TSR_IntImage* image);

void nearestNeighbor(TSR_Image *src, TSR_Image *dst);

void integralImages(TSR_Image *src, TSR_IntImage *sum);

void cvText(IplImage* img, const char* text, int x, int y, CvScalar textColor, double hscale, double vscale, int linewidth);

void subImageCrop(TSR_Image * src, TSR_Image * dst, TSR_Point * p);



#endif /* __TSR_US_imageOperation_H__ */




