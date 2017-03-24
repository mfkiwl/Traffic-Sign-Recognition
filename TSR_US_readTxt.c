/**********************************************************************************************************

************************************************************************************************************/
#include "TSR_US_type.h"
#include "TSR_US_imageOperation.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int32_t * GwC_index_array_coarse;
float32_t * alpha_array_coarse;
float32_t ** weak_classifier_table_array_coarse;
uint8_t ** featureCoordinate900;
uint8_t ** featureCoordinate11025;
uint8_t ** featureCoordinate18225;
float32_t * rejectionThreshold;

int32_t * GwC_index_array_fine1;
float32_t * alpha_array_fine1;
float32_t ** weak_classifier_table_array_fine1;

int32_t * GwC_index_array_fine2;
float32_t * alpha_array_fine2;
float32_t ** weak_classifier_table_array_fine2;

int32_t * GwC_index_array_fine3;
float32_t * alpha_array_fine3;
float32_t ** weak_classifier_table_array_fine3;

void readTextClassifier(UserDefine * user)
{
	int8_t txtString1[30];
	int8_t txtString2[4096];
	int i;
	FILE * alphaFile;
	errno_t errFile;
	/* Loading soft cascade adaboost parameter 'aplha' */
	errFile = fopen_s(&alphaFile, user->coarse_alpha_file, "r");
	if (errFile != 0)
		printf("The file 'coarse_alpha.txt' was not opened\n");
	else
		printf("The file 'coarse_alpha.txt' was opened\n");

	alpha_array_coarse = (float32_t *)malloc(sizeof(float32_t)* user->coarseFeatNum);
	i = 0;
	while (fgets(txtString1, 30, alphaFile) != NULL)
	{
		alpha_array_coarse[i] = (float32_t)atof(txtString1);
		i++;
	}
	fclose(alphaFile);



	errFile = fopen_s(&alphaFile, user->fine_alpha_file, "r");
	if (errFile != 0)
		printf("The file 'fine1_alpha.txt' was not opened\n");
	else
		printf("The file 'fine1_alpha.txt' was opened\n");

	alpha_array_fine1 = (float32_t *)malloc(sizeof(float32_t)* user->fine1FeatNum);
	i = 0;
	while (fgets(txtString1, 30, alphaFile) != NULL)
	{
		alpha_array_fine1[i] = (float32_t)atof(txtString1);
		i++;
	}
	fclose(alphaFile);


	errFile = fopen_s(&alphaFile, "30-30 system//alpha391.txt", "r");
	if (errFile != 0)
		printf("The file 'fine2_alpha.txt' was not opened\n");
	else
		printf("The file 'fine2_alpha.txt' was opened\n");

	alpha_array_fine2 = (float32_t *)malloc(sizeof(float32_t)* user->fine2FeatNum);
	i = 0;
	while (fgets(txtString1, 30, alphaFile) != NULL)
	{
		alpha_array_fine2[i] = (float32_t)atof(txtString1);
		i++;
	}
	fclose(alphaFile);

	errFile = fopen_s(&alphaFile, "30-30 system//alpha542.txt", "r");
	if (errFile != 0)
		printf("The file 'fine3_alpha.txt' was not opened\n");
	else
		printf("The file 'fine3_alpha.txt' was opened\n");

	alpha_array_fine3 = (float32_t *)malloc(sizeof(float32_t)* user->fine3FeatNum);
	i = 0;
	while (fgets(txtString1, 30, alphaFile) != NULL)
	{
		alpha_array_fine3[i] = (float32_t)atof(txtString1);
		i++;
	}
	fclose(alphaFile);

	FILE * Gwc_indexFile;
	/* Loading soft cascade adaboost parameter 'Gwc' */
	errFile = fopen_s(&Gwc_indexFile, user->coarse_Gwc_index_file, "r");
	if (errFile != 0)
		printf("The file 'coarse_Gwc_index.txt' was not opened\n");
	else
		printf("The file 'coarse_Gwc_index.txt' was opened\n");

	GwC_index_array_coarse = (int32_t *)malloc(sizeof(int32_t)* user->coarseFeatNum);
	i = 0;
	while (fgets(txtString1, 30, Gwc_indexFile) != NULL)
	{
		GwC_index_array_coarse[i] = atoi(txtString1);
		i++;
	}
	fclose(Gwc_indexFile);


	errFile = fopen_s(&Gwc_indexFile, user->fine_Gwc_index_file, "r");
	if (errFile != 0)
		printf("The file 'fine1_Gwc_index.txt' was not opened\n");
	else
		printf("The file 'fine1_Gwc_index.txt' was opened\n");

	GwC_index_array_fine1 = (int32_t *)malloc(sizeof(int32_t)* user->fine1FeatNum);
	i = 0;
	while (fgets(txtString1, 30, Gwc_indexFile) != NULL)
	{
		GwC_index_array_fine1[i] = atoi(txtString1);
		i++;
	}
	fclose(Gwc_indexFile);


	errFile = fopen_s(&Gwc_indexFile, "30-30 system//Gwc_index391.txt", "r");
	if (errFile != 0)
		printf("The file 'fine2_Gwc_index.txt' was not opened\n");
	else
		printf("The file 'fine2_Gwc_index.txt' was opened\n");

	GwC_index_array_fine2 = (int32_t *)malloc(sizeof(int32_t)* user->fine2FeatNum);
	i = 0;
	while (fgets(txtString1, 30, Gwc_indexFile) != NULL)
	{
		GwC_index_array_fine2[i] = atoi(txtString1);
		i++;
	}
	fclose(Gwc_indexFile);

	errFile = fopen_s(&Gwc_indexFile, "30-30 system//Gwc_index542.txt", "r");
	if (errFile != 0)
		printf("The file 'fine3_Gwc_index.txt' was not opened\n");
	else
		printf("The file 'fine3_Gwc_index.txt' was opened\n");

	GwC_index_array_fine3 = (int32_t *)malloc(sizeof(int32_t)* user->fine3FeatNum);
	i = 0;
	while (fgets(txtString1, 30, Gwc_indexFile) != NULL)
	{
		GwC_index_array_fine3[i] = atoi(txtString1);
		i++;
	}
	fclose(Gwc_indexFile);

	FILE * rejectionThresholdFile;
	/* Loading soft cascade adaboost parameter 'rejectionThreshold' */
	errFile = fopen_s(&rejectionThresholdFile, user->coarse_reject_threshold_file, "r");
	if (errFile != 0)
		printf("The file 'coarse_RejectionThreshold.txt' was not opened\n");
	else
		printf("The file 'coarse_RejectionThreshold.txt' was opened\n");

	rejectionThreshold = (float32_t *)malloc(sizeof(float32_t)* user->coarseFeatNum);
	i = 0;
	while (fgets(txtString1, 30, rejectionThresholdFile) != NULL)
	{
		rejectionThreshold[i] = (float32_t)atof(txtString1);
		i++;
	}
	fclose(rejectionThresholdFile);





	FILE * weak_classifier_tableFile;
	/* Loading soft cascade adaboost parameter 'weak_classifier_table' */
	errFile = fopen_s(&weak_classifier_tableFile, user->coarse_weak_classifier_table_file, "r");
	if (errFile != 0)
		printf("The file 'coarse_weak_classifier_table.txt' was not opened\n");
	else
		printf("The file 'coarse_weak_classifier_table.txt' was opened\n");

	/* Arrange Space for 2 dimension array */
	char *tokenPtr;
	weak_classifier_table_array_coarse = (float32_t**)malloc(sizeof(float32_t *)* user->coarseFeatNum);
	for (int i = 0; i < user->coarseFeatNum; i++)
	{
		weak_classifier_table_array_coarse[i] = (float32_t *)malloc(sizeof(float32_t)* 256);
	}
	/* Read data from TXT */
	for (i = 0; i < user->coarseFeatNum; i++)
	{
		if (fgets(txtString2, 4096, weak_classifier_tableFile) != NULL)
		{
			tokenPtr = strtok(txtString2, " ");
			for (int j = 0; j < 256; j++)
			{
				weak_classifier_table_array_coarse[i][j] = (float32_t)atof(tokenPtr);
				tokenPtr = strtok(NULL, "");
				tokenPtr = strtok(tokenPtr, " ");
			}
		}
	}
	fclose(weak_classifier_tableFile);



	errFile = fopen_s(&weak_classifier_tableFile, user->fine_weak_classifier_table_file, "r");
	if (errFile != 0)
		printf("The file 'fine_weak_classifier_table.txt' was not opened\n");
	else
		printf("The file 'fine_weak_classifier_table.txt' was opened\n");

	/* Arrange Space for 2 dimension array */
	weak_classifier_table_array_fine1 = (float32_t**)malloc(sizeof(float32_t *)* user->fine1FeatNum);
	for (int i = 0; i < user->fine1FeatNum; i++)
	{
		weak_classifier_table_array_fine1[i] = (float32_t *)malloc(sizeof(float32_t)* 256);
	}
	/* Read data from TXT */
	for (i = 0; i < user->fine1FeatNum; i++)
	{
		if (fgets(txtString2, 4096, weak_classifier_tableFile) != NULL)
		{
			tokenPtr = strtok(txtString2, " ");
			for (int j = 0; j < 256; j++)
			{
				weak_classifier_table_array_fine1[i][j] = (float32_t)atof(tokenPtr);
				tokenPtr = strtok(NULL, "");
				tokenPtr = strtok(tokenPtr, " ");
			}
		}
	}
	fclose(weak_classifier_tableFile);


	errFile = fopen_s(&weak_classifier_tableFile, "30-30 system//weak_classifier_table391.txt", "r");
	if (errFile != 0)
		printf("The file 'fine2_weak_classifier_table.txt' was not opened\n");
	else
		printf("The file 'fine2_weak_classifier_table.txt' was opened\n");

	/* Arrange Space for 2 dimension array */
	weak_classifier_table_array_fine2 = (float32_t**)malloc(sizeof(float32_t *)* user->fine2FeatNum);
	for (int i = 0; i < user->fine2FeatNum; i++)
	{
		weak_classifier_table_array_fine2[i] = (float32_t *)malloc(sizeof(float32_t) * 256);
	}
	/* Read data from TXT */
	for (i = 0; i < user->fine2FeatNum; i++)
	{
		if (fgets(txtString2, 4096, weak_classifier_tableFile) != NULL)
		{
			tokenPtr = strtok(txtString2, " ");
			for (int j = 0; j < 256; j++)
			{
				weak_classifier_table_array_fine2[i][j] = (float32_t)atof(tokenPtr);
				tokenPtr = strtok(NULL, "");
				tokenPtr = strtok(tokenPtr, " ");
			}
		}
	}
	fclose(weak_classifier_tableFile);

	errFile = fopen_s(&weak_classifier_tableFile, "30-30 system//weak_classifier_table542.txt", "r");
	if (errFile != 0)
		printf("The file 'fine3_weak_classifier_table.txt' was not opened\n");
	else
		printf("The file 'fine3_weak_classifier_table.txt' was opened\n");

	/* Arrange Space for 2 dimension array */
	weak_classifier_table_array_fine3 = (float32_t**)malloc(sizeof(float32_t *)* user->fine3FeatNum);
	for (int i = 0; i < user->fine3FeatNum; i++)
	{
		weak_classifier_table_array_fine3[i] = (float32_t *)malloc(sizeof(float32_t) * 256);
	}
	/* Read data from TXT */
	for (i = 0; i < user->fine3FeatNum; i++)
	{
		if (fgets(txtString2, 4096, weak_classifier_tableFile) != NULL)
		{
			tokenPtr = strtok(txtString2, " ");
			for (int j = 0; j < 256; j++)
			{
				weak_classifier_table_array_fine3[i][j] = (float32_t)atof(tokenPtr);
				tokenPtr = strtok(NULL, "");
				tokenPtr = strtok(tokenPtr, " ");
			}
		}
	}
	fclose(weak_classifier_tableFile);

	FILE * featureCoordinateFile;
	int featureRows;
	/* Loading coordinate look up table feature coordinates */
	errFile = fopen_s(&featureCoordinateFile, "15-15 system//Feature Coordinates 900.txt", "r");
	if (errFile != 0)
		printf("The file 'Feature Coordinates.txt' was not opened\n");
	else
		printf("The file 'Feature Coordinates.txt' was opened\n");

	/* Arrange Space for 2 dimension array */
	if (fgets(txtString1, 30, featureCoordinateFile) != NULL)
	{
		featureRows = atoi(txtString1);
		user->coarseMnlbpRows = featureRows;
		featureCoordinate900 = (uint8_t **)malloc(sizeof(uint8_t *)* featureRows);
		for (int i = 0; i < featureRows; i++)
		{
			featureCoordinate900[i] = (uint8_t *)malloc(sizeof(uint8_t)* 4);
		}
	}
	/* Read data from TXT */
	for (i = 0; i < featureRows; i++)
	{
		if (fgets(txtString1, 30, featureCoordinateFile) != NULL)
		{
			tokenPtr = strtok(txtString1, " ");
			for (int j = 0; j < 4; j++)
			{
				featureCoordinate900[i][j] = (uint8_t)atoi(tokenPtr);
				tokenPtr = strtok(NULL, "");
				tokenPtr = strtok(tokenPtr, " ");
			}
		}
	}

	errFile = fopen_s(&featureCoordinateFile, "30-30 system//Feature Coordinates 11025.txt", "r");
	if (errFile != 0)
		printf("The file 'Feature Coordinates.txt' was not opened\n");
	else
		printf("The file 'Feature Coordinates.txt' was opened\n");

	/* Arrange Space for 2 dimension array */
	if (fgets(txtString1, 30, featureCoordinateFile) != NULL)
	{
		featureRows = atoi(txtString1);
		user->fineMnlbpRows1 = featureRows;
		featureCoordinate11025 = (uint8_t **)malloc(sizeof(uint8_t *)* featureRows);
		for (int i = 0; i < featureRows; i++)
		{
			featureCoordinate11025[i] = (uint8_t *)malloc(sizeof(uint8_t)* 4);
		}
	}
	/* Read data from TXT */
	for (i = 0; i < featureRows; i++)
	{
		if (fgets(txtString1, 30, featureCoordinateFile) != NULL)
		{
			tokenPtr = strtok(txtString1, " ");
			for (int j = 0; j < 4; j++)
			{
				featureCoordinate11025[i][j] = (uint8_t)atoi(tokenPtr);
				tokenPtr = strtok(NULL, "");
				tokenPtr = strtok(tokenPtr, " ");
			}
		}
	}

	errFile = fopen_s(&featureCoordinateFile, "30-30 system//Feature Coordinates 18225.txt", "r");
	if (errFile != 0)
		printf("The file 'Feature Coordinates.txt' was not opened\n");
	else
		printf("The file 'Feature Coordinates.txt' was opened\n");

	/* Arrange Space for 2 dimension array */
	if (fgets(txtString1, 30, featureCoordinateFile) != NULL)
	{
		featureRows = atoi(txtString1);
		user->fineMnlbpRows2 = featureRows;
		featureCoordinate18225 = (uint8_t **)malloc(sizeof(uint8_t *)* featureRows);
		for (int i = 0; i < featureRows; i++)
		{
			featureCoordinate18225[i] = (uint8_t *)malloc(sizeof(uint8_t) * 4);
		}
	}
	/* Read data from TXT */
	for (i = 0; i < featureRows; i++)
	{
		if (fgets(txtString1, 30, featureCoordinateFile) != NULL)
		{
			tokenPtr = strtok(txtString1, " ");
			for (int j = 0; j < 4; j++)
			{
				featureCoordinate18225[i][j] = (uint8_t)atoi(tokenPtr);
				tokenPtr = strtok(NULL, "");
				tokenPtr = strtok(tokenPtr, " ");
			}
		}
	}
}




