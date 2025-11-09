#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* test dynamic memory arrays */
struct tda{
  FILE *ifp;
  FILE *ofp;
  char *infile;
  char *outfile;
  int **iarr;
  float **farr;
  unsigned long width;
  unsigned long height;
};
void mem_set(int val, struct tda *t);
void init_tda(unsigned long wi,unsigned long he,struct tda *t);
void destroy_tda(struct tda *t);
void fill_arrays(struct tda *t);
void print_tda(struct tda *t);
void save_int_array(char * fname,struct tda *t);
void save_float_array(char *fname,struct tda *t);
int main(int argc, char **argv){
  struct tda t1;
  unsigned long i,j;
  if(argc<2){
    fprintf(stderr,"usage:%s <width> <height>\n",argv[0]);
    return 1;
  }
  i=atol(argv[1]);j=atol(argv[2]);
  printf("going to allocate %ld x %ld arrays\n",i,j);
  init_tda(i,j,&t1);
/*  mem_set(0,&t1); */
  fill_arrays(&t1);
/*  print_tda(&t1); */
  t1.outfile="intarray001.raw";
  save_int_array(t1.outfile,&t1);
  t1.outfile="floatarray001.raw";
  save_float_array(t1.outfile,&t1);
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
void mem_set(int val, struct tda *t){
  unsigned long int i,j;
  for(i=0;i<t->height;i++){
    for(j=0;j<t->width;j++)
      t->iarr[i][j] = val;
  }
  for(i=0;i<t->height;i++){
    for(j=0;j<t->width;j++)
      t->farr[i][j]=(float)val;
  }
  return;
}
void init_tda(unsigned long wi,unsigned long he, struct tda *t){
  unsigned long i;
  t->width=wi;
  t->height=he;
  t->iarr=(int **)malloc(t->height*sizeof(int*));
  for(i=0;i<t->height;i++)
    t->iarr[i]=(int *)malloc(t->width*sizeof(int));
  t->farr=(float **)malloc(t->height*sizeof(float*));
  for(i=0;i<t->height;i++)
    t->farr[i]=(float *)malloc(t->width*sizeof(float));
  return;
}
void destroy_tda(struct tda *t){
  unsigned long i;
  for(i=0;i<t->height;i++)
    free((int *)t->iarr[i]);
  free((int **)t->iarr);
  for(i=0;i<t->height;i++)
    free((float *)t->farr[i]);
  free((float **)t->farr);
  return;
}
void save_int_array(char * fname,struct tda *t){
  unsigned long i,j;
  t->ofp=fopen(fname, "wb");
  if(!t->ofp){
    fprintf(stderr,"save_int_array() error in opening the file for writing, maybe the file exist?\n");
    exit(1);
  }
  for(i=0;i<t->height;i++){
    for(j=0;j<t->width;j++)
      fwrite(&(t->iarr[i][j]),sizeof(int),1,t->ofp);
  }
  fclose(t->ofp);
  return;
}
void save_float_array(char *fname,struct tda *t){
  unsigned long i,j;
  t->ofp=fopen(fname, "wb");
  if(!t->ofp){
    fprintf(stderr,"save_float_array() error in opening the file for writing, maybe the file exist?\n");
    exit(1);
  }
  for(i=0;i<t->height;i++){
    for(j=0;j<t->width;j++)
      fwrite(&(t->farr[i][j]),sizeof(float),1,t->ofp);
  }
  fclose(t->ofp);
  return;
}
void fill_arrays(struct tda *t){
  unsigned long int i,j;
  int count;
  count=0;
  for(i=0;i<t->height;i++){
    for(j=0;j<t->width;j++)
      t->iarr[i][j] = count++;
  }
  count=0;
  for(i=0;i<t->height;i++){
    for(j=0;j<t->width;j++)
      t->farr[i][j]=(float)count++;
  }
  return;
}
