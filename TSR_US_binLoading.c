#include "TSR_US_type.h"
#include "stdio.h"



/****************************
*OpenCV header
*****************************/
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "ml.h"


/****************************************************************************************
* Defines
****************************************************************************************/
#define CAPTURE_IN_W     (int32_t)(1280)
#define CAPTURE_IN_H     (int32_t)(672)
#define IMAGE_FILE_SIZE  (CAPTURE_IN_W * CAPTURE_IN_H * 2)
#define CAPTURE_YUV_BUFF_DUMP_SIZE              ((CAPTURE_IN_W        * (CAPTURE_IN_H       + 8)) *2)
#define tiny  1e-7

/****************************************************************************************
* Type definitions
****************************************************************************************/
typedef uint8_t    BYTE;

BYTE m_pInputYUV[CAPTURE_YUV_BUFF_DUMP_SIZE];
bool_t m_bFrameChangeFlag = 0;
unsigned long long int m_nFileLength, m_nFileCurpos;
FILE * m_pVideoFp;
IplImage * m_pViewImg, *m_pViewImg_1C;

/****************************************************************************************
** NAME: LoadVideo
** CALLED BY: main
** PARAMETER: const char*  strFilePath
** RETURN VALUE: none
** DESCRIPTION: This function open an "BIN" file for read.
** Input:
** "const int8_t * strFilePath" is the path of the Bin file to be read.
****************************************************************************************/
void LoadVideo(const char * strFilePath)
{

	errno_t err;
	err = fopen_s(&m_pVideoFp, strFilePath, "rb");
	if (m_pVideoFp == NULL)

	{
		printf("Can not open Bin file for read.");
		exit(1);
	}
	_fseeki64(m_pVideoFp, 0, SEEK_END);
	m_nFileLength = _ftelli64(m_pVideoFp);
	m_nFileLength = m_nFileLength / IMAGE_FILE_SIZE;
	_fseeki64(m_pVideoFp, 0, SEEK_SET);
	m_nFileCurpos = 0;

}

/****************************************************************************************
** NAME: MakeFrame
** CALLED BY: main
** PARAMETER: IplImage* view_image, unsigned short* buf
** RETURN VALUE: none
** DESCRIPTION: This function Convert Bin file data to an RGB image, and save the image into
** OpenCV format "IplImage".
** Input:
** "IplImage * view_image" is the OpenCV image. The Bin file is converted to images before
** detection procedure comes in.
** "uint16_t * buf" is the buffer that used to save bin file data, and then convert to images.
****************************************************************************************/
void MakeFrame(IplImage * view_image, uint16_t * buf)
{
	int32_t R, G, B, u, v;
	BYTE Y1, Y2, U, V;
	for (v = 0; v < CAPTURE_IN_H; v = v + 1) {
		for (u = 0; u < CAPTURE_IN_W; u = u + 2) {
			U = (BYTE)(buf[(v * CAPTURE_IN_W + u)] & 0xFF);
			Y1 = (BYTE)(buf[(v * CAPTURE_IN_W + u)] >> 8);
			V = (BYTE)(buf[(v * CAPTURE_IN_W + u + 1)] & 0xFF);
			Y2 = (BYTE)(buf[(v * CAPTURE_IN_W + u + 1)] >> 8);

			R = ((512 * Y1) / 128 + (718 * (V - 128)) / 128) / 4;
			G = ((512 * Y1) / 128 + (-176 * (U - 128)) / 128 + (-366 * (V - 128)) / 128) / 4;
			B = ((512 * Y1) / 128 + (907 * (U - 128)) / 128) / 4;

			R = (R > 255) ? 255 : R;
			R = (R < 0) ? 0 : R;
			G = (G > 255) ? 255 : G;
			G = (G < 0) ? 0 : G;
			B = (B > 255) ? 255 : B;
			B = (B < 0) ? 0 : B;

			view_image->imageData[v * CAPTURE_IN_W * 3 + u * 3 + 0] = (BYTE)B;
			view_image->imageData[v * CAPTURE_IN_W * 3 + u * 3 + 1] = (BYTE)G;
			view_image->imageData[v * CAPTURE_IN_W * 3 + u * 3 + 2] = (BYTE)R;

			R = ((512 * Y2) / 128 + (718 * (V - 128)) / 128) / 4;
			G = ((512 * Y2) / 128 + (-176 * (U - 128)) / 128 + (-366 * (V - 128)) / 128) / 4;
			B = ((512 * Y2) / 128 + (907 * (U - 128)) / 128) / 4;

			R = (R > 255) ? 255 : R;
			R = (R < 0) ? 0 : R;
			G = (G > 255) ? 255 : G;
			G = (G < 0) ? 0 : G;
			B = (B > 255) ? 255 : B;
			B = (B < 0) ? 0 : B;

			view_image->imageData[v * CAPTURE_IN_W * 3 + (u + 1) * 3 + 0] = (BYTE)B;
			view_image->imageData[v * CAPTURE_IN_W * 3 + (u + 1) * 3 + 1] = (BYTE)G;
			view_image->imageData[v * CAPTURE_IN_W * 3 + (u + 1) * 3 + 2] = (BYTE)R;

		}
	}
}

/****************************************************************************************
** NAME: Callback_Func
** CALLED BY: main
** PARAMETER: none
** RETURN VALUE: none
** DESCRIPTION: This function arrange the memory space for IplImage type of image and locate
** the first data location in memory for the next RGB image to be read.
****************************************************************************************/
void Callback_Func()
{
	uint16_t * buf;
	buf = (uint16_t *)m_pInputYUV;
	m_pViewImg = cvCreateImage(cvSize(CAPTURE_IN_W, CAPTURE_IN_H), 8, 3);
	m_pViewImg_1C = cvCreateImage(cvSize(CAPTURE_IN_W, CAPTURE_IN_H), 8, 1);

	if (m_bFrameChangeFlag != 0){
		unsigned long long int temp_index = 0;
		temp_index = m_nFileCurpos * IMAGE_FILE_SIZE;
		_fseeki64(m_pVideoFp, temp_index, SEEK_SET);
		m_bFrameChangeFlag = 0;
	}

	uint32_t len = (uint32_t)fread(buf, 1, IMAGE_FILE_SIZE, m_pVideoFp);
	if (len != IMAGE_FILE_SIZE){
		return;
	}
	m_nFileCurpos = m_nFileCurpos + 1;

	MakeFrame(m_pViewImg, buf);
	cvCvtColor(m_pViewImg, m_pViewImg_1C, CV_BGR2GRAY);
}
