#ifndef __TSR_US_recognitionCore_h__
#define __TSR_US_recognitionCore_h__

#include "TSR_US_imageOperation.h"
#include "TSR_US_tracking.h"


typedef int bool_C;

#define true 1

#define false 0

enum { C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR };	/* svm_type */
enum { LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED }; /* kernel_type */


typedef struct svm_node
{
	int index;
	double value;
} svm_node;




typedef struct 
{
	int svm_type;
	int kernel_type;
	int degree;	/* for poly */
	double gamma;	/* for poly/rbf/sigmoid */
	double coef0;	/* for poly/sigmoid */

					/* these are for training only */
	double cache_size; /* in MB */
	double eps;	/* stopping criteria */
	double C;	/* for C_SVC, EPSILON_SVR and NU_SVR */
	int nr_weight;		/* for C_SVC */
	int *weight_label;	/* for C_SVC */
	double* weight;		/* for C_SVC */
	double nu;	/* for NU_SVC, ONE_CLASS, and NU_SVR */
	double p;	/* for EPSILON_SVR */
	int shrinking;	/* use the shrinking heuristics */
	int probability; /* do probability estimates */
} svm_parameter;








typedef struct svm_model
{
	svm_parameter param;	/* parameter */
	int nr_class;		/* number of classes, = 2 in regression/one class svm */
	int l;			/* total #SV */
	struct svm_node **SV;		/* SVs (SV[l]) */
	double **sv_coef;	/* coefficients for SVs in decision functions (sv_coef[k-1][l]) */
	double *rho;		/* constants in decision functions (rho[k*(k-1)/2]) */
	double *probA;		/* pariwise probability information */
	double *probB;
	int *sv_indices;        /* sv_indices[0,...,nSV-1] are values in [1,...,num_traning_data] to indicate SVs in the training set */

							/* for classification only */

	int *label;		/* label of each class (label[k]) */
	int *nSV;		/* number of SVs for each class (nSV[k]) */
					/* nSV[0] + nSV[1] + ... + nSV[k-1] = l */
					/* XXX */
	int free_sv;		/* 1 if svm_model is created by svm_load_model*/
						/* 0 if svm_model is created by svm_train */
}svm_model;

int HOG_SVM(int locCountBlockNum, int i, int locCountTimes, TSR_CountBlock ** locCountBlock, UserDefine * user, TSR_Image * img,
	svm_model * model, IplImage * m_pViewImg, char * featureBuffer);
int majority_vote(int locCountBlockNum, int i, int locCountTimes, TSR_CountBlock ** locCountBlock);
int array_max(int * vote, int num);
svm_model *svm_load_model(const char *model_file_name);
static char* readline(FILE *input);
bool_C read_model_header(FILE *fp, svm_model* model);
int svm_get_svm_type(const svm_model *model);
int svm_get_nr_class(const svm_model *model);
double predict(char * line, svm_model *model);
void exit_input_error(int line_num);
double svm_predict(const svm_model *model, const svm_node *x);
double svm_predict_values(const svm_model *model, const svm_node *x, double* dec_values);
double k_function(const svm_node *x, const svm_node *y, const svm_parameter param);
static double powi(double base, int times);
double dot(const svm_node *px, const svm_node *py);

#endif
