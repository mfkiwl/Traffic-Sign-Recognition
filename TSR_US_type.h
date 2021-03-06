/****************************************************************************************
| Project Name: TSR
| Hardware: .
| Description:  US traffic sign detection algorithm
| File Name: TSR_US_type.h
|
|----------------------------------------------------------------------------------------
| C O P Y R I G H T
|----------------------------------------------------------------------------------------
|----------------------------------------------------------------------------------------
| A U T H O R I D E N T I T Y
|----------------------------------------------------------------------------------------
| Initials  Name                 Contact
| -------- --------------------- ----------------------
| T.Wang   Tianyu WANG          twang@icloud.com
|
|----------------------------------------------------------------------------------------
| R E V I S I O N H I S T O R Y
|----------------------------------------------------------------------------------------
| Date        Version Author Description
| ----------  ------- ------ --------------------------------------------------------------
| 2015-03-09  0.01.00 T.Wang Creation
|
|***************************************************************************************/

#ifndef __TSR_US_TYPE_H__
#define __TSR_US_TYPE_H__

/****************************************************************************************
* Include files
****************************************************************************************/


/****************************************************************************************
* Defines
****************************************************************************************/

/** Definition of TRUE*/
#ifdef TRUE
#undef TRUE
#endif
#define TRUE    1U

/** Definition of FALSE*/
#ifdef FALSE
#undef FALSE
#endif
#define FALSE   0U

#define ABS(a)			    (((a) > 0) ? (a) : -(a))
#define MAX_VAL(a, b)		( ((a) > (b)) ? (a) : (b) )
#define MIN_VAL(a, b)		( ((a) < (b)) ? (a) : (b) )

/****************************************************************************************
* Type definitions
****************************************************************************************/

typedef unsigned char               uint8_t;
typedef unsigned short              uint16_t;
typedef unsigned int                uint32_t;

typedef signed int                  int32_t;
typedef signed short                int16_t;
typedef signed char                 int8_t;

typedef float						float32_t;

typedef uint32_t     bool_t;    /**< use 32 bit because it could be faster */







/****************************************************************************************
* Global data definitions
****************************************************************************************/
#endif /* __TSR_US_TYPE_H__ */
/******************************** end of <TSR_US_type>.h **************************************/
