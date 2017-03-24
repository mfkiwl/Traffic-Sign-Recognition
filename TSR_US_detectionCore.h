#ifndef __TSR_US_detectionCore_H__
#define __TSR_US_detectionCore_H__

#include "TSR_US_type.h"
#include "TSR_US_mathOperation.h"
#include "TSR_US_imageOperation.h"




TSR_Rect* detectObjects(TSR_Image * image, TSR_Cascade * cascade, int32_t scaleNum, UserDefine * user);

int32_t SimiCheck(TSR_Rect *_allCandidates, int RectCount, TSR_Rect r);

void SlidingWindow(TSR_Cascade * _cascade, TSR_IntImage *  _IntImage, TSR_Rect *_allCandidates, TSR_Image * _img, int32_t i, UserDefine * user);

int32_t MN_LBP_Calculation_coarse(TSR_Cascade * _cascade, TSR_Point * _p, TSR_IntImage * _IntImage, UserDefine * user, float32_t * Cumulated);

int32_t MN_LBP_Calculation_fine(TSR_Cascade * _cascade, TSR_Point * _p,  TSR_IntImage * _IntImage, UserDefine * user, float32_t * Cumulated, int fineSysSelect);

void boundingBoxMerge(TSR_Rect * differentLoc, int * differentLocIndex, DetectSign * _deSigns, int i, UserDefine * user);

float responseSum(TSR_Rect * _candidateRect);

void findFloatMin(float * src, int numElements, float * minValue, int * index);

#endif __TSR_US_detectionCore_H__

