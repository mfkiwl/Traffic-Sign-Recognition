#include "TSR_US_tracking.h"
#include "TSR_US_detectionCore.h"

#define TSR_MINIMUM_MARGIN   40

void signConfirm(TrackingObject * Loc, int * LocIndex, UserDefine * user, int * LocCT, int * LocCBNum, TSR_CountBlock ** LocCB, int32_t frame)
{

	int temp = 0;
	char addingNum[100];
	memset(addingNum, 1, 100);
	float distance[100][100];
	int minDistanceIndex = 0;
	float minDistance = 10000;

	for (int i = 0; i < (*LocIndex); i++)
	{
		Loc[i].newComingSignFlag = 1;
	}

	for (int j = 0; j < (*LocCBNum); j++)
	{
		minDistance = 10000;
		if (LocCT[j] == 0)
			continue;
		else
		{

			user->blockRadius = LocCB[j][LocCT[j] - 1].radius;
			for (int i = 0; i < (*LocIndex); i++)
			{
				if (Loc[i].newComingSignFlag == 0)
					continue;
				distance[j][i] = pointInBlockCheck(Loc[i], LocCB[j][LocCT[j] - 1], user);
				if ((i == 0) && (distance[j][i] != -1))
				{
					minDistanceIndex = 0;
					minDistance = distance[j][0];
				}
				if ((i != 0) && (distance[j][i] != -1))
				{
					if (distance[j][i] < minDistance)
					{
						minDistance = distance[j][i];
						minDistanceIndex = i;
					}
				}
			}
			if (minDistance != 10000)
			{

				addingNum[j] = 0;
				Loc[minDistanceIndex].newComingSignFlag = 0;
				
				LocCT[j]++;
				LocCB[j][LocCT[j] - 1].signDetectFlag = 1;
				LocCB[j][LocCT[j] - 1].lastFrames = 0;
				LocCB[j][LocCT[j] - 1].x = Loc[minDistanceIndex].x;
				LocCB[j][LocCT[j] - 1].y = Loc[minDistanceIndex].y;
				LocCB[j][LocCT[j] - 1].width = Loc[minDistanceIndex].width;
				LocCB[j][LocCT[j] - 1].height = Loc[minDistanceIndex].height;
				LocCB[j][LocCT[j] - 1].pyramidIndex = Loc[minDistanceIndex].pyramidIndex;
				LocCB[j][LocCT[j] - 1].frame_num = frame + 1;
				Loc[minDistanceIndex].detectionTime = LocCT[j];
				Loc[minDistanceIndex].blockIndex = j;
				if (Loc[minDistanceIndex].width > LocCB[j][LocCT[j] - 2].radius)
					LocCB[j][LocCT[j] - 1].radius = Loc[minDistanceIndex].width;
				else
					LocCB[j][LocCT[j] - 1].radius = LocCB[j][LocCT[j] - 2].radius;
			}

		}
	}

	// new sign location coming
	for (int i = 0; i < (*LocIndex); i++)
	{
		if (Loc[i].newComingSignFlag == 1)
		{

			(*LocCBNum)++;
			temp = (*LocCBNum) - 1;
			addingNum[temp] = 0;
			LocCT[temp]++;
			LocCB[temp][LocCT[temp] - 1].signDetectFlag = 1;
			LocCB[temp][LocCT[temp] - 1].lastFrames = 0;
			LocCB[temp][LocCT[temp] - 1].x = Loc[i].x;
			LocCB[temp][LocCT[temp] - 1].y = Loc[i].y;
			LocCB[temp][LocCT[temp] - 1].width = Loc[i].width;
			LocCB[temp][LocCT[temp] - 1].height = Loc[i].height;
			LocCB[temp][LocCT[temp] - 1].pyramidIndex = Loc[i].pyramidIndex;//
			LocCB[temp][LocCT[temp] - 1].sign_class = Loc[i].sign_class;//
			LocCB[temp][LocCT[temp] - 1].frame_num = frame + 1;
			Loc[i].detectionTime = 1;
			Loc[i].blockIndex = (*LocCBNum) - 1;
			LocCB[temp][LocCT[temp] - 1].radius = Loc[i].width;// renew radius
		}
	}

	for (int i = 1; i <= (*LocCBNum); i++)
	{
		temp = i - 1;
		if (LocCT[temp] == 0)
			continue;
		LocCB[temp][LocCT[temp] - 1].lastFrames += (int)addingNum[temp];
		if (LocCB[temp][LocCT[temp] - 1].lastFrames != 0)
		{
			LocCB[temp][LocCT[temp] - 1].signDetectFlag = 0;
		}
		if (LocCB[temp][LocCT[temp] - 1].lastFrames >= user->lastFrameThr)
			LocCT[temp] = 0;
	}
}

