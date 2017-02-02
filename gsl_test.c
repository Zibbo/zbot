#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_cdf.h>
#include "zconv.h"

int main()
{
  int64_t count = 0;
  double g, sum = 0, start_time;
  start_time = ztime();

  for (g = -3; g < 3.0; g+=0.0000001)
    {
      sum+= gsl_cdf_ugaussian_P(g);
      count++;
    }
  printf("%lf %i %f\n", sum, count, ztime()-start_time);
}
