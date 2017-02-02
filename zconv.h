#ifndef _ZCONV_H_
#define _ZCONV_H_

#include <gsl/gsl_rng.h>

double ztime();
double ztime_double();
float ztime_float();
void zsleep_float(float seconds);
void zsleep_minimal();

double zrand();
double zrand_gsl(gsl_rng *rng);
long zrandom_r();
void *zalloc_refcount(int size);
void *zfree_refcount(void *item);
void zinc_refcount(void *item);
int *get_shuffled_array(int start, int len, int inc);
#endif
