/****************************************************************************************
| Project Name: TSR
| Hardware:
| Description:  US traffic sign detection and recognition algorithm
| File Name: TSR_US_main.c
|
|----------------------------------------------------------------------------------------
| C O P Y R I G H T
|----------------------------------------------------------------------------------------
|----------------------------------------------------------------------------------------
| A U T H O R I D E N T I T Y
|----------------------------------------------------------------------------------------
| Initials  Name                 Contact
| -------- --------------------- ----------------------
| T.Wang   Tianyu WANG          twang@mobis-usa.com
|
|----------------------------------------------------------------------------------------
| R E V I S I O N H I S T O R Y
|----------------------------------------------------------------------------------------
| Date        Version Author Description
| ----------  ------- ------ --------------------------------------------------------------
| 2015-03-09  1.00.00 T.Wang Creation
| 2015-10-28  2.00.00 T.Wang Modification(rebuild the system with soft cascading adaboost)
| 2016-03-15  3.00.00 T.Wang Modification(coarse to fine detection architecture, recognition and tracking)
|***************************************************************************************/

/****************************************************************************************
* Include files
****************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "io.h"
#include "direct.h"
#include "string.h"
#include "time.h"

/****************************
*OpenCV header
*****************************/
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "ml.h"


#include "TSR_US_imageOperation.h"
#include "TSR_US_readTxt.h"
#include "TSR_US_binLoading.h"
#include "TSR_US_mathOperation.h" 
#include "TSR_US_detectionCore.h"
#include "TSR_US_recognitionCore.h"
#include "TSR_US_HOGcalculation.h"
#include "TSR_US_tracking.h"

/****************************************************************************************
* Defines
****************************************************************************************/
#define TSR_TRACKING_ENABLE  1
#define TSR_COUNT_BLOCK_SIZE 300
#define TSR_MINIMUM_MARGIN   40

/****************************************************************************************
* Local constants
****************************************************************************************/
extern FILE * m_pVideoFp;
extern bool_t m_bFrameChangeFlag;
extern IplImage * m_pViewImg, *m_pViewImg_1C;
extern unsigned long long int m_nFileLength, m_nFileCurpos;
UserDefine _user;
UserDefine  * user = &_user;
int32_t start, end, Timecost_detection = 0, Timecost_SVM_calculation = 0, Timecost_HOG_calculation = 0;

