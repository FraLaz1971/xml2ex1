#include <stdio.h>
#include <string.h>
#include "png.h"
#define MEMZERO(var) ((void)memset(&var, 0, sizeof(var)))
struct test1{
  int iel;
  float fel;
  unsigned char ubel;
  unsigned short uwel;
  long lel;
};
void print_test1(struct test1 *t1);
void set_test1(int i, float f, unsigned char ub,
unsigned short uw, long l, struct test1 *t1);
int main(){
  int i;
  int v1[10];
  memset(&v1,0,sizeof(v1));
  for(i=0;i<10;i++)
    printf("%d\n", v1[1]);
  struct test1 t1;
  set_test1(-128,6.666,255,65535,12001,&t1);
  print_test1(&t1);
  printf("zeroing structure\n");
  MEMZERO(t1);
  print_test1(&t1);
  return 0;
}
void print_test1(struct test1 *t1){
  printf("iel:%d\n",t1->iel);
  printf("fel:%f\n",t1->fel);
  printf("ubel:%d\n",t1->ubel);
  printf("uwel:%u\n",t1->uwel);
  printf("lel:%ld\n",t1->lel);
}
void set_test1(int i, float f, unsigned char ub,
unsigned short uw, long l, struct test1 *t1){
  t1->iel = i;
  t1->fel = f;
  t1->ubel = ub;
  t1->uwel = uw;
  t1->lel = l;
}
