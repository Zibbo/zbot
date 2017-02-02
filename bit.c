#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>

#include "zconv.h"

uint32_t bitcount_64(uint64_t v)
{
  uint64_t c;
  v = v - ((v >> 1) & (uint64_t)~(uint64_t)0/3);                           // temp
  v = (v & (uint64_t)~(uint64_t)0/15*3) + ((v >> 2) & (uint64_t)~(uint64_t)0/15*3);      // temp
  v = (v + (v >> 4)) & (uint64_t)~(uint64_t)0/255*15;                      // temp
  c = (uint64_t)(v * ((uint64_t)~(uint64_t)0/255)) >> (sizeof(uint64_t) - 1) * CHAR_BIT; // count
  return (uint32_t)c;
}

uint32_t bitcount_32(uint32_t v)
{
  uint32_t c;
  v = v - ((v >> 1) & (uint32_t)~(uint32_t)0/3);                           // temp
  v = (v & (uint32_t)~(uint32_t)0/15*3) + ((v >> 2) & (uint32_t)~(uint32_t)0/15*3);      // temp
  v = (v + (v >> 4)) & (uint32_t)~(uint32_t)0/255*15;                      // temp
  c = (uint32_t)(v * ((uint32_t)~(uint32_t)0/255)) >> (sizeof(uint32_t) - 1) * CHAR_BIT; // count
  return c;
}


uint32_t bitcount_before(uint64_t v, unsigned int pos)
{
  //uint64_t v;       // Compute the rank (bits set) in v from the MSB to pos.
  //unsigned int pos; // Bit position to count bits upto.
  uint64_t r;       // Resulting rank of bit at pos goes here.

  // Shift out bits after given position.
  r = v >> (sizeof(v) * CHAR_BIT - pos);
  // Count set bits in parallel.
  // r = (r & 0x5555...) + ((r >> 1) & 0x5555...);
  r = r - ((r >> 1) & ~0UL/3);
  // r = (r & 0x3333...) + ((r >> 2) & 0x3333...);
  r = (r & ~0UL/5) + ((r >> 2) & ~0UL/5);
  // r = (r & 0x0f0f...) + ((r >> 4) & 0x0f0f...);
  r = (r + (r >> 4)) & ~0UL/17;
  // r = r % 255;
  r = (r * (~0UL/255)) >> ((sizeof(v) - 1) * CHAR_BIT);
  return (uint32_t)r;
}

uint32_t bitcount_before_bitmap(uint64_t *d, int32_t bit)
{
  uint32_t bitcount = 0;
  int i = 0;
  while (bit >= sizeof(uint64_t)*CHAR_BIT)
    {
      bitcount += bitcount_64(d[i++]);
      bit -= sizeof(uint64_t)*CHAR_BIT;
    }
  if (bit != 0)
    bitcount += bitcount_before(d[i], bit);
  return bitcount;
}

uint64_t is_bit_set(uint64_t *d, int pos)
{
  return d[pos/(sizeof(*d)*CHAR_BIT)]&(0x8000000000000000>>(pos%(sizeof(*d)*CHAR_BIT)));
}

uint32_t is_lowest_bit_set(uint64_t *d, int bits, int pos)
{
  if (bitcount_before_bitmap(d, pos))
    return 0;
  return 1;
}

uint32_t is_highest_bit_set(uint64_t *d, int bits, int pos)
{
  if (bitcount_before_bitmap(d, pos+1) < bitcount_before_bitmap(d, bits))
    return 0;
  return 1;
}

uint32_t get_nth_set_bit(uint64_t v, int r)
{
  //  uint64_t v;          // Input value to find position with rank r.
  //  unsigned int r;      // Input: bit's desired rank [1-64].
  unsigned int s;      // Output: Resulting position of bit with rank r [1-64]
  uint64_t a, b, c, d; // Intermediate temporaries for bit count.
  unsigned int t;      // Bit count temporary.
  
  // Do a normal parallel bit count for a 64-bit integer,                     
  // but store all intermediate steps.                                        
  // a = (v & 0x5555...) + ((v >> 1) & 0x5555...);
  a =  v - ((v >> 1) & ~0UL/3);
  // b = (a & 0x3333...) + ((a >> 2) & 0x3333...);
  b = (a & ~0UL/5) + ((a >> 2) & ~0UL/5);
  // c = (b & 0x0f0f...) + ((b >> 4) & 0x0f0f...);
  c = (b + (b >> 4)) & ~0UL/0x11;
  // d = (c & 0x00ff...) + ((c >> 8) & 0x00ff...);
  d = (c + (c >> 8)) & ~0UL/0x101;
  t = (d >> 32) + (d >> 48);
  // Now do branchless select!                                                
  s  = 64;
  // if (r > t) {s -= 32; r -= t;}
  s -= ((t - r) & 256) >> 3; r -= (t & ((t - r) >> 8));
  t  = (d >> (s - 16)) & 0xff;
  // if (r > t) {s -= 16; r -= t;}
  s -= ((t - r) & 256) >> 4; r -= (t & ((t - r) >> 8));
  t  = (c >> (s - 8)) & 0xf;
  // if (r > t) {s -= 8; r -= t;}
  s -= ((t - r) & 256) >> 5; r -= (t & ((t - r) >> 8));
  t  = (b >> (s - 4)) & 0x7;
  // if (r > t) {s -= 4; r -= t;}
  s -= ((t - r) & 256) >> 6; r -= (t & ((t - r) >> 8));
  t  = (a >> (s - 2)) & 0x3;
  // if (r > t) {s -= 2; r -= t;}
  s -= ((t - r) & 256) >> 7; r -= (t & ((t - r) >> 8));
  t  = (v >> (s - 1)) & 0x1;
  // if (r > t) s--;
  s -= ((t - r) & 256) >> 8;
  s = 65 - s;
  return s;
}

