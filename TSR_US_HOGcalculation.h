#ifndef __TSR_US_HOGcalculation_H___
#define __TSR_US_HOGcalculation_H___

#include "TSR_US_imageOperation.h"

#define RESIZE_WIDTH 48
#define RESIZE_HEIGHT 64
#define BLOCK_SIZE 2
#define CELL_SIZE 8
#define DIR 9
#define PI 3.14159265359
#define BLOCK_NUM 35
//(((RESIZE_HEIGHT - (BLOCK_SIZE * CELL_SIZE)) / CELL_SIZE) + 1) * (((RESIZE_WIDTH - (BLOCK_SIZE * CELL_SIZE)) / CELL_SIZE) + 1)


typedef struct
{
	double * pixels;
} channel;

TSR_Image * subWindow;
channel channels[3];
int imageSize;

typedef struct
{

	double magnitude;
	double orient;
	int rgb_channel;

	
}gradient;

gradient * gradients;

typedef struct 
{
	double votes[BLOCK_SIZE*BLOCK_SIZE*DIR];

}hogOfBlock;

hogOfBlock * blocks;
int block;








double deviation(double left, double right, double top, double bottom, double* angle);
void HOGcalculation(TSR_Image * img, char* buffer); //System trained with Grayscale Image
void gammaCompression();
void calGradients(int height, int width);
//void printSilhouette();
void calHogs(int width, int height, char * buffer);
void calBlockHogs(int row, int col, int width, double *votes, double total);
//void printOutHog();




#endif