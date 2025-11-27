#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv){
  FILE *ifp;
  int i, j, res;
  unsigned int nrows;
  unsigned int ncols;
  char *fname;
  unsigned char **arr;
  if(argc<4){
    fprintf(stderr,"usage:%s <nrows> <ncols> <infile>\n",argv[0]);
    return 1;
  }
  nrows=atoi(argv[1]);
  ncols=atoi(argv[2]);
  fname=argv[3];
  arr=(unsigned char**)malloc(nrows*sizeof(unsigned char*));
  for(i=0; i<nrows; i++)
	arr[i]=(unsigned char*)malloc(ncols*sizeof(unsigned char))
  ifp=fopen(fname,"rb");
  for(i=0;i<nrows;i++){
    for(j=0;j<ncols;j++){
	res=fread(&arr[i][j],1,1,ifp);
        printf("%3d ",arr[i][j]);
    }
    puts("");
  }
  fclose(ifp);
  return 0;
}
