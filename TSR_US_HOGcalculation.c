
#include "TSR_US_HOGcalculation.h"
#include "TSR_US_imageOperation.h"
/****************************
*OpenCV header
*****************************/
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "ml.h"

const double posx[2] = { 4, 12 };
const double posy[2] = { 4, 12 };
const double dirs[DIR] = { PI / 18, 3 * PI / 18, 5 * PI / 18, 7 * PI / 18, 9 * PI / 18, 11 * PI / 18, 13 * PI / 18, 15 * PI / 18, 17 * PI / 18 };

void HOGcalculation(TSR_Image * img, char* buffer) //System trained with Grayscale Image
{

	subWindow = img;
	imageSize = subWindow->width * subWindow->height;
	for (int channelIndex = 0; channelIndex < 3; channelIndex++)
		channels[channelIndex].pixels = (double * )malloc(sizeof(double) * imageSize);//Generate memory space for Image Pixels!! Need free

		gradients = (gradient * )malloc((sizeof(gradient)) * imageSize);//Generate memory space for Image Gradients!! Need free

		blocks = (hogOfBlock * ) malloc((sizeof(hogOfBlock)) * (BLOCK_NUM));
	for (int i = 0; i < imageSize; i++) // Convert the Image to Double Image.
	{
		for (int k = 0; k < 3; k++)
		{
			double temp = subWindow->data[i];
			if (temp < 0) temp += 256;
			channels[k].pixels[i] = temp;
		}
	}
	gammaCompression();
	calGradients(subWindow->height, subWindow->width);
	//printSilhouette();
	calHogs(subWindow->width, subWindow->height, buffer);
	

	for (int channelIndex = 0; channelIndex < 3; channelIndex++)
		free(channels[channelIndex].pixels);//Generate memory space for Image Pixels!! Need free
	free(gradients);
	//free(blocks);
}


void gammaCompression()
{
	int length = imageSize;
	for (int i = 0; i < length; i++)
	{
		for (int j = 0; j < 3; j++)
			channels[j].pixels[i] = sqrt(channels[j].pixels[i]);
	}
}

void calGradients(int height, int width)
{

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			double temp, left, right, top, bottom, angle;
			gradient point = { 0, 0, 0};
			
			for (int k = 0; k < 3; k++)
			{
				left = (j == 0 ? -1 : channels[k].pixels[i*width + j - 1]);
				right = (j == width - 1 ? -1 : channels[k].pixels[i*width + j + 1]);
				top = (i == 0 ? -1 : channels[k].pixels[(i - 1)*width + j]);
				bottom = (i == height - 1 ? -1 : channels[k].pixels[(i + 1)*width + j]);
				temp = deviation(left, right, top, bottom, &angle);
				if (point.magnitude < temp)
				{
					point.magnitude = temp;
					point.orient = angle;
					point.rgb_channel = k;
				}
			}
			gradients[i * width + j] = point;
			/*printf("%f ", point.magnitude);*/
		}
	}
}

double deviation(double left, double right, double top, double bottom, double* angle)
{
	if (left == -1)
	{
		if (right == -1)
		{
			*angle = 0;
			return 0;
		}
		else
			left = right;
	}
	else
	{
		if (right == -1)
			right = left;
	}

	if (top == -1)
	{
		if (bottom == -1)
		{
			*angle = 0;
			return 0;
		}
		else
			top = bottom;
	}
	else
	{
		if (bottom == -1)
			bottom = top;
	}

	double var = sqrt((right - left) * (right - left) + (top - bottom) * (top - bottom));
	if (var == 0)
	{
		*angle = 0;
		return 0;
	}
	double sin_dir = (top - bottom) / var;
	double cos_dir = (right - left) / var;

	//pedestrain 0~pi
	*angle = acos(cos_dir);
	return var;

}

//void printSilhouette() // Show the Gradient Image using OpenCV
//{
//	double max = -1;
//	int length = imageSize;
//	for (int i = 0; i < length; i++)
//	{
//		if (max < gradients[i].magnitude)
//			max = gradients[i].magnitude;
//	}
//	
//	
//	IplImage * ptr = cvCreateImage(cvSize(48, 64), 8, 1);
//
//	for (int i = 0; i < length; i++)
//	{
//		double temp = gradients[i].magnitude / max * 255;
//		int temp_pixel = (int)temp;
//		if (temp > 127)
//			temp -= 256;
//		ptr->imageData[i] = (char)temp_pixel;
//	}
//
//
//
//	cvNamedWindow("silhouette", 0);
//	cvShowImage("silhouette", ptr);
//
//	cvWaitKey(0);
//
//	cvDestroyWindow("silhouette");
//	cvReleaseImage(&ptr);
//}


