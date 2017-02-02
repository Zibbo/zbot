#include <unistd.h>
#include "khash.h"



KHASH_MAP_INIT_INT(32, short int)
int main() {
  int ret, is_missing, i,j;
  khiter_t k;
  for (j = 0; j < 20000; j++)
    {
      khash_t(32) *h = kh_init(32);
      
      for (i = 0; i < 8192; i+=4)
	{
	  k = kh_put(32, h, i, &ret);
	  kh_value(h,k) = i/10;
	}
    }  
  printf("done sleeping\n");
  sleep(20);
  /* k = kh_put(32, h, 5, &ret); */
  /* kh_value(h, k) = 10; */
  /* k = kh_get(32, h, 10); */
  /* is_missing = (k == kh_end(h)); */
  /* k = kh_get(32, h, 5); */
  /* kh_del(32, h, k); */
  /* for (k = kh_begin(h); k != kh_end(h); ++k) */
  /*   if (kh_exist(h, k)) kh_value(h, k) = 1; */
  //kh_destroy(32, h);
  return 0;
}
  
