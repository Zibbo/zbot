#ifndef _BIT_H_
#define _BIT_H_

uint64_t bitcount_64(uint64_t v);
uint32_t bitcount_32(uint32_t v);
uint64_t bitcount_before(uint64_t v, unsigned int pos);
uint32_t bitcount_before_bitmap(uint64_t *d, int32_t bit);
uint64_t is_bit_set(uint64_t *d, int pos);
uint32_t is_lowest_bit_set(uint64_t *d, int bits, int pos);
uint32_t is_highest_bit_set(uint64_t *d, int bits, int pos);
uint32_t get_nth_set_bit_bitmap(uint64_t *d, int n);
void set_bit(uint64_t *d, int pos);
void clear_bit(uint64_t *d, int pos);
uint64_t *alloc_bitfield(int size);
int bitfield_bytesize(int bits);
int bitfield_wordsize(int bits);
void bitfield_and(uint64_t *target, uint64_t *src1, uint64_t *src2, int n_bits);
int biterate(uint64_t *d, int last_i, int bits);

#endif