void calHogs(int width, int height, char * buffer)
{
	block = 0;
	hogOfBlock temp;
	//i, j as row, col of the top-left corner of the block
	for (int i = 0; i <= height - CELL_SIZE * BLOCK_SIZE; i += CELL_SIZE)
	{
		for (int j = 0; j <= width - CELL_SIZE * BLOCK_SIZE; j += CELL_SIZE)
		{
			block++;
			
			//memset(temp.votes, 0, sizeof(hogOfBlock));
			memset(temp.votes, 0, sizeof(double) * BLOCK_SIZE*BLOCK_SIZE*DIR);
			double total = 0;
			for (int a = 0; a < BLOCK_SIZE * CELL_SIZE; a++)
			{
				for (int b = 0; b < BLOCK_SIZE * CELL_SIZE; b++)
				{
					total += gradients[(i + a)*width + j + b].magnitude;
				}
			}
			if (total == 0)
				total = 0.001;
			calBlockHogs(i, j, width, temp.votes, total);
			blocks[block - 1] = temp;
		}
	}
	char out[512];

	for (int i = 0; i < BLOCK_NUM; i++)
	{
		for (int j = 0; j < BLOCK_SIZE*BLOCK_SIZE*DIR; j++)
		{
			sprintf(out, "%d:%f ", i * BLOCK_SIZE*BLOCK_SIZE*DIR + j + 1, blocks[i].votes[j]);
			strcat(buffer, out);
		}
	}
	free(blocks);
	
}


