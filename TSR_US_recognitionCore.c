#include "stdio.h"
#include "TSR_US_recognitionCore.h"
#include "locale.h"
#include "string.h"
#include "stdlib.h"
#include "ctype.h"
#include "math.h"
#include "TSR_US_imageOperation.h"
#include "TSR_US_tracking.h"
#include "TSR_US_HOGcalculation.h"
#include "time.h"

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

static char *line = NULL;
static int max_line_len;

struct svm_node *x;

int max_nr_attr = 64;

extern UserDefine * user;
extern int32_t start, end, Timecost_HOG_calculation, Timecost_SVM_calculation;

int HOG_SVM(int locCountBlockNum, int i, int locCountTimes, TSR_CountBlock ** locCountBlock, UserDefine * user, TSR_Image * img,
	 svm_model * model, IplImage * m_pViewImg, char * featureBuffer)
{
	TSR_Image _subImage;
	TSR_Image * subImage = &_subImage;
	TSR_Image _subResizeImage;
	TSR_Image * subResizeImage = &_subResizeImage;
	TSR_Point _startPoint;
	TSR_Point * startPoint = &_startPoint;

	int predict_label = 0;
	//char putText[20];

	startPoint->x = locCountBlock[i][locCountTimes - 1].x;
	startPoint->y = locCountBlock[i][locCountTimes - 1].y;
	createImage(locCountBlock[i][locCountTimes - 1].width, locCountBlock[i][locCountTimes - 1].height, subImage);
	createImage(user->recognitionSize.width, user->recognitionSize.height, subResizeImage);
	subImageCrop(img, subImage, startPoint);
	nearestNeighbor(subImage, subResizeImage);
	freeImage(subImage);
	start = (int32_t)clock();
	HOGcalculation(subResizeImage, featureBuffer);
	end = (int32_t)clock();
	Timecost_HOG_calculation += end - start;
	freeImage(subResizeImage);
	start = (int32_t)clock();
	predict_label = (int)predict(featureBuffer, model);
	end = (int32_t)clock();
	Timecost_SVM_calculation += end - start;
	return predict_label;
}


int majority_vote(int locCountBlockNum, int i, int locCountTimes, TSR_CountBlock ** locCountBlock)
{
	int vote[3] = {0}; // Count how many times of each signs 1.speed limit 2.no turn on red 3.stop
	int vote_sign_class;
	for (int j = 0; j < locCountTimes; j++)
	{
		if (locCountBlock[i][j].sign_class == 1)
		{
			vote[0]++;
		}
		if (locCountBlock[i][j].sign_class == 2)
		{
			vote[1]++;
		}
		if (locCountBlock[i][j].sign_class == 3)
		{
			vote[2]++;
		}
	}
	vote_sign_class = array_max(vote, 3);

	return vote_sign_class + 1;
}

int array_max(int * vote, int num)
{
	int max_value, max_index;
	max_value = vote[0];
	max_index = 0;
	for (int i = 1; i < num; i++)
	{
		if (vote[i] > max_value)
		{
			max_value = vote[i];
			max_index = i;
		}

	}
	return max_index;
}



static double powi(double base, int times)
{
	double tmp = base, ret = 1.0;

	for (int t = times; t>0; t /= 2)
	{
		if (t % 2 == 1) ret *= tmp;
		tmp = tmp * tmp;
	}
	return ret;
}


