#ifndef __TSR_US_tracking_H__
#define __TSR_US_tracking_H__
#include "TSR_US_imageOperation.h"


typedef struct
{
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
	int radius;
	int32_t LocIndex;
	int32_t countTimes;
	int32_t lastFrames;
	char signDetectFlag;
	int32_t pyramidIndex;
	int32_t sign_class;
	int sign_class_vote;
	int frame_num;
}
TSR_CountBlock;

typedef struct
{
	float32_t maxResponse;
	int32_t maxx;
	int32_t maxy;
	int32_t maxw;
	int32_t maxh;
	int32_t maxindex_scale;
	float32_t maxdetection_response;
	float32_t maxdetection_fine1_response;
	float32_t maxdetection_fine2_response;
}
MaxTrackingResponse;





void signConfirm(TrackingObject * Loc, int * LocIndex, UserDefine * user, int * LocCT, int * LocCBNum, TSR_CountBlock ** LocCB, int32_t j);
float pointInBlockCheck(TrackingObject Loc, TSR_CountBlock Block, UserDefine * user);
TSR_Rect Tracking(TSR_Image * image, TSR_Cascade * cascade, int32_t scaleNum, UserDefine * user, int LocCT, int * LocCBNum, int i, TSR_CountBlock ** LocCB, MaxTrackingResponse * TrackingResponse);






#endif // !__TSR_US_tracking_H__