void calBlockHogs(int row, int col, int width, double *votes, double total)
{

	int index_x, index_y, index_z;
	for (int i = 0; i < BLOCK_SIZE * CELL_SIZE; i++)
	{
		for (index_x = 0; index_x < 2; index_x++)
		{
			if ((i - posx[index_x]) < 0)
				break;
		}
		//vector<gradient>::iterator ii = (gradients.begin() + (row + i) * width + col);
		for (int j = 0; j < BLOCK_SIZE * CELL_SIZE; j++)
		{
			gradient temp = gradients[(row + i) * width + col + j];
			for (index_y = 0; index_y < 2; index_y++)
			{
				if ((j - posy[index_y]) < 0)
					break;
			}
			for (index_z = 0; index_z < 9; index_z++)
			{
				if ((temp.orient - dirs[index_z]) < 0)
					break;
			}
			if (index_x == 0)
			{
				if (index_y == 0)
				{
					if (index_z == 0)
					{
						votes[0] += temp.magnitude;
					}
					else if (index_z == 9)
					{
						votes[8] += temp.magnitude;
					}
					else
					{
						votes[index_z - 1] += temp.magnitude * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[index_z] += temp.magnitude * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
					}
				}
				else if (index_y == 1)
				{
					if (index_z == 0)
					{
						votes[0] += temp.magnitude * (1 - (j - posy[0]) / 8);
						votes[DIR] += temp.magnitude *((j - posy[0]) / 8);
					}
					else if (index_z == 9)
					{
						votes[8] += temp.magnitude * (1 - (j - posy[0]) / 8);
						votes[DIR + 9] += temp.magnitude * ((j - posy[0]) / 8);
					}
					else
					{
						votes[index_z - 1] += temp.magnitude * (1 - (j - posy[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[index_z] += temp.magnitude * (1 - (j - posy[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[DIR + index_z - 1] += temp.magnitude * ((j - posy[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[DIR + index_z] += temp.magnitude * ((j - posy[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
					}
				}
				else
				{
					if (index_z == 0)
					{
						votes[DIR] += temp.magnitude;
					}
					else if (index_z == 9)
					{
						votes[DIR + 8] += temp.magnitude;
					}
					else
					{
						votes[DIR + index_z - 1] += temp.magnitude * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[DIR + index_z] += temp.magnitude * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
					}
				}
			}
			else if (index_x == 1)
			{
				if (index_y == 0)
				{
					if (index_z == 0)
					{
						votes[0] += temp.magnitude * (1 - (i - posx[0]) / 8);
						votes[BLOCK_SIZE*DIR] += temp.magnitude * ((i - posx[0]) / 8);
					}
					else if (index_z == 9)
					{
						votes[8] += temp.magnitude * (1 - (i - posx[0]) / 8);
						votes[BLOCK_SIZE*DIR + 8] += temp.magnitude * ((i - posx[0]) / 8);
					}
					else
					{
						votes[index_z - 1] += temp.magnitude * (1 - (i - posx[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[index_z] += temp.magnitude * (1 - (i - posx[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + index_z - 1] += temp.magnitude * ((i - posx[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + index_z] += temp.magnitude * ((i - posx[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
					}
				}
				else if (index_y == 1)
				{
					if (index_z == 0)
					{
						votes[0] += temp.magnitude * (1 - (i - posx[0]) / 8) * (1 - (j - posy[0]) / 8);
						votes[DIR] += temp.magnitude * (1 - (i - posx[0]) / 8) * ((j - posy[0]) / 8);
						votes[BLOCK_SIZE*DIR] += temp.magnitude * ((i - posx[0]) / 8) * (1 - (j - posy[0]) / 8);
						votes[BLOCK_SIZE*DIR + DIR] += temp.magnitude * ((i - posx[0]) / 8) * ((j - posy[0]) / 8);
					}
					else if (index_z == 9)
					{
						votes[8] += temp.magnitude * (1 - (i - posx[0]) / 8) * (1 - (j - posy[0]) / 8);
						votes[DIR + 8] += temp.magnitude * (1 - (i - posx[0]) / 8) * ((j - posy[0]) / 8);
						votes[BLOCK_SIZE*DIR + 8] += temp.magnitude * ((i - posx[0]) / 8) * (1 - (j - posy[0]) / 8);
						votes[BLOCK_SIZE*DIR + DIR + 8] += temp.magnitude * ((i - posx[0]) / 8) * ((j - posy[0]) / 8);
					}
					else
					{
						votes[index_z - 1] += temp.magnitude * (1 - (i - posx[0]) / 8) * (1 - (j - posy[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[index_z] += temp.magnitude * (1 - (i - posx[0]) / 8) * (1 - (j - posy[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[DIR + index_z - 1] += temp.magnitude * (1 - (i - posx[0]) / 8) * ((j - posy[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[DIR + index_z] += temp.magnitude * (1 - (i - posx[0]) / 8) * ((j - posy[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + index_z - 1] += temp.magnitude * ((i - posx[0]) / 8) * (1 - (j - posy[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + index_z] += temp.magnitude * ((i - posx[0]) / 8) * (1 - (j - posy[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + DIR + index_z - 1] += temp.magnitude * ((i - posx[0]) / 8) * ((j - posy[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + DIR + index_z] += temp.magnitude * ((i - posx[0]) / 8) * ((j - posy[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
					}
				}
				else
				{
					if (index_z == 0)
					{
						votes[DIR] += temp.magnitude * (1 - (i - posx[0]) / 8);
						votes[BLOCK_SIZE*DIR + DIR] += temp.magnitude * ((i - posx[0]) / 8);
					}
					else if (index_z == 9)
					{
						votes[8 + DIR] += temp.magnitude * (1 - (i - posx[0]) / 8);
						votes[BLOCK_SIZE*DIR + DIR + 8] += temp.magnitude * ((i - posx[0]) / 8);
					}
					else
					{
						votes[DIR + index_z - 1] += temp.magnitude * (1 - (i - posx[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[DIR + index_z] += temp.magnitude * (1 - (i - posx[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + DIR + index_z - 1] += temp.magnitude * ((i - posx[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + DIR + index_z] += temp.magnitude * ((i - posx[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
					}
				}
			}
			else
			{
				if (index_y == 0)
				{
					if (index_z == 0)
					{
						votes[BLOCK_SIZE*DIR] += temp.magnitude;
					}
					else if (index_z == 9)
					{
						votes[BLOCK_SIZE*DIR + 8] += temp.magnitude;
					}
					else
					{
						votes[BLOCK_SIZE*DIR + index_z - 1] += temp.magnitude * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + index_z] += temp.magnitude * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
					}
				}
				else if (index_y == 1)
				{
					if (index_z == 0)
					{
						votes[BLOCK_SIZE*DIR] += temp.magnitude * (1 - (j - posy[0]) / 8);
						votes[BLOCK_SIZE*DIR + DIR] += temp.magnitude *((j - posy[0]) / 8);
					}
					else if (index_z == 9)
					{
						votes[BLOCK_SIZE*DIR + 8] += temp.magnitude * (1 - (j - posy[0]) / 8);
						votes[BLOCK_SIZE*DIR + DIR + 8] += temp.magnitude * ((j - posy[0]) / 8);
					}
					else
					{
						votes[BLOCK_SIZE*DIR + index_z - 1] += temp.magnitude * (1 - (j - posy[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + index_z] += temp.magnitude * (1 - (j - posy[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + DIR + index_z - 1] += temp.magnitude * ((j - posy[0]) / 8) * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + DIR + index_z] += temp.magnitude * ((j - posy[0]) / 8) * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
					}
				}
				else
				{
					if (index_z == 0)
					{
						votes[BLOCK_SIZE*DIR + DIR] += temp.magnitude;
					}
					else if (index_z == 9)
					{
						votes[BLOCK_SIZE*DIR + DIR + 9] += temp.magnitude;
					}
					else
					{
						votes[BLOCK_SIZE*DIR + DIR + index_z - 1] += temp.magnitude * (1 - (temp.orient - dirs[index_z - 1]) * 9 / PI);
						votes[BLOCK_SIZE*DIR + DIR + index_z] += temp.magnitude * ((temp.orient - dirs[index_z - 1]) * 9 / PI);
					}
				}
			}
		}
	}
	int i = 0;
	for (i = 0; i < BLOCK_SIZE*BLOCK_SIZE*DIR; i++)
	{
		votes[i] /= total;
	}
}

//void printOutHog(char * buffer)
//{
//	char out[512];
//
//	for (int i = 0; i < BLOCK_NUM; i++)
//	{
//		for (int j = 0; j < BLOCK_SIZE*BLOCK_SIZE*DIR; j++)
//		{
//			sprintf(out, "%d:%f ", i * BLOCK_SIZE*BLOCK_SIZE*DIR + j + 1, blocks[i].votes[j]);
//			strcat(buffer, out);
//		}
//	}
//
//
//}