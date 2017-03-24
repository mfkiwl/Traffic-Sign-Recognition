#include "TSR_US_imageOperation.h"
#include "TSR_US_mathOperation.h"
#include "TSR_US_detectionCore.h"
///****************************************************************************************
//* Global data definitions
//****************************************************************************************/
extern int32_t * GwC_index_array_coarse;
extern float32_t * alpha_array_coarse;
extern float32_t ** weak_classifier_table_array_coarse;
extern uint8_t ** featureCoordinate900;
extern uint8_t ** featureCoordinate11025;
extern uint8_t ** featureCoordinate18225;
extern float32_t * rejectionThreshold;

extern int32_t * GwC_index_array_fine1;
extern float32_t * alpha_array_fine1;
extern float32_t ** weak_classifier_table_array_fine1;

extern int32_t * GwC_index_array_fine2;
extern float32_t * alpha_array_fine2;
extern float32_t ** weak_classifier_table_array_fine2;

extern int32_t * GwC_index_array_fine3;
extern float32_t * alpha_array_fine3;
extern float32_t ** weak_classifier_table_array_fine3;

///*******************************************************************************************************
//** NAME: detectObjects
//** CALLED BY: main funcion
//** PARAMETER: Mat & img, Size & minSize, Size & maxSize, myCascade* cascade, double scaleFactor
//** RETURN VALUE: MyRct *
//** DESCRIPTION: This function build the image pyramid, apply sliding window strategy scan each image.
//** The LBP feature is calculated for each window.
//** Input:
//** "MyImage * image" is the frame that current being processing.
//** "myCascade * cascade" is the parameters that need to build traffic sign detector
//** "uint8_t scaleNum" is the number of different scales in Image pyramid.
//** Output:
//** "MyRect * " is the coordinates, width and height of the candidate sliding window.
//********************************************************************************************************/
TSR_Rect* detectObjects(TSR_Image * image, TSR_Cascade * cascade, int32_t scaleNum, UserDefine * user)
{

	static TSR_Rect allCandidates[500];
	TSR_Image image1Obj;
	TSR_IntImage sum1Obj;
	TSR_Image * img_resize = &image1Obj;
	TSR_IntImage *IntImage = &sum1Obj;
	//////////////////////////////////////////
	for (int32_t i = 0; i < scaleNum; i++)
	{
		TSR_Size winSize = { cascade->winSize[i][0], cascade->winSize[i][1] };
		cascade->origin_slidingWindow = winSize;
		TSR_Size sz = { cascade->sz[i][0], cascade->sz[i][1] };
		cascade->factor = cascade->factor_array[i];
		createImage(sz.width, sz.height, img_resize);
		createSumImage(sz.width, sz.height, IntImage);
		nearestNeighbor(image, img_resize);
		integralImages(img_resize, IntImage);
		SlidingWindow(cascade, IntImage, allCandidates, img_resize, i, user);
		freeImage(img_resize);
		freeSumImage(IntImage);
	}
	return allCandidates;
}