float pointInBlockCheck(TrackingObject Loc, TSR_CountBlock Block, UserDefine * user)
{
	int left_bound = Block.x - 2 * user->blockRadius;
	int right_bound = Block.x + 2 * user->blockRadius;
	int up_bound = Block.y - 2 * user->blockRadius;
	int down_bound = Block.y + 2 * user->blockRadius;

	if ((Loc.x > left_bound) && (Loc.x < right_bound) && (Loc.y > up_bound) && (Loc.y < down_bound))
		return (float)sqrt(pow((double)(Loc.x - Block.x), 2.0) + pow((double)(Loc.y - Block.y), 2.0));
	else
		return (float)-1;
}


TSR_Rect Tracking(TSR_Image * image, TSR_Cascade * cascade, int32_t scaleNum, UserDefine * user, int LocCT, int * LocCBNum, int i, TSR_CountBlock ** LocCB, MaxTrackingResponse * TrackingResponse)
{

	static TSR_Rect allCandidates;
	allCandidates.x = -1;
	allCandidates.y = -1;
	TSR_Image imgResize;
	TSR_IntImage Int_Image;
	TSR_Image * img_resize = &imgResize;
	TSR_IntImage *IntImage = &Int_Image;
	int32_t detect_start = -1, detect_end = -1;
	TSR_Point p;
	int32_t result;
	int32_t y_start, x_start, x, y, x_end, y_end;

	TSR_Rect r;
	float32_t Cumulated_corse, Cumulated_fine2;
	int32_t radius_fine;
	float32_t detection_response_all;
	int left_margin, right_margin;

	//////////////////////////////////////////

	detect_start = -1;
	detect_end = -1;
	if ((LocCB[i][LocCT - 1].pyramidIndex == 0) || (LocCB[i][LocCT - 1].pyramidIndex == 1))
		detect_start = 0;
	if ((LocCB[i][LocCT - 1].pyramidIndex == scaleNum - 1) || (LocCB[i][LocCT - 1].pyramidIndex == scaleNum - 2))
		detect_end = scaleNum - 1;
	if (detect_start == -1)
		detect_start = LocCB[i][LocCT - 1].pyramidIndex - 2;
	if (detect_end == -1)
		detect_end = LocCB[i][LocCT - 1].pyramidIndex + 2;
	TrackingResponse->maxResponse = -1;
	for (int32_t scale_index = detect_start; scale_index <= detect_end; scale_index++)
	{

		TSR_Size winSize = { cascade->winSize[scale_index][0], cascade->winSize[scale_index][1] };

		TSR_Size sz = { cascade->sz[scale_index][0], cascade->sz[scale_index][1] };
		cascade->factor = cascade->factor_array[scale_index];
		createImage(sz.width, sz.height, img_resize);
		createSumImage(sz.width, sz.height, IntImage);
		nearestNeighbor(image, img_resize);
		integralImages(img_resize, IntImage);
		radius_fine = 10;// (TSR_round(LocCB[i][LocCT[i] - 1].radius / cascade->factor));
		x_start = (TSR_round(LocCB[i][LocCT - 1].x / cascade->factor) - radius_fine);
		y_start = (TSR_round(LocCB[i][LocCT - 1].y / cascade->factor) - radius_fine);
		if (y_start < 0)
		{
			y_start = 0;
		}
		if (x_start < 0)
		{
			x_start = 0;
		}
		x_end = x_start + 2 * radius_fine;
		y_end = y_start + 2 * radius_fine;
		if (y_end > IntImage->height)
		{
			y_end = IntImage->height;
		}
		if (x_start > IntImage->width)
		{
			x_start = IntImage->width;
		}
		for (y = y_start; y < y_end; y += user->fine_step)
			for (x = x_start; x < x_end; x += user->fine_step)
			{
				p.x = x;
				p.y = y;

				result = MN_LBP_Calculation_coarse(cascade, &p, IntImage, user, &Cumulated_corse);
				//if ((result == 0) || (Cumulated_corse < user->coarse_response_thresh))
				//	continue;

				//result = MN_LBP_Calculation_fine(cascade, &p, IntImage, user, &Cumulated_fine1, 11);

				//result = MN_LBP_Calculation_fine(cascade, &p, IntImage, user, &Cumulated_fine2, 12);


				if (Cumulated_corse < user->tracking_coarse_response_thresh) 
					continue;

				//if ((Cumulated_fine1 + Cumulated_fine2) < user->tracking_fine_response_thresh)
				//	continue;
				if (TrackingResponse->maxResponse == -1)
				{
					right_margin = user->frameSize.width - (TSR_round(x  * cascade->factor) + winSize.width);
					left_margin = TSR_round(x  * cascade->factor);
					if ((right_margin >= TSR_MINIMUM_MARGIN) && (left_margin >= TSR_MINIMUM_MARGIN)) // if sign is very close to left boundary or right boundary of Image, Stop tracking
					{
						TrackingResponse->maxResponse = Cumulated_corse;
						TrackingResponse->maxx = TSR_round(x  * cascade->factor);
						TrackingResponse->maxy = TSR_round(y  * cascade->factor);
						if ((TrackingResponse->maxx >= user->frameSize.width) || (TrackingResponse->maxy >= user->frameSize.height))
							continue;
						TrackingResponse->maxw = winSize.width;
						TrackingResponse->maxh = r.height = winSize.height;
						TrackingResponse->maxindex_scale = scale_index;
						TrackingResponse->maxdetection_response = Cumulated_corse;
						//TrackingResponse->maxdetection_fine1_response = Cumulated_fine1;
						//TrackingResponse->maxdetection_fine2_response = Cumulated_fine2;
					}

				}
				else
				{
					detection_response_all = Cumulated_corse;
					if (detection_response_all > TrackingResponse->maxResponse)
					{
						right_margin = user->frameSize.width - (TSR_round(x  * cascade->factor) + winSize.width);
						left_margin = TSR_round(x  * cascade->factor);
						if ((right_margin >= TSR_MINIMUM_MARGIN) && (left_margin >= TSR_MINIMUM_MARGIN)) // if sign is very close to left boundary or right boundary of Image, Stop tracking
						{
							TrackingResponse->maxResponse = detection_response_all;
							TrackingResponse->maxx = TSR_round(x  * cascade->factor);
							TrackingResponse->maxy = TSR_round(y  * cascade->factor);
							if ((TrackingResponse->maxx >= user->frameSize.width) || (TrackingResponse->maxy >= user->frameSize.height))
								continue;
							TrackingResponse->maxw = winSize.width;
							TrackingResponse->maxh = r.height = winSize.height;
							TrackingResponse->maxindex_scale = scale_index;
							TrackingResponse->maxdetection_response = Cumulated_corse;
							//TrackingResponse->maxdetection_fine1_response = Cumulated_fine1;
							//TrackingResponse->maxdetection_fine2_response = Cumulated_fine2;
						}
					}
				}
			}
		freeImage(img_resize);
		freeSumImage(IntImage);
	}
	if (TrackingResponse->maxResponse != -1)
	{
		r.coarseResponse = TrackingResponse->maxdetection_response;
		//r.fineResponse1 = TrackingResponse->maxdetection_fine1_response;
		//r.fineResponse2 = TrackingResponse->maxdetection_fine2_response;
		r.width = TrackingResponse->maxw;
		r.height = TrackingResponse->maxh;
		r.index_scale = TrackingResponse->maxindex_scale;
		r.x = TrackingResponse->maxx;
		r.y = TrackingResponse->maxy;
		allCandidates = r;
	}
	return allCandidates;
}