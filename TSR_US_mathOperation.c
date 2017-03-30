/****************************************************************************************
| Project Name: TSR
| Hardware: .
| Description:  US traffic sign detection algorithm
| File Name: TSR_US_functions.c
|
|----------------------------------------------------------------------------------------
| C O P Y R I G H T
|----------------------------------------------------------------------------------------
|----------------------------------------------------------------------------------------
| A U T H O R I D E N T I T Y
|----------------------------------------------------------------------------------------
| Initials  Name                 Contact
| -------- --------------------- ----------------------
| T.Wang   Tianyu WANG          tianyuw@icloud.com
|
|----------------------------------------------------------------------------------------
| R E V I S I O N H I S T O R Y
|----------------------------------------------------------------------------------------
| Date        Version Author Description
| ----------  ------- ------ --------------------------------------------------------------
| 2015-03-09  0.01.00 T.Wang Creation

|
|***************************************************************************************/

/****************************************************************************************
* Include files
****************************************************************************************/
#include "TSR_US_type.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

/****************************
*OpenCV header
*****************************/
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "ml.h"







/****************************************************************************************
** NAME: myRound
** CALLED BY: main detectObjects, MYScaleImage_Invoker, MYScaleImage_Invoker_skip
** PARAMETER: float32_t value
** RETURN VALUE: int32_t
** DESCRIPTION: Input is an float type of number, and return the integer that most close to
** that float number.
** Input:
** "float32_t value" is a float type number.
** Output:
** "int32_t" is the integer that most close to that float type number.
****************************************************************************************/
int32_t  TSR_round(float32_t value)
{
	return (int32_t)(value + (value >= 0 ? 0.5 : -0.5));
}

/*******************************************************************************************************
** NAME: Area
** CALLED BY: System1_validation, System2_validation, System3_validation
** PARAMETER: int32_t * A, int32_t * B, int32_t element1, int32_t element2, int32_t C
** RETURN VALUE: float32_t
** DESCRIPTION: This function utilize the integral image to calculate the area of a specific rectangle.
** you can find more information: http://en.wikipedia.org/wiki/Summed_area_table
** Input:
** "int32_t * A" is the pointer to the starting address of one row in integral image.
** "int32_t * B" is the pointer to the starting address of one row in integral image.
** "int32_t * element1" is the colume index.
** "int32_t * element2" is the colume index.
** "int32_t C" is the number of small blocks inside the LBP rectangle.
** Output:
** "float32_t" is the area calculated through integral image.
*********************************************************************************************************/
float32_t Area(int32_t * A, int32_t * B, int32_t element1, int32_t element2, int32_t C)
{
	float32_t area = (float32_t)(A[element1] - A[element2] - B[element1] + B[element2]) / C;
	return area;
}

/*******************************************************************************************************
** NAME: VectorSumFloat
** CALLED BY: System1_validation, System2_validation
** PARAMETER: float32_t * a, int32_t b
** RETURN VALUE: float32_t
** DESCRIPTION: This function calculates the summation of a float32_t type of array, and return that
** summation.
** Input:
** "float32_t * a" is the pointer to the start address of a float type array.
** "int32_t b" indicates how many element in the array.
** Output:
** "float32_t" is the summation of the elements in array.
*********************************************************************************************************/
float32_t VectorSumFloat(float32_t * a, int32_t b)
{
	float32_t sum = 0;

	for (int32_t it = 0; it < b; it++)
		sum = sum + a[it];
	return sum;
}
