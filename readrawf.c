#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv){
  FILE *ifp;
  int i, j, res;
  unsigned int nrows;
  unsigned int ncols;
  char *fname;
  float **arr;
  if(argc<4){
    fprintf(stderr,"usage:%s <nrows> <ncols> <infile>\n",argv[0]);
    return 1;
  }
  fname=argv[3];
  nrows=atof(argv[1]);
  ncols=atof(argv[2]);
  arr=(float **)malloc(nrows*sizeof(float *));
  for(i=0;i<nrows;i++)
    arr[i]=(float*)malloc(ncols*sizeof(float));
  ifp=fopen(fname,"rb");
  for(i=0;i<nrows;i++){
    for(j=0;j<ncols;j++){
	res=fread(&arr[i][j],4,1,ifp);
        printf("%15.6f ",10000*arr[i][j]);
    }
    puts("");
  }
  fclose(ifp);
  for(i=0;i<nrows;i++)
    free(arr[i]);
  free(arr);  
  return 0;
}
