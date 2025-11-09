#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv){
  FILE *ifp;
  int i, j, res;
  if(argc<4){
    fprintf(stderr,"usage:%s <nrows> <ncols> <infile>\n",argv[0]);
    return 1;
  }
  unsigned int nrows=atoi(argv[1]);
  unsigned int ncols=atoi(argv[2]);
  char *fname=argv[3];
  int (*arr)[ncols]=malloc(sizeof(int[nrows][ncols]));
  ifp=fopen(fname,"rb");
  for(i=0;i<nrows;i++){
    for(j=0;j<ncols;j++){
	res=fread(&arr[i][j],4,1,ifp);
        printf("%3d ",arr[i][j]);
    }
    puts("");
  }
  fclose(ifp);
  return 0;
}