svm_model *svm_load_model(const char *model_file_name)
{
	FILE *fp = fopen(model_file_name, "rb");
	if (fp == NULL) return NULL;

	char *old_locale = _strdup(setlocale(LC_ALL, NULL));
	setlocale(LC_ALL, "C");

	// read parameters

	svm_model *model = malloc(1 * sizeof(svm_model));
	model->rho = NULL;
	model->probA = NULL;
	model->probB = NULL;
	model->sv_indices = NULL;
	model->label = NULL;
	model->nSV = NULL;

	// read header
	if (!read_model_header(fp, model))
	{
		fprintf(stderr, "ERROR: fscanf failed to read model\n");
		setlocale(LC_ALL, old_locale);
		free(old_locale);
		free(model->rho);
		free(model->label);
		free(model->nSV);
		free(model);
		return NULL;
	}

	// read sv_coef and SV

	int elements = 0;
	long pos = ftell(fp);

	max_line_len = 1024;
	line = Malloc(char, max_line_len);
	char *p, *endptr, *idx, *val;

	while (readline(fp) != NULL)
	{
		p = strtok(line, ":");
		while (1)
		{
			p = strtok(NULL, ":");
			if (p == NULL)
				break;
			++elements;
		}
	}
	elements += model->l;

	fseek(fp, pos, SEEK_SET);

	int m = model->nr_class - 1;
	int l = model->l;
	model->sv_coef = Malloc(double *, m);
	int i;
	for (i = 0; i<m; i++)
		model->sv_coef[i] = Malloc(double, l);
	model->SV = Malloc(svm_node*, l);
	svm_node *x_space = NULL;
	if (l>0) x_space = Malloc(svm_node, elements);

	int j = 0;
	for (i = 0; i<l; i++)
	{
		readline(fp);
		model->SV[i] = &x_space[j];

		p = strtok(line, " \t");
		model->sv_coef[0][i] = strtod(p, &endptr);
		for (int k = 1; k<m; k++)
		{
			p = strtok(NULL, " \t");
			model->sv_coef[k][i] = strtod(p, &endptr);
		}

		while (1)
		{
			idx = strtok(NULL, ":");
			val = strtok(NULL, " \t");

			if (val == NULL)
				break;
			x_space[j].index = (int)strtol(idx, &endptr, 10);
			x_space[j].value = strtod(val, &endptr);

			++j;
		}
		x_space[j++].index = -1;
	}
	free(line);

	setlocale(LC_ALL, old_locale);
	free(old_locale);

	if (ferror(fp) != 0 || fclose(fp) != 0)
		return NULL;

	model->free_sv = 1;	// XXX
	return model;
}


