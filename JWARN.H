/*
 *	jwarn.h
 */

/*
 *	Siez of Line Buffer
 */
#define LBUFSIZE	1024

/*
 *	Nr of currently used rules & max number of rules on command line
 */
#ifdef EXPERIMENTAL
#define NR_RULES	13
#else
#define NR_RULES	10
#endif
#define MAXRULES	64

/*
 *	GPU or DSP
 */
#define GPU_CODE	0
#define DSP_CODE	1