/****************************************************************************************
** NAME: main
** CALLED BY: none
** PARAMETER: none
** RETURN VALUE: int
** DESCRIPTION: The main function of U.S traffic sign detection
**
****************************************************************************************/
int32_t main(int32_t argc, int8_t *argv[])
{
	/* TSR user interface explanation */
	printf("<<<This Software Belongs to MOBIS-NORTH-AMERICA LLC>>>\n");
	printf("1. Please contact Jungme Park or Tianyu Wang for more info\n");
	printf("2. please make sure input all the parameters \nin the 'Input_Parameters.txt' before run this program\n");
	printf("\n");
	printf("Please type anything to continue\n");
	char input_anything;
	scanf("%c", &input_anything);
	
	/****************************************************************************************
	* User input parameters
	****************************************************************************************/
	user->coarseFeatNum = 143; 
	user->fine1FeatNum = 499;
	user->fine2FeatNum = 391;
	user->fine3FeatNum = 542;
	user->aspectR = 1.0f;
	user->scaleFactor = 1.1f;
	user->maxSize.width = TSR_round(170.0f * user->aspectR);
	user->maxSize.height = 170;
	user->coarseSlidingWindow.height = 15;
	user->coarseSlidingWindow.width = 15;
	user->fineSlidingWindow1.height = 30;
	user->fineSlidingWindow1.width = 30;
	user->fineSlidingWindow2.height = 36;
	user->fineSlidingWindow2.width = 36;
	user->frameSize.height = 672;
	user->frameSize.width = 1280;

	/* Load "Input_Parameters.txt" */
	FILE * Input_Parameters_File;
	errno_t errFile;
	int8_t Input_Parameters_String[100];
	int8_t * delim1 = " = ", *delim2 = ";",  * begin, *remain;

	/* Loading soft cascade adaboost parameter 'aplha' */
	errFile = fopen_s(&Input_Parameters_File, "Input_Parameters.txt", "r");
	if (errFile != 0)
		printf("The file 'Input_Parameters.txt' was not opened\n");
	else
		printf("The file 'Input_Parameters.txt' was opened\n");

	fgets(Input_Parameters_String, 100, Input_Parameters_File); // Skip One Row

	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->step = (float32_t)atof(begin);
	}// load Step Size
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->large_step = (float32_t)atof(begin);
	}// load Large Step Size
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->low_response_thresh = (float32_t)atof(begin);
	}// load Low Response Threshold for Adaptive Scanning
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->high_response_thresh = (float32_t)atof(begin);
	}// load High Response Threshold for Adaptive Scanning
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->fine1_response_thresh = (float32_t)atof(begin);
	}// load Fine System Final Response Threshold
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->fine2_response_thresh = (float32_t)atof(begin);
	}// load Fine System Final Response Threshold
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->tracking_coarse_response_thresh = (float32_t)atof(begin);
	}// load Tracking Coarse Response Threshold
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->signMergeRadius = (float32_t)atof(begin);
	}// load Sign Merge Distance
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->lastFrameThr = atoi(begin);
	}// load Sign Last Frame Threshold

	fgets(Input_Parameters_String, 100, Input_Parameters_File); // Skip One Row
	fgets(Input_Parameters_String, 100, Input_Parameters_File); // Skip One Row

	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 2, delim2, &remain);
		strcpy(user->coarse_alpha_file, begin);
	}// load coarse_alpha_file
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 2, delim2, &remain);
		strcpy(user->coarse_Gwc_index_file, begin);
	}// load coarse_Gwc_index_file
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 2, delim2, &remain);
		strcpy(user->coarse_weak_classifier_table_file, begin);
	}// load coarse_weak_classifier_table_file
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 2, delim2, &remain);
		strcpy(user->coarse_reject_threshold_file, begin);
	}// load coarse_reject_threshold_file
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 2, delim2, &remain);
		strcpy(user->fine_alpha_file, begin);
	}// load fine_alpha_file
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 2, delim2, &remain);
		strcpy(user->fine_Gwc_index_file, begin);
	}// load fine_Gwc_index_file
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 2, delim2, &remain);
		strcpy(user->fine_weak_classifier_table_file, begin);
	}// load fine_weak_classifier_table_file
	fgets(Input_Parameters_String, 100, Input_Parameters_File); // Skip One Row
	fgets(Input_Parameters_String, 100, Input_Parameters_File); // Skip One Row

	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 2, delim2, &remain);
		strcpy(user->SVM_trained_file, begin);
	}// load SVM_trained_file

	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->svm_type = atoi(begin);
	}// load SVM_type
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->kernel_type = atoi(begin);
	}// load kernel_type
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->nr_class = atoi(begin);
	}// load nr_class
	if (fgets(Input_Parameters_String, 100, Input_Parameters_File) != NULL)
	{
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->hog_length = atoi(begin);
	}// load hog_length

	for (int index = 0; index < user->nr_class; index++)
	{
		fgets(Input_Parameters_String, 100, Input_Parameters_File);
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->rho[index] = (float32_t)atof(begin);
	}// load rho
	for (int index = 0; index < user->nr_class; index++)
	{
		fgets(Input_Parameters_String, 100, Input_Parameters_File);
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->label[index] = atoi(begin);
	}// load label
	for (int index = 0; index < user->nr_class; index++)
	{
		fgets(Input_Parameters_String, 100, Input_Parameters_File);
		begin = strtok_s(Input_Parameters_String, delim1, &remain);
		begin = strtok_s(remain + 1, delim2, &remain);
		user->nSV[index] = atoi(begin);
	}// load nSV
	fclose(Input_Parameters_File);

	printf("<<Parameters List : >>\n");
	printf("Coarse_Feature_Number = 143\n");
	printf("Fine1_Feature_Number = 709\n");
	printf("Fine2_Feature_Number = 535\n");
	printf("Coarse_Window_Size = 15 * 15\n");
	printf("Fine_Window_Size = 30 * 30\n");
	printf("Normal_Step_Size = %f\n", user->step);
	printf("Large_Step_Size = %f\n", user->large_step);
	printf("Adaptive_Low_Response_Threshold = %f\n", user->low_response_thresh);
	printf("Adaptive_High_Response_Threshold = %f\n", user->high_response_thresh);
	printf("System_Fine1_Threshold = %f\n", user->fine1_response_thresh);
	printf("System_Fine2_Threshold = %f\n", user->fine2_response_thresh);
	printf("Tracking_Coarse_Response_Thresh = %f\n", user->tracking_coarse_response_thresh);
	printf("Sign_Merge_Radius = %f\n", user->signMergeRadius);
	printf("Last_Frame_Thresh = %d\n", user->lastFrameThr);
	printf("\n");


	/****************************************************************************************
	* Sign Merge
	****************************************************************************************/
	TSR_Rect diffLoc[250]; //Save all the different signs location
	int diffLocIndex = 0; //Record how many different signs detected
	DetectSign deSigns[300]; //Save all the signs location that is detected
	int deSignsIndex;
	int left_margin, right_margin;

	/****************************************************************************************
	* Sign Tracking
	****************************************************************************************/
	user->blockRadius = 70;
	user->fine_step = 1;
	int locCountTimes[TSR_COUNT_BLOCK_SIZE];
	memset(locCountTimes, 0, TSR_COUNT_BLOCK_SIZE);
	int locCountBlockNum = 0;
	TSR_CountBlock ** locCountBlock;
	locCountBlock = (TSR_CountBlock **)malloc(sizeof(TSR_CountBlock *)* TSR_COUNT_BLOCK_SIZE);
	for (int i = 0; i < TSR_COUNT_BLOCK_SIZE; i++)
	{
		locCountBlock[i] = (TSR_CountBlock *)malloc(sizeof(TSR_CountBlock)* TSR_COUNT_BLOCK_SIZE);
	}
	MaxTrackingResponse TrackingResponse;

	/****************************************************************************************
	* Sign Recognition
	****************************************************************************************/
	svm_model* model;
	model = svm_load_model(user->SVM_trained_file);
	if (model == 0)
	{
		printf("Can't open SVM model file.\n");
		return 0;
	}
	else
	{
		printf("SVM model file open successfully.\n");
	}

	
	/****************************************************************************************
	* HOG + SVM Recognition
	****************************************************************************************/
	user->recognitionSize.width = 48;
	user->recognitionSize.height = 64;
	char featureBuffer[100000] = "0 ";	//0 is not the target label of the samples

	/****************************************************************************************
	* Local data definitions
	****************************************************************************************/
	int32_t i, j, FrameLength = 0, BinNum;
	TSR_Rect * result;
	char * begin_str = NULL, *next_str = NULL;
	const char * split = ".";
	int32_t scaleNum = 0;
	double fps = 15;
	float32_t  TimeCostWS = 0, TimeCostWWS = 0;
	TSR_Cascade  _cascade;
	TSR_Cascade * cascade = &_cascade;
	TSR_Image _img;
	TSR_Image * img = &_img;

	cascade->step_size_H = TSR_round((float32_t)user->coarseSlidingWindow.height * user->step);
	cascade->step_size_W = TSR_round((float32_t)user->coarseSlidingWindow.width * user->step);
	cascade->large_step_H = TSR_round((float32_t)user->coarseSlidingWindow.height * user->large_step);  
	cascade->large_step_W = TSR_round((float32_t)user->coarseSlidingWindow.width * user->large_step);
	cascade->neighbor = 2;
	cascade->RectCount = 0;
	/****************************************************************************************
	* Preparation for Image Pyramid
	****************************************************************************************/
	for (float32_t factor = 2.0f;; factor *= user->scaleFactor)
	{
		if (TSR_round((float32_t)user->frameSize.width / factor) <= user->coarseSlidingWindow.width || TSR_round((float32_t)user->frameSize.height / factor) <= user->coarseSlidingWindow.height)
			break;
		if (TSR_round((float32_t)user->coarseSlidingWindow.width  * factor) > user->maxSize.width || TSR_round((float32_t)user->coarseSlidingWindow.height * factor) > user->maxSize.height)
			break;
		cascade->winSize[scaleNum][0] = TSR_round((float32_t)user->coarseSlidingWindow.width * factor);
		cascade->winSize[scaleNum][1] = TSR_round((float32_t)user->coarseSlidingWindow.height * factor);
		cascade->sz[scaleNum][0] = TSR_round((float32_t)user->frameSize.width / factor);
		cascade->sz[scaleNum][1] = TSR_round((float32_t)user->frameSize.height / factor);
		cascade->factor_array[scaleNum] = factor;
		scaleNum++;
	}

	/****************************************************************************************
	* Read parameters to build detector
	****************************************************************************************/
	readTextClassifier(user);

	intptr_t file;
	struct _finddata_t find;
	if ((file = _findfirst("BIN\\*.bin", &find)) == -1L)
	{
		printf("NO VALID VIDEO FILE FOUND!\n");
		exit(0);
	}

	img->width = user->frameSize.width;
	img->height = user->frameSize.height;
	BinNum = 1;

	/* variables for fine detection*/
	TSR_Image _subImage;
	TSR_Image * subImage = &_subImage;
	TSR_Image _subResizeImage;
	TSR_Image * subResizeImage = &_subResizeImage;
	TSR_Point _startPoint;
	TSR_Point * startPoint = &_startPoint;
	TSR_IntImage _IntImage;
	TSR_IntImage * IntSubImage = &_IntImage;
	TSR_Rect r;
	float fineResponse1;
	float fineResponse2;
	float fineResponse3;
	int fineResult1;
	int fineResult2;
	int fineResult3;
	TSR_Point _p;
	TSR_Point * p = &_p;

	while (1)
	{
		Timecost_detection = 0;
		printf("\n-- Start Processing Video --\r\n");
		char  output_video_name[100] = { "Output_AVI\\" };
		char  input_video_name[300] = { "BIN\\" };
		char  input_xml_name[100] = { "XML\\" };
		//int8_t  detection_window_name[300];

		strcat_s(input_video_name, sizeof(input_video_name), find.name);
		begin_str = strtok_s(find.name, split, &next_str);
		strcat_s(output_video_name, sizeof(output_video_name), begin_str);
		strcat_s(output_video_name, sizeof(output_video_name), ".avi");
		strcat_s(input_xml_name, sizeof(input_xml_name), begin_str);
		strcat_s(input_xml_name, sizeof(input_xml_name), ".xml");

		/* Using Opencv writing video data to AVI */
		CvSize size = cvSize(1280, 672);
		CvVideoWriter* writer = cvCreateVideoWriter(output_video_name, CV_FOURCC('D', 'I', 'V', 'X'), fps, size, 1);

		LoadVideo(input_video_name);
		FrameLength = FrameLength + (int32_t)m_nFileLength;
		TSR_IntImage sum1Obj;
		TSR_IntImage *IntImage = &sum1Obj;
		
		locCountBlockNum = 0;
		memset(locCountTimes, 0, TSR_COUNT_BLOCK_SIZE);
		for (int i = 0; i < TSR_COUNT_BLOCK_SIZE; i++)
		{
			memset(locCountBlock[i], 0, TSR_COUNT_BLOCK_SIZE);
		}

		for (j = 0; j < m_nFileLength; j++)
		{
			Timecost_HOG_calculation = 0;
			Timecost_SVM_calculation = 0;
			Callback_Func();
			img->data = (uchar *)m_pViewImg_1C->imageData;
			m_bFrameChangeFlag = 1;
			/* Time used for Coarse Detection*/
/******************************Start***********************************/
			start = (int32_t)clock(); 
			result = detectObjects(img, cascade, scaleNum, user);
			end = (int32_t)clock();
			Timecost_detection += (end - start);
/******************************End***********************************/
			char putText[100];

			createImage(user->fineSlidingWindow1.width, user->fineSlidingWindow1.height, subResizeImage);

			createSumImage(user->fineSlidingWindow1.width, user->fineSlidingWindow1.height, IntSubImage);
			/* Time used for Fine Detection*/
/******************************Start***********************************/
			start = (int32_t)clock();
			deSignsIndex = 0;
			diffLocIndex = 0;
	    	for (i = 0; i < cascade->RectCount; i++)
			{
				
				fineResult1 = 0;
				fineResponse1 = 0;
				fineResult2 = 0;
				fineResponse2 = 0;
				r = result[i];

				startPoint->x = r.x;
				startPoint->y = r.y;
				createImage(r.width, r.height, subImage);
				
				subImageCrop(img, subImage, startPoint);
				nearestNeighbor(subImage, subResizeImage);

				integralImages(subResizeImage, IntSubImage);
				
				 //Fine Detection1
				p->x = 0; p->y = 0;

				fineResult1 = MN_LBP_Calculation_fine(cascade, p, IntSubImage, user, &fineResponse1, 1);
				if (fineResult1 == 1)
				{
					if (fineResponse1 > user->fine1_response_thresh)
					{
						//Fine Detection2
						fineResult2 = MN_LBP_Calculation_fine(cascade, p, IntSubImage, user, &fineResponse2, 2);
						if (fineResult2 == 1)
						{
							//if ((fineResponse1 + fineResponse2) > user->fine_response_thresh)
							if (fineResponse2 > user->fine2_response_thresh)
							{
								fineResult3 = 0;
								fineResponse3 = 0;

								freeImage(subResizeImage);
								freeSumImage(IntSubImage);
								createImage(user->fineSlidingWindow2.width, user->fineSlidingWindow2.height, subResizeImage);

								createSumImage(user->fineSlidingWindow2.width, user->fineSlidingWindow2.height, IntSubImage);
								nearestNeighbor(subImage, subResizeImage);
								integralImages(subResizeImage, IntSubImage);
								fineResult3 = MN_LBP_Calculation_fine(cascade, p, IntSubImage, user, &fineResponse3, 3);
								if (fineResponse3 > 265.2f)
								{
									right_margin = user->frameSize.width - (r.x + r.width);
									left_margin = r.x;
									if ((right_margin >= TSR_MINIMUM_MARGIN) && (left_margin >= TSR_MINIMUM_MARGIN)) // if sign is very close to left boundary or right boundary of Image, Stop tracking
									{
										deSignsIndex++;
										r.fineResponse1 = fineResponse1;
										r.fineResponse2 = fineResponse2;
										deSigns[deSignsIndex - 1].sign = r;
										deSigns[deSignsIndex - 1].coarseResponse = r.coarseResponse;
										deSigns[deSignsIndex - 1].fineResponse1 = fineResponse1;
										deSigns[deSignsIndex - 1].fineResponse2 = fineResponse2;
										deSigns[deSignsIndex - 1].fineResponse3 = fineResponse3;
									}
								}

							}
						}
					}
				}
				freeImage(subImage);
			}

			/****************************************************************************************
			* Merge Signs, Tracking, Plot rectangles on Image
			****************************************************************************************/
			//Check how many signs detected, if one, just show one boundingbox. 
			//If two sign detected, start 'mergeBoundingBox'
			if (deSignsIndex == 1)
			{

					diffLoc[0] = deSigns[0].sign;
					diffLocIndex = 1;
					cvRectangle(m_pViewImg, cvPoint(deSigns[0].sign.x, deSigns[0].sign.y), cvPoint(deSigns[0].sign.x + deSigns[0].sign.width - 1, deSigns[0].sign.y + deSigns[0].sign.height - 1), cvScalar(0, 0, 255, 0), 2, 1, 0);
					sprintf_s(putText, 100, "%f", deSigns[0].coarseResponse);
					cvText(m_pViewImg, putText, deSigns[0].sign.x, deSigns[0].sign.y, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
					sprintf_s(putText, 100, "%f", deSigns[0].fineResponse1);
					cvText(m_pViewImg, putText, deSigns[0].sign.x, deSigns[0].sign.y - 15, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
					sprintf_s(putText, 100, "%f", deSigns[0].fineResponse2);
					cvText(m_pViewImg, putText, deSigns[0].sign.x, deSigns[0].sign.y - 30, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
					sprintf_s(putText, 100, "%f", deSigns[0].fineResponse3);
					cvText(m_pViewImg, putText, deSigns[0].sign.x, deSigns[0].sign.y - 45, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
			}
			if (deSignsIndex > 1)
			{
				for (int i = 0; i < deSignsIndex; i++)
				{
					boundingBoxMerge(diffLoc, &diffLocIndex, &deSigns[i], i, user);//Bounding Box Merge
				}
				for (i = 0; i < diffLocIndex; i++)
				{
					cvRectangle(m_pViewImg, cvPoint(diffLoc[i].x, diffLoc[i].y), cvPoint(diffLoc[i].x + diffLoc[i].width - 1, diffLoc[i].y + diffLoc[i].height - 1), cvScalar(0, 0, 255, 0), 2, 1, 0);
					sprintf_s(putText, 100, "%f", diffLoc[i].coarseResponse);
					cvText(m_pViewImg, putText, diffLoc[i].x, diffLoc[i].y, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
					sprintf_s(putText, 100, "%f", diffLoc[i].fineResponse1);
					cvText(m_pViewImg, putText, diffLoc[i].x, diffLoc[i].y - 15, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
					sprintf_s(putText, 100, "%f", diffLoc[i].fineResponse2);
					cvText(m_pViewImg, putText, diffLoc[i].x, diffLoc[i].y - 30, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
					sprintf_s(putText, 100, "%f", diffLoc[i].fineResponse3);
					cvText(m_pViewImg, putText, diffLoc[i].x, diffLoc[i].y - 45, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);

				}
			}
			end = (int32_t)clock();
			Timecost_detection += (end - start);
/******************************End***********************************/
			sprintf_s(putText, 100, "Detection Time -> %f ms", (float32_t)Timecost_detection / ((float32_t)j + 1));
			cvText(m_pViewImg, putText, 0, 20, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);



			//Tracking
			/* Time used for Tracking and Recognition*/
/******************************Start***********************************/
			
			TrackingObject trackObject[250];
			int trackObjectIndex = 0;
			for (i = 0; i < diffLocIndex; i++)
			{
				trackObject[i].x = diffLoc[i].x;
				trackObject[i].y = diffLoc[i].y;
				trackObject[i].width = diffLoc[i].width;
				trackObject[i].height = diffLoc[i].height;
				trackObject[i].detection_fine1_response = diffLoc[i].fineResponse1;
				trackObject[i].detection_fine2_response = diffLoc[i].fineResponse2;
				trackObject[i].detection_fine3_response = diffLoc[i].fineResponse3;
				trackObject[i].detection_response = diffLoc[i].coarseResponse;
				trackObject[i].pyramidIndex = diffLoc[i].index_scale;
				trackObjectIndex++;
			}
			
  			signConfirm(trackObject, &trackObjectIndex, user, locCountTimes, &locCountBlockNum, locCountBlock, j);

			// If no sign detected in the detectObjects function
			TSR_Rect TrackingResult[10]; //Tracking Maximum 10 signs
			int TrackingNum = 0;
			for (i = 0; i < locCountBlockNum; i++)
			{
				if ((locCountTimes[i] == 0) || (locCountTimes[i] == 1))
				{
					continue;
				}
#if TSR_TRACKING_ENABLE
				if (locCountBlock[i][locCountTimes[i] - 1].signDetectFlag == 0) // if no sign detected in this CountBlock, tracking start
				{

					TrackingResult[TrackingNum] = Tracking(img, cascade, scaleNum, user, locCountTimes[i], &locCountBlockNum, i, locCountBlock, &TrackingResponse);

					if ((TrackingResult[TrackingNum].x != -1) && (TrackingResult[TrackingNum].y != -1))
					{
						//Plot Tracking Result
						cvRectangle(m_pViewImg, cvPoint(TrackingResult[TrackingNum].x, TrackingResult[TrackingNum].y), cvPoint(TrackingResult[TrackingNum].x + TrackingResult[TrackingNum].width - 1, TrackingResult[TrackingNum].y + TrackingResult[TrackingNum].height - 1), cvScalar(0, 0, 255, 0), 2, 1, 0);
						sprintf_s(putText, 100, "%f", TrackingResult[TrackingNum].coarseResponse);
						cvText(m_pViewImg, putText, TrackingResult[TrackingNum].x, TrackingResult[TrackingNum].y, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
						//sprintf_s(putText, 20, "%f", TrackingResult[TrackingNum].fineResponse1);
						//cvText(m_pViewImg, putText, TrackingResult[TrackingNum].x, TrackingResult[TrackingNum].y - 15, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
						//sprintf_s(putText, 20, "%f", TrackingResult[TrackingNum].fineResponse2);
						//cvText(m_pViewImg, putText, TrackingResult[TrackingNum].x, TrackingResult[TrackingNum].y - 30, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
						diffLoc[diffLocIndex] = TrackingResult[TrackingNum];
						diffLocIndex++;
						
						locCountTimes[i]++;
						locCountBlock[i][locCountTimes[i] - 1].pyramidIndex = TrackingResult[TrackingNum].index_scale;
						locCountBlock[i][locCountTimes[i] - 1].x = TrackingResult[TrackingNum].x;
						locCountBlock[i][locCountTimes[i] - 1].y = TrackingResult[TrackingNum].y;
						locCountBlock[i][locCountTimes[i] - 1].width = TrackingResult[TrackingNum].width;
						locCountBlock[i][locCountTimes[i] - 1].height = TrackingResult[TrackingNum].height;
						locCountBlock[i][locCountTimes[i] - 1].signDetectFlag = 1;
						locCountBlock[i][locCountTimes[i] - 1].lastFrames = 0;
						locCountBlock[i][locCountTimes[i] - 1].frame_num = j + 1;
						if (TrackingResult[TrackingNum].width > locCountBlock[i][locCountTimes[i] - 1].radius)
							locCountBlock[i][locCountTimes[i] - 1].radius = TrackingResult[TrackingNum].width;
						else
							locCountBlock[i][locCountTimes[i] - 1].radius = locCountBlock[i][locCountTimes[i] - 1].radius;
					}
					TrackingNum++;
				}

#endif
				if (locCountBlock[i][locCountTimes[i] - 1].lastFrames == 0) // do sign recognition and majority vote if sign detected
				{

					locCountBlock[i][locCountTimes[i] - 1].sign_class = HOG_SVM(locCountBlockNum, i, locCountTimes[i], locCountBlock, user, img, model, m_pViewImg, featureBuffer); // recognition using HOG + SVM
					locCountBlock[i][locCountTimes[i] - 1].sign_class_vote = majority_vote(locCountBlockNum, i, locCountTimes[i], locCountBlock);
					if (locCountBlock[i][locCountTimes[i] - 1].sign_class_vote == 1)
					{
						sprintf_s(putText, 100, "%s", "Speed Limit");
					}
					else if (locCountBlock[i][locCountTimes[i] - 1].sign_class_vote == 2)
					{
						sprintf_s(putText, 100, "%s", "No Turn On Red");
					}
					else if (locCountBlock[i][locCountTimes[i] - 1].sign_class_vote == 3)
					{
						sprintf_s(putText, 100, "%s", "Stop");
					}

					cvText(m_pViewImg, putText, locCountBlock[i][locCountTimes[i] - 1].x, locCountBlock[i][locCountTimes[i] - 1].y - 60, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
				}
			}

/******************************End***********************************/
			sprintf_s(putText, 100, "HOG calculation time -> %d ms", Timecost_HOG_calculation);
			cvText(m_pViewImg, putText, 0, 40, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
			sprintf_s(putText, 100, "SVM prediction time -> %d ms", Timecost_SVM_calculation);
			cvText(m_pViewImg, putText, 0, 60, cvScalar(0, 0, 255, 0), 0.5, 0.5, 2);
			//cvNamedWindow("program debug", 1);
			//cvShowImage("program debug", m_pViewImg);
			//cvWaitKey(0);
			/****************************************************************************************
			* Release memory and Images
			****************************************************************************************/
			cvWriteFrame(writer, m_pViewImg);
			cvReleaseImage(&m_pViewImg);
			cvReleaseImage(&m_pViewImg_1C);
			cascade->RectCount = 0;
	     }

		/****************************************************************************************
		* Display
		****************************************************************************************/
		printf("The video name is '%s'\n", begin_str);
		
		cvReleaseVideoWriter(&writer);
		if (_findnext(file, &find) != 0)
			break;
		BinNum++;
	}
	for (int i = 0; i < TSR_COUNT_BLOCK_SIZE; i++)
	{
		free(locCountBlock[i]);
	}
	free(locCountBlock);
	free(model);

	releaseTextClassifier(user);
	_findclose(file);
	return 0;
}