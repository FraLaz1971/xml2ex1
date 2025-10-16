#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int debug=0;
void leftpad(char *s,char c);
int swapword(unsigned short int *w);
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
  unsigned short (*arr)[ncols]=malloc(sizeof(unsigned short[nrows][ncols]));
  ifp=fopen(fname,"rb");
  for(i=0;i<nrows;i++){
    for(j=0;j<ncols;j++){
	res=fread(&arr[i][j],2,1,ifp);
	swapword(&arr[i][j]);
        printf("%3d ",arr[i][j]);
    }
    puts("");
  }
  fclose(ifp);
  return 0;
}
int swapword(unsigned short int *w){
  int status = 0;
  unsigned short int b1,b2;
  if (debug) printf("input word:%d\n",*w);
  b1=(*w)&255;
  b2=(*w)>>8;
  if (debug) printf("MSB:%d\n",b2);
  if (debug) printf("LSB:%d\n",b1);
  if (debug) printf("after swap:\n");
  *w=(b1<<8)+b2;
  if (debug) printf("output word:%d\n",*w);
  return status;
}

void leftpad(char *s,char c){
  int i,slen;
  slen=strlen(s);
  for(i=0;(i<slen)&&(s[i]==' ');++s){
    s[i]=c;
  }
  return ;
}