/*******************************************************************************************************
** NAME: releaseTextClassifier
** CALLED BY: main
** PARAMETER: none
** RETURN VALUE: none
** DESCRIPTION: This function free the memory space that used to save all parameters used to build traffic
** sign detector.
*********************************************************************************************************/
void releaseTextClassifier(UserDefine * user)
{
	free(GwC_index_array_coarse);
	free(GwC_index_array_fine1);
	free(GwC_index_array_fine2);
	free(GwC_index_array_fine3);

	free(alpha_array_coarse);
	free(alpha_array_fine1);
	free(alpha_array_fine2);
	free(alpha_array_fine3);

	for (int i = 0; i < user->coarseFeatNum; i++)
	{
		free(weak_classifier_table_array_coarse[i]);
	}
	for (int i = 0; i < user->fine1FeatNum; i++)
	{
		free(weak_classifier_table_array_fine1[i]);
	}
	for (int i = 0; i < user->fine2FeatNum; i++)
	{
		free(weak_classifier_table_array_fine2[i]);
	}
	for (int i = 0; i < user->fine3FeatNum; i++)
	{
		free(weak_classifier_table_array_fine3[i]);
	}

	for (int i = 0; i < user->coarseMnlbpRows; i++)
	{
		free(featureCoordinate900[i]);
	}
	for (int i = 0; i < user->fineMnlbpRows1; i++)
	{
		free(featureCoordinate11025[i]);
	}
	for (int i = 0; i < user->fineMnlbpRows2; i++)
	{
		free(featureCoordinate18225[i]);
	}

}