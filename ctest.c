
struct arr
{
  float foo[100];
};

struct testi
{
  float jee[10];
  //float foo[][100];
  //  struct arr *foo;
};

int main()
{
  struct testi t;
  long double jeejee;
  __float80 test;
  __float128 ttt;
  float *hwev[2];
  printf("%i %i\n", sizeof(hwev), sizeof(struct testi));
  printf("%i %i %i\n", sizeof(jeejee), sizeof(test), sizeof(ttt));
}
