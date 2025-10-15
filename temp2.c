#include <stdio.h>
#include <stdlib.h>
/* test dynamic memory arrays */
struct tda{
  int **iarr;
  float **farr;
  unsigned long width;
  unsigned long height;
};
void init_tda(unsigned long wi,unsigned long he,struct tda *t);
void destroy_tda(struct tda *t);
void print_tda(struct tda *t);
int main(int argc, char **argv){
  struct tda t1;
  unsigned long i,j;
  if(argc<2){
    fprintf(stderr,"usage:%s <width> <height>",argv[0]);
    return 1;
  }
  i=atol(argv[1]);j=atol(argv[2]);
  printf("going to allocate %ld x %ld arrays\n",i,j);
  init_tda(i,j,&t1);
  printf("size of t1:%ld\n",sizeof(t1.iarr));
//  memset(&t1.iarr[0][0],-1,t1.width*sizeof(int*)+t1.width*t1.height*sizeof(int));
  print_tda(&t1);
  destroy_tda(&t1);
  return 0;
}
void print_tda(struct tda *t){
  unsigned long int i,j;
  for(i=0;i<t->height;i++){
    for(j=0;j<t->width;j++)
      printf("%d ",t->iarr[i][j]);
    puts("");
  }
  for(i=0;i<t->height;i++){
    for(j=0;j<t->width;j++)
      printf("%f ",t->farr[i][j]);
    puts("");
  }
  return;
}
void init_tda(unsigned long wi,unsigned long he, struct tda *t){
  unsigned long i;
  t->width=wi;
  t->height=he;
  printf("%ld x %ld\n",t->width,t->height);
  t->iarr=(int **)malloc(t->width*sizeof(int*));
  for(i=0;i<t->width;i++)
    t->iarr[i]=(int *)malloc(t->height*sizeof(int));
  t->farr=(float **)malloc(t->width*sizeof(float*));
  for(i=0;i<t->width;i++)
    t->farr[i]=(float *)malloc(t->height*sizeof(float));
  return;
}
void destroy_tda(struct tda *t){
  unsigned long i;
  for(i=0;i<t->width;i++)
    free((int *)t->iarr[i]);
  free((int **)t->iarr);
  for(i=0;i<t->width;i++)
    free((float *)t->farr[i]);
  free((float **)t->farr);
  return;
}