uint32_t get_nth_set_bit_bitmap(uint64_t *d, int n)
{
  int i = 0;
  uint32_t bc, retval = 0;
  n+=1;
  while ((bc=bitcount_64(d[i])) < n)
    {
      retval += sizeof(*d)*CHAR_BIT;
      n -= bc;
      i++;
    }
  return retval+get_nth_set_bit(d[i], n)-1;
}


void set_bit(uint64_t *d, int pos)
{
  d[pos/(sizeof(*d)*CHAR_BIT)] = d[pos/(sizeof(*d)*CHAR_BIT)] | (0x8000000000000000>>(pos%(sizeof(*d)*CHAR_BIT)));
}

void clear_bit(uint64_t *d, int pos)
{
  d[pos/(sizeof(*d)*CHAR_BIT)] = d[pos/(sizeof(*d)*CHAR_BIT)] & (~(0x8000000000000000>>(pos%(sizeof(*d)*CHAR_BIT))));
}

uint64_t *alloc_bitfield(int size)
{
  assert(size > 0);
  return calloc(1,sizeof(uint64_t)*((size-1)/(sizeof(uint64_t)*CHAR_BIT)+1));
}

int bitfield_bytesize(int bits)
{
  return sizeof(uint64_t)*((bits-1)/(sizeof(uint64_t)*CHAR_BIT)+1);
}

int bitfield_wordsize(int bits)
{
  return ((bits-1)/(sizeof(uint64_t)*CHAR_BIT)+1);
}

int get_random_zero_bit(uint64_t *d, int len)
{
  int b, i = 0;
  do
    {
      b = zrandom_r()%len;
      i++;
    }
  while (is_bit_set(d, b) && i < 1000000);
  if (i < 1000000)
    return b;
  return -1;
}

void bitfield_and(uint64_t *target, uint64_t *src1, uint64_t *src2, int n_bits)
{
  int i;

  for (i = 0; i < (n_bits-1)/sizeof(uint64_t)+1;i++)
    {
      target[i] = src1[i]&src2[i];
    }
}

int biterate(uint64_t *d, int last_i, int bits)
{
  last_i++;     
  int i = last_i/(sizeof(uint64_t)*CHAR_BIT);
  int pos = last_i%(sizeof(uint64_t)*CHAR_BIT);
  uint64_t mask = 0x8000000000000000>>pos;
  
  while (last_i < bits && d[i] == 0)
    {
      i++;
      last_i += sizeof(uint64_t)*CHAR_BIT;
    }
  
  while (last_i < bits)
    {
      if (d[i]&mask)
	return last_i;
      //i += mask&1;
      //mask = mask>>1 | mask<<(sizeof(mask)*CHAR_BIT - 1);
      mask = mask>>1;
      last_i++;      
      if (!mask)
	{
	  mask = 0x8000000000000000;
	  while (last_i < bits && d[++i] == 0)
	    last_i += sizeof(uint64_t)*CHAR_BIT;
	}
    }
  return -1;
}
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
	     
/* int main() */
/* { */
/*   int i, retval; */
/*   float f,tot=0; */
/*   void *j = NULL; */

/*   for (i = 0; i < 256; i++) */
/*     { */
/*       J1S(retval, j, zrandom_r()%(256*256)); */
/*     } */
/*   J1MU(retval, j); */
/*   printf("%i\n",  retval); */
/*   /\* for (i = 0; i < 1000000000; i++) *\/ */
/*   /\*   { *\/ */
/*   /\*     f = (float)500000000-i; *\/ */
/*   /\*     set_negative_to_zero2(&f); *\/ */
/*   /\*     tot+=f; *\/ */
/*   /\*   } *\/ */
/*   /\* f = 123.0; *\/ */
/*   /\* set_negative_to_zero(&f); *\/ */
/*   /\* printf("%f\n",  f); *\/ */
/*   /\* set_negative_to_zero2(&f); *\/ */
/*   /\* printf("%f\n",  f); *\/ */
/*   /\* printf("%f\n", tot); *\/ */

/*   /\* printf("%i\n", bitcount_before(0x0fffffff0fffffff, 63)); *\/ */
/*   return 0; */
/* } */