static char* readline(FILE *input)
{
	int len;

	if (fgets(line, max_line_len, input) == NULL)
		return NULL;

	while (strrchr(line, '\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *)realloc(line, max_line_len);
		len = (int)strlen(line);
		if (fgets(line + len, max_line_len - len, input) == NULL)
			break;
	}
	return line;
}



bool_C read_model_header(FILE *fp, svm_model* model)
{
	

	model->param.svm_type = user->svm_type; // 0->"c_svc", 1->"nu_svc", 2->"one_class", 3->"epsilon_svr", 4->"nu_svr", 5->"NULL"

	model->param.kernel_type = user->kernel_type;

	model->nr_class = user->nr_class;

	model->l = user->hog_length;

	int n = model->nr_class * (model->nr_class - 1) / 2;
	model->rho = Malloc(double, n);

	model->rho[0] = user->rho[0];
	model->rho[1] = user->rho[1];
	model->rho[2] = user->rho[2];

	n = model->nr_class;
	model->label = Malloc(int, n);

	model->label[0] = user->label[0];
	model->label[1] = user->label[1];
	model->label[2] = user->label[2];
	n = model->nr_class;
	model->nSV = Malloc(int, n);

	model->nSV[0] = user->nSV[0];
	model->nSV[1] = user->nSV[1];
	model->nSV[2] = user->nSV[2];


	return true;

}


int svm_get_svm_type(const svm_model *model)
{
	return model->param.svm_type;
}

int svm_get_nr_class(const svm_model *model)
{
	return model->nr_class;
}

void exit_input_error(int line_num)
{
	fprintf(stderr, "Wrong input format at line %d\n", line_num);
	exit(1);
}

double predict(char * line, svm_model *model)
{
	int correct = 0;
	int total = 0;
	double error = 0;
	double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0;

	int svm_type = svm_get_svm_type(model);
	int nr_class = svm_get_nr_class(model);
	double *prob_estimates = NULL;
	
	x = malloc(max_nr_attr * sizeof(svm_node));
		int i = 0;
		double target_label, predict_label;
		char *idx, *val, *label, *endptr;
		int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0

		label = strtok(line, " \t\n");


		target_label = strtod(label, &endptr);


		while (1)
		{
			if (i >= max_nr_attr - 1)	// need one more for index = -1
			{
				max_nr_attr *= 2;
				x = (struct svm_node *) realloc(x, max_nr_attr*sizeof(struct svm_node));
			}

			idx = strtok(NULL, ":");
			val = strtok(NULL, " \t");

			if (val == NULL)
				break;
			errno = 0;
			x[i].index = (int)strtol(idx, &endptr, 10);
			if (endptr == idx || errno != 0 || *endptr != '\0' || x[i].index <= inst_max_index)
				exit_input_error(total + 1);
			else
				inst_max_index = x[i].index;

			errno = 0;
			x[i].value = strtod(val, &endptr);
			if (endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(total + 1);

			++i;
		}
		x[i].index = -1;
			predict_label = svm_predict(model, x);
			free(x);
			return predict_label;

}


double svm_predict(const svm_model *model, const svm_node *x)
{
	int nr_class = model->nr_class;
	double *dec_values;
	if (model->param.svm_type == ONE_CLASS ||
		model->param.svm_type == EPSILON_SVR ||
		model->param.svm_type == NU_SVR)
		dec_values = Malloc(double, 1);
	else
		dec_values = Malloc(double, nr_class*(nr_class - 1) / 2);
	double pred_result = svm_predict_values(model, x, dec_values);
	free(dec_values);
	return pred_result;
}

double svm_predict_values(const svm_model *model, const svm_node *x, double* dec_values)
{
	int i;
	if (model->param.svm_type == ONE_CLASS ||
		model->param.svm_type == EPSILON_SVR ||
		model->param.svm_type == NU_SVR)
	{
		double *sv_coef = model->sv_coef[0];
		double sum = 0;
		for (i = 0; i<model->l; i++)
			sum += sv_coef[i] * k_function(x, model->SV[i], model->param);
		sum -= model->rho[0];
		*dec_values = sum;

		if (model->param.svm_type == ONE_CLASS)
			return (sum>0) ? 1 : -1;
		else
			return sum;
	}
	else
	{
		int nr_class = model->nr_class;
		int l = model->l;

		double *kvalue = Malloc(double, l);
		for (i = 0; i<l; i++)
			kvalue[i] = k_function(x, model->SV[i], model->param);

		int *start = Malloc(int, nr_class);
		start[0] = 0;
		for (i = 1; i<nr_class; i++)
			start[i] = start[i - 1] + model->nSV[i - 1];

		int *vote = Malloc(int, nr_class);
		for (i = 0; i<nr_class; i++)
			vote[i] = 0;

		int p = 0;
		for (i = 0; i<nr_class; i++)
			for (int j = i + 1; j<nr_class; j++)
			{
				double sum = 0;
				int si = start[i];
				int sj = start[j];
				int ci = model->nSV[i];
				int cj = model->nSV[j];

				int k;
				double *coef1 = model->sv_coef[j - 1];
				double *coef2 = model->sv_coef[i];
				for (k = 0; k<ci; k++)
					sum += coef1[si + k] * kvalue[si + k];
				for (k = 0; k<cj; k++)
					sum += coef2[sj + k] * kvalue[sj + k];
				sum -= model->rho[p];
				dec_values[p] = sum;

				if (dec_values[p] > 0)
					++vote[i];
				else
					++vote[j];
				p++;
			}

		int vote_max_idx = 0;
		for (i = 1; i<nr_class; i++)
			if (vote[i] > vote[vote_max_idx])
				vote_max_idx = i;

		free(kvalue);
		free(start);
		free(vote);
		return model->label[vote_max_idx];
	}
}

double k_function(const svm_node *x, const svm_node *y, const svm_parameter param)
{
	switch (param.kernel_type)
	{
	case LINEAR:
		return dot(x, y);
	case POLY:
		return powi(param.gamma*dot(x, y) + param.coef0, param.degree);
	case RBF:
	{
		double sum = 0;
		while (x->index != -1 && y->index != -1)
		{
			if (x->index == y->index)
			{
				double d = x->value - y->value;
				sum += d*d;
				++x;
				++y;
			}
			else
			{
				if (x->index > y->index)
				{
					sum += y->value * y->value;
					++y;
				}
				else
				{
					sum += x->value * x->value;
					++x;
				}
			}
		}

		while (x->index != -1)
		{
			sum += x->value * x->value;
			++x;
		}

		while (y->index != -1)
		{
			sum += y->value * y->value;
			++y;
		}

		return exp(-param.gamma*sum);
	}
	case SIGMOID:
		return tanh(param.gamma*dot(x, y) + param.coef0);
	case PRECOMPUTED:  //x: test (validation), y: SV
		return x[(int)(y->value)].value;
	default:
		return 0;  // Unreachable 
	}
}

double dot(const svm_node *px, const svm_node *py)
{
	double sum = 0;
	while (px->index != -1 && py->index != -1)
	{
		if (px->index == py->index)
		{
			sum += px->value * py->value;
			++px;
			++py;
		}
		else
		{
			if (px->index > py->index)
				++py;
			else
				++px;
		}
	}
	return sum;
}