int32_t SimiCheck(TSR_Rect *_allCandidates, int RectCount, TSR_Rect r)
{
	int similarity;
	for (int i = 0; i < RectCount; i++)
	{
		similarity = (r.x - _allCandidates[i].x) + (r.y - _allCandidates[i].y) + (r.width - _allCandidates[i].width) + (r.height - _allCandidates[i].height);
		if (similarity == 0)
		{
			return 0;
		}
	}
	return 1;
}
//
//
/*******************************************************************************************************
** NAME: MN_LBP_Calculation
** CALLED BY: SlidingWindow, SlidingWindow_skip
** PARAMETER: myCascade * _cascade, MyPoint * _p, MySize * _winSize, MyIntImage * _IntImage
** RETURN VALUE: int
** DESCRIPTION: This function receives the sliding window from "SlidingWindow" and build the detector
** logic that all the windows need to pass. The sliding window which can survive at the end of logic
** will be regarded as candidate traffic sign. The window falls to pass any intermediate layer, will
** be rejected as negative.
** Input:
** "myCascade * _cascade" saves all the required parameters to build traffic sign detector.
** "MyPoint * _p" is the coordinate of the top left corner of the current sliding window being processed
** "MyIntImage * _IntImage" is the integral image that being processed
** Output:
** "int32_t" the value can only be 1 or 0, 1 means the current sliding window is traffic sign, 0 represents
** the current sliding window be rejected.
*********************************************************************************************************/
int32_t MN_LBP_Calculation_coarse(TSR_Cascade * _cascade, TSR_Point * _p, TSR_IntImage * _IntImage, UserDefine * user, float32_t * Cumulated)
{
	TSR_Cascade * cascade = _cascade;
	TSR_IntImage * IntImage = _IntImage;
	TSR_Point * p = _p;
	int32_t Numstage = user->coarseFeatNum;
	int32_t stage;



	int32_t * row1, *row2, *row3, *row4;
	float32_t Average, sum_value = 0, result1 = 0, CenterValue;
	int32_t coor_y, coor_x, scale1, scale2, m, n, sqr_scale;
	int32_t Concern_Feature, LBP_Value = 0;
	float32_t alpha, value[8], HypoT;
	float32_t * weak_classifier_table;
	float32_t cumulatedSum = 0;
	int32_t map[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	for (stage = 0; stage < Numstage; stage++)
	{
		sum_value = 0;
		LBP_Value = 0;
		Concern_Feature = GwC_index_array_coarse[stage];
		alpha = alpha_array_coarse[stage];
		weak_classifier_table = weak_classifier_table_array_coarse[stage];
		coor_y = featureCoordinate900[Concern_Feature][0];
		coor_x = featureCoordinate900[Concern_Feature][1];
		scale1 = featureCoordinate900[Concern_Feature][2];
		scale2 = featureCoordinate900[Concern_Feature][3];
		sqr_scale = scale1 * scale2;
		m = p->y + (coor_y - 1);
		n = p->x + (coor_x - 1);

		if (m < scale1 + 1)
			m = scale1 + 1;
		if (n < scale2 + 1)
			n = scale2 + 1;

		row1 = &IntImage->data[(m - scale1 - 1) * IntImage->width];
		row2 = &IntImage->data[(m - 1) * IntImage->width];
		row3 = &IntImage->data[(m + scale1 - 1) * IntImage->width];
		row4 = &IntImage->data[(m + 2 * scale1 - 1) * IntImage->width];

		CenterValue = (float32_t)((row3[n + scale2 - 1] - row3[n - 1] - row2[n + scale2 - 1] + row2[n - 1]) / sqr_scale); 
		value[0] = (float32_t)((row2[n - 1] - row2[n - scale2 - 1] - row1[n - 1] + row1[n - scale2 - 1]) / sqr_scale); 
		value[1] = (float32_t)((row2[n + scale2 - 1] - row2[n - 1] - row1[n + scale2 - 1] + row1[n - 1]) / sqr_scale);
		value[2] = (float32_t)((row2[n + 2 * scale2 - 1] - row2[n + scale2 - 1] - row1[n + 2 * scale2 - 1] + row1[n + scale2 - 1]) / sqr_scale);
		value[3] = (float32_t)((row3[n + 2 * scale2 - 1] - row3[n + scale2 - 1] - row2[n + 2 * scale2 - 1] + row2[n + scale2 - 1]) / sqr_scale);
		value[4] = (float32_t)((row4[n + 2 * scale2 - 1] - row4[n + scale2 - 1] - row3[n + 2 * scale2 - 1] + row3[n + scale2 - 1]) / sqr_scale);
		value[5] = (float32_t)((row4[n + scale2 - 1] - row4[n - 1] - row3[n + scale2 - 1] + row3[n - 1]) / sqr_scale);
		value[6] = (float32_t)((row4[n - 1] - row4[n - scale2 - 1] - row3[n - 1] + row3[n - scale2 - 1]) / sqr_scale);
		value[7] = (float32_t)((row3[n - 1] - row3[n - scale2 - 1] - row2[n - 1] + row2[n - scale2 - 1]) / sqr_scale);

		for (int32_t it = 0; it < 8; it++)
			sum_value = sum_value + value[it];
		Average = (sum_value + CenterValue) / 9;

		for (int32_t it = 0; it < 8; it++)
		{
			if (value[it] >= Average)
				LBP_Value |= map[it];
			else
				;
		}

		HypoT = (weak_classifier_table[LBP_Value]) * alpha;
		cumulatedSum = cumulatedSum + HypoT;

		//HypoT = (weak_classifier_table[LBP_Value]);
		//cumulatedSum = cumulatedSum + HypoT;
		
		(*Cumulated) = cumulatedSum;
		if (cumulatedSum <= rejectionThreshold[stage])
		{
			return 0;
		}
	}
	return 1;
}


/*******************************************************************************************************************
** NAME: SlidingWindow
** CALLED BY: detectObjects
** PARAMETER: myCascade* _cascade, MyIntImage*  _IntImage, MyRect *_allCandidates, MyImage* _img, int32_t i
** RETURN VALUE: none
** DESCRIPTION: This function using sliding window strategy to scan the whole image, and search for
** the possible position of traffic sign. Once a candidate found,save its coordinates and scales.
** Input:
** "myCascade* _cascade" saves all the parameters needed to build detector.
** "MyIntImage * _IntImage" is the integral image which being processing.
** "MyRect * _allCandidates" is a MyRect type of array, that used to save candidate sign location, width and height
** "ing32_t i" indicates which level of image pyramid, that the current integral image in.
*********************************************************************************************************************/
void SlidingWindow(TSR_Cascade * _cascade, TSR_IntImage *  _IntImage, TSR_Rect *_allCandidates, TSR_Image * _img, int32_t i, UserDefine * user)
{

	TSR_Cascade * cascade = _cascade;
	TSR_IntImage * IntImage = _IntImage;
	TSR_Image * img = _img;
	TSR_Point p;
	int32_t result, x1, x2, y1, y2;
	int32_t step_size_H = cascade->step_size_H, step_size_W = cascade->step_size_W;
	TSR_Size winSize0 = user->coarseSlidingWindow; // The scale of sliding window
	TSR_Size winSize = cascade->origin_slidingWindow; // the scale of the sliding window in the largest image pyramid
	TSR_Rect r;
	float32_t Cumulated;
	int fineStartx, fineEndx, fineStarty, fineEndy;

	y1 = 0;
	x1 = 0;
	y2 = TSR_round((float32_t)(0.558 * IntImage->height)) - winSize0.height - 1;
	x2 = IntImage->width - winSize0.width;

	for (int32_t y = y1; y < y2; y += step_size_H)
	for (int32_t x = x1; x < x2; x += step_size_W)
	{
		p.x = x;
		p.y = y;

		step_size_H = cascade->step_size_H;
		step_size_W = cascade->step_size_W;

		result = MN_LBP_Calculation_coarse(cascade, &p, IntImage, user, &Cumulated);
		if (result == 1)
		{
			r.coarseResponse = Cumulated;
			if (r.coarseResponse >= user->coarse_response_thresh)
			{
				r.x = TSR_round(x  * cascade->factor);
				r.y = TSR_round(y  * cascade->factor);
				r.width = winSize.width;
				r.height = winSize.height;
				r.index_scale = i;
				_allCandidates[cascade->RectCount] = r;
				cascade->RectCount++;
			}
		}
		/* if response less than the lower threshold, using large step size*/
		if ((x != x1) && (Cumulated < user->low_response_thresh))
		{
			step_size_H = cascade->large_step_H;
			step_size_W = cascade->large_step_W;
		}
		/* if response greater than the upper threshold, using small step size*/
		if ((x != x1) && (Cumulated >= user->high_response_thresh))
		{
			fineStartx = x - cascade->neighbor;
			if (fineStartx < 0)
				fineStartx = 0;
			fineEndx = x + cascade->neighbor;
			if (fineEndx >= x2)
				fineEndx = x2 - 1;
			fineStarty = y - cascade->neighbor;
			if (fineStarty < 0)
				fineStarty = 0;
			fineEndy = y + cascade->neighbor;
			if (fineEndy >= y2)
				fineEndy = y2 - 1;

			for (int neighbx = fineStartx; neighbx <= fineEndx; neighbx++)
			{
				for (int neighby = fineStarty; neighby <= fineEndy; neighby++)
				{
					if ((neighbx == x) && (neighby == y))
					{
						continue;
					}
					p.x = neighbx;
					p.y = neighby;

					result = MN_LBP_Calculation_coarse(cascade, &p, IntImage, user, &Cumulated);
					if (result == 1)
					{
						r.coarseResponse = Cumulated;
						if (r.coarseResponse < user->coarse_response_thresh)
							continue;
						r.x = TSR_round(neighbx  * cascade->factor);
						r.y = TSR_round(neighby  * cascade->factor);
						r.width = winSize.width;
						r.height = winSize.height;
						r.index_scale = i;
						if (SimiCheck(_allCandidates, cascade->RectCount, r) == 1)
						{
							_allCandidates[cascade->RectCount] = r;
							cascade->RectCount++;
						}
					}
				}
			}
		}
	}
}


int32_t MN_LBP_Calculation_fine(TSR_Cascade * _cascade, TSR_Point * _p, TSR_IntImage * _IntImage, UserDefine * user, float32_t * Cumulated, int fineSysSelect)
{
	TSR_Cascade * cascade = _cascade;
	TSR_IntImage * IntImage = _IntImage;
	int32_t Numstage;
	if ((fineSysSelect == 1) || (fineSysSelect == 11))
		Numstage = user->fine1FeatNum;
	if ((fineSysSelect == 2) || (fineSysSelect == 12))
		Numstage = user->fine2FeatNum;
	if ((fineSysSelect == 3) || (fineSysSelect == 13))
		Numstage = user->fine3FeatNum;
		
	

	int32_t stage;
	TSR_Point *p = _p;


	int32_t * row1, *row2, *row3, *row4;
	float32_t Average, sum_value = 0, result1 = 0, CenterValue;
	int32_t coor_y, coor_x, scale1, scale2, m, n, sqr_scale;
	int32_t Concern_Feature, LBP_Value = 0;
	float32_t alpha, value[8], HypoT;
	float32_t * weak_classifier_table = NULL;
	float32_t cumulatedSum = 0;
	int32_t map[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	for (stage = 0; stage < Numstage; stage++)
	{
		sum_value = 0;
		LBP_Value = 0;
		if ((fineSysSelect == 1) || (fineSysSelect == 11))
		{
			Concern_Feature = GwC_index_array_fine1[stage];
			alpha = alpha_array_fine1[stage];
			weak_classifier_table = weak_classifier_table_array_fine1[stage];
		}
		if ((fineSysSelect == 2) || (fineSysSelect == 12))
		{
			
			Concern_Feature = GwC_index_array_fine2[stage];
			alpha = alpha_array_fine2[stage];
			weak_classifier_table = weak_classifier_table_array_fine2[stage];
		}

		if ((fineSysSelect == 3) || (fineSysSelect == 13))
		{
			Concern_Feature = GwC_index_array_fine3[stage];
			alpha = alpha_array_fine3[stage];
			weak_classifier_table = weak_classifier_table_array_fine3[stage];
		}
		if (fineSysSelect == 1 || fineSysSelect == 2 || fineSysSelect == 11 || fineSysSelect == 12)
		{
			coor_y = featureCoordinate11025[Concern_Feature][0];
			coor_x = featureCoordinate11025[Concern_Feature][1];
			scale1 = featureCoordinate11025[Concern_Feature][2];
			scale2 = featureCoordinate11025[Concern_Feature][3];
		}
		if (fineSysSelect == 3 || fineSysSelect == 13)
		{
			coor_y = featureCoordinate18225[Concern_Feature][0];
			coor_x = featureCoordinate18225[Concern_Feature][1];
			scale1 = featureCoordinate18225[Concern_Feature][2];
			scale2 = featureCoordinate18225[Concern_Feature][3];
		}
		sqr_scale = scale1 * scale2;
		if ((fineSysSelect == 1) || (fineSysSelect == 2) || (fineSysSelect == 3))
		{
			m = 0 + (coor_y - 1);
			n = 0 + (coor_x - 1);
		}
		if ((fineSysSelect == 11) || (fineSysSelect == 12) || (fineSysSelect == 13))
		{
			m = p->y + (coor_y - 1);
			n = p->x + (coor_x - 1);
		}
		if (m < scale1 + 1)
			m = scale1 + 1;
		if (n < scale2 + 1)
			n = scale2 + 1;

		row1 = &IntImage->data[(m - scale1 - 1) * IntImage->width];
		row2 = &IntImage->data[(m - 1) * IntImage->width];
		row3 = &IntImage->data[(m + scale1 - 1) * IntImage->width];
		row4 = &IntImage->data[(m + 2 * scale1 - 1) * IntImage->width];

		CenterValue = (float32_t)((row3[n + scale2 - 1] - row3[n - 1] - row2[n + scale2 - 1] + row2[n - 1]) / sqr_scale);
		value[0] = (float32_t)((row2[n - 1] - row2[n - scale2 - 1] - row1[n - 1] + row1[n - scale2 - 1]) / sqr_scale);
		value[1] = (float32_t)((row2[n + scale2 - 1] - row2[n - 1] - row1[n + scale2 - 1] + row1[n - 1]) / sqr_scale);
		value[2] = (float32_t)((row2[n + 2 * scale2 - 1] - row2[n + scale2 - 1] - row1[n + 2 * scale2 - 1] + row1[n + scale2 - 1]) / sqr_scale);
		value[3] = (float32_t)((row3[n + 2 * scale2 - 1] - row3[n + scale2 - 1] - row2[n + 2 * scale2 - 1] + row2[n + scale2 - 1]) / sqr_scale);
		value[4] = (float32_t)((row4[n + 2 * scale2 - 1] - row4[n + scale2 - 1] - row3[n + 2 * scale2 - 1] + row3[n + scale2 - 1]) / sqr_scale);
		value[5] = (float32_t)((row4[n + scale2 - 1] - row4[n - 1] - row3[n + scale2 - 1] + row3[n - 1]) / sqr_scale);
		value[6] = (float32_t)((row4[n - 1] - row4[n - scale2 - 1] - row3[n - 1] + row3[n - scale2 - 1]) / sqr_scale);
		value[7] = (float32_t)((row3[n - 1] - row3[n - scale2 - 1] - row2[n - 1] + row2[n - scale2 - 1]) / sqr_scale);

		for (int32_t it = 0; it < 8; it++)
			sum_value = sum_value + value[it];
		Average = (sum_value + CenterValue) / 9;

		for (int32_t it = 0; it < 8; it++)
		{
			if (value[it] >= Average)
				LBP_Value |= map[it];
			else
				;
		}
		//HypoT = (weak_classifier_table[LBP_Value]) * alpha;
		//cumulatedSum = cumulatedSum + HypoT;

		HypoT = (weak_classifier_table[LBP_Value]);
		cumulatedSum = cumulatedSum + HypoT;

		(*Cumulated) = cumulatedSum;
	}

	//if ((cumulatedSum < user->fine1_response_thresh) && ((fineSysSelect == 1) || (fineSysSelect == 3)))
	//{
	//	return 0;
	//}
	//if ((cumulatedSum < user->fine2_response_thresh) && ((fineSysSelect == 2) || (fineSysSelect == 4)))
	//{
	//	return 0;
	//}
	return 1;

}

void boundingBoxMerge(TSR_Rect * differentLoc, int * differentLocIndex, DetectSign * _deSigns, int i, UserDefine * user)
{
	TSR_Rect boundingBox = (*_deSigns).sign;

	float signDistance[20], signMinDistance = 0;
	int signMinIndex = 0;
	if (i == 0)// If this is the first sign
	{
		differentLoc[(*differentLocIndex)] = boundingBox;
		(*differentLocIndex)++;
	}
	else
	{
		for (int j = 0; j < *differentLocIndex; j++)
		{
			signDistance[j] = (float)sqrt(pow(boundingBox.x - differentLoc[j].x, 2) + pow(boundingBox.y - differentLoc[j].y, 2));
		}
		findFloatMin(signDistance, *differentLocIndex, &signMinDistance, &signMinIndex);
		if (signMinDistance > user->signMergeRadius)
		{
			differentLoc[(*differentLocIndex)] = boundingBox;
			(*differentLocIndex)++;
		}
		else
		{
			if (responseSum(&differentLoc[signMinIndex]) < responseSum(&boundingBox))
			{
				differentLoc[signMinIndex] = boundingBox;
			}
		}
	}
}

float responseSum(TSR_Rect * _candidateRect)
{
	return((_candidateRect->coarseResponse) + (_candidateRect->fineResponse1));
}


void findFloatMin(float * src, int numElements, float * minValue, int * index)
{
	(*minValue) = src[0];
	(*index) = 0;
	for (int i = 1; i < numElements; i++)
	{
		if (src[i] < (*minValue))
		{
			(*minValue) = src[i];
			(*index) = i;
		}
	}
}