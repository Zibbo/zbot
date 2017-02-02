#include <pthread.h>

struct slots
{
  short int board_slot;
  short int hand_slots[1326];
};


int main()
{
  struct slots s[52];
  printf("%i\n", sizeof(pthread_mutex_t));
  printf("%i\n", sizeof(s));
}
