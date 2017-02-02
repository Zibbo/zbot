#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <Judy.h>

#include "defs.h"
#include "bit.h"

/* uint64_t bitcount_64(uint64_t v) */
/* { */
/*   uint64_t c; */
/*   v = v - ((v >> 1) & (uint64_t)~(uint64_t)0/3);                           // temp */
/*   v = (v & (uint64_t)~(uint64_t)0/15*3) + ((v >> 2) & (uint64_t)~(uint64_t)0/15*3);      // temp */
/*   v = (v + (v >> 4)) & (uint64_t)~(uint64_t)0/255*15;                      // temp */
/*   c = (uint64_t)(v * ((uint64_t)~(uint64_t)0/255)) >> (sizeof(uint64_t) - 1) * CHAR_BIT; // count */
/*   return c; */
/* } */

/* uint32_t bitcount_32(uint32_t v) */
/* { */
/*   uint32_t c; */
/*   v = v - ((v >> 1) & (uint32_t)~(uint32_t)0/3);                           // temp */
/*   v = (v & (uint32_t)~(uint32_t)0/15*3) + ((v >> 2) & (uint32_t)~(uint32_t)0/15*3);      // temp */
/*   v = (v + (v >> 4)) & (uint32_t)~(uint32_t)0/255*15;                      // temp */
/*   c = (uint32_t)(v * ((uint32_t)~(uint32_t)0/255)) >> (sizeof(uint32_t) - 1) * CHAR_BIT; // count */
/*   return c; */
/* } */


/* uint64_t bitcount_before(uint64_t v, unsigned int pos) */
/* { */
/*   //uint64_t v;       // Compute the rank (bits set) in v from the MSB to pos. */
/*   //unsigned int pos; // Bit position to count bits upto. */
/*   uint64_t r;       // Resulting rank of bit at pos goes here. */

/*   // Shift out bits after given position. */
/*   r = v >> (sizeof(v) * CHAR_BIT - pos); */
/*   // Count set bits in parallel. */
/*   // r = (r & 0x5555...) + ((r >> 1) & 0x5555...); */
/*   r = r - ((r >> 1) & ~0UL/3); */
/*   // r = (r & 0x3333...) + ((r >> 2) & 0x3333...); */
/*   r = (r & ~0UL/5) + ((r >> 2) & ~0UL/5); */
/*   // r = (r & 0x0f0f...) + ((r >> 4) & 0x0f0f...); */
/*   r = (r + (r >> 4)) & ~0UL/17; */
/*   // r = r % 255; */
/*   r = (r * (~0UL/255)) >> ((sizeof(v) - 1) * CHAR_BIT); */
/*   return r; */
/* } */

/* inline int is_bit_set(uint64_t *d, int pos) */
/* { */
/*   return d[pos/(sizeof(*d)*CHAR_BIT)]&(0x8000000000000000>>(pos%(sizeof(*d)*CHAR_BIT))); */
/* } */

/* void set_negative_to_zero(float *f) */
/* { */
/*   *(uint32_t*)f = (~(-((*(uint32_t*)f)>>31))&(*(uint32_t*)f)); */
/* } */

/* inline void set_negative_to_zero2(float *f) */
/* { */
/*   *f= *f<0?0:*f; */
/* } */

/* struct bitfield */
/* { */
/*   int max_level; */
/*   int real_len; */
/*   int mem_len; // units */
/*   int mem_used_in_last; //in bits */
/*   uint64_t *d; */
/* }; */

/* void bf_is_bit_set(struct bitfield *b, int bit) */
/* { */
/*   int index = 0; */
/*   uint64_t bm = 0x8000000000000000; */
/*   uint64_t *d = b->d; */
/*   int cur_val = 0; */
/*   int level = b->max_level; */

/*   while (cur_val <= bit) */
/*     { */
/*       if (d[index] & bm) */
/* 	level--; */
/*       else */
/* 	{ */
/* 	  cur_val += pow(2,level); */
/* 	  level = bit_count32((cur_val&-cur_val)-1); */
/* 	} */
/*       if (bm == 0x1) */
/* 	{ */
/* 	  bm <<= (64-1); */
/* 	  index++; */
/* 	} */
/*     } */
/* } */

/* void bf_shift_right(struct bitfield *b, int start_bit) */
/* { */
/*   int i; */
/*   unit64_t next_carry = 0, cur_carry = 0; */

/*   start_i = start_bit/sizeof(*(b->d)); */
/*   start_bit = start_bit%sizeof(*(b->d)); */

/*   i = b->mem_len-1; */

/*   if (b->mem_used_in_last+bits >= sizeof(*(b->d))) */
/*     { */
/*       b->mem_len++; */
/*       b->d = realloc(b->d, sizeof(*(b->d))*b->mem_len); */
/*       b->d[b->mem_len-1] = b->d[b->mem_len-2]<<(sizeof(*(b->d))-bits); */
/*     } */
/*   prev_carry = 0; */
/*   cur_carry = b->d[i]<<(sizeof(*(b->d))*CHAR_BIT-1) */
/*   for (i = b->mem_len-1; i > start_i; i--) */
/*     { */
/*       b->d[i] = (b->d[i]>>1)& (b->d[i-1]<<(sizeof(*(b->d))*CHAR_BIT-1)); */
/*     } */
/*   //  b->d[i] = (-1<<sizeof(*(b->d))*CHAR_BIT-start_bit    b->d[i] */
/* } */
	     
int main()
{
  int i, retval, bit;
  float f,tot=0;
  void *j = NULL;
  struct situ *s;
  /* for (i = 0; i < 256; i++) */
  /*   { */
  /*     J1S(retval, j, random()%(256*256)); */
  /*   } */
  /* J1MU(retval, j); */
  /* printf("%i\n",  retval); */
  /* for (i = 0; i < 1000000000; i++) */
  /*   { */
  /*     f = (float)500000000-i; */
  /*     set_negative_to_zero2(&f); */
  /*     tot+=f; */
  /*   } */
  /* f = 123.0; */
  /* set_negative_to_zero(&f); */
  /* printf("%f\n",  f); */
  /* set_negative_to_zero2(&f); */
  /* printf("%f\n",  f); */
  /* printf("%f\n", tot); */
  uint64_t *bf = alloc_bitfield(128);
  /*  printf("%li %li\n", is_bit_set(bf,0)?1:0,is_bit_set(bf,1)?1:0);
  set_bit(bf,0);
  printf("%li %li\n", is_bit_set(bf,0)?1:0,is_bit_set(bf,1)?1:0);

  set_bit(bf,1);
  printf("%li %li\n", is_bit_set(bf,0)?1:0,is_bit_set(bf,1)?1:0);
  */
  memset(bf, 0x55, bitfield_bytesize(128));
  
  for (i = 0; i < 128; i++)
    {
      printf("bitcount %li\n", !is_bit_set(bf, i));
      //clear_bit(bf, bit);
    }
    //printf("bitcount %i\n", bitcount_before_bitmap(bf,i)); 
  /* printf("bitcount %i\n", bitcount_before_bitmap(bf,64)); */
  /* printf("bitcount %i\n", bitcount_before_bitmap(bf,65)); */
  /* printf("bitcount %i\n", bitcount_before_bitmap(bf,127)); */
  /* printf("bitcount %i\n", bitcount_before_bitmap(bf,128)); */
  //  printf("bitcount %i\n", bitcount_before_bitmap(bf,1));
  //printf("bitcount %i\n", bitcount_before_bitmap(bf,1));
  //printf("%i\n", sizeof(struct situ.hd)); 
  return 0;
}
