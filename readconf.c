#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pds.h"
int main(void){
  unsigned short bitpix = 0; /* size of a pixel in bits */
  unsigned char sign = 0; /* 0=unsigned  1=signed */
  int end=0; /* 0=LSB 1=MSB */
  char ssign[16];
  char send[16];
  char dtype[1024];
  unsigned int width = 0;  /* Sample == n. of columns OO */
  unsigned int height = 0; /* Line == n. of rows */
  int cnt,res;
  const char* confname="arpds.conf";
  char buf[1024];
  FILE *cfp;
  cfp=fopen(confname, "r");
  if(cfp==NULL){
     perror("configuration file not present");
     exit(1);
  }
  /* read configuration file content into variables in memory */
  res=fscanf(cfp, "BITPIX %hu\n",&bitpix);
  res=fscanf(cfp, "SIGN %s\n",buf);
  if(!(strcmp(buf,"signed"))){
  	sign = 1;
	strcpy(ssign,"Signed");
  } else if(strcmp(buf,"unsigned")) {
	sign = 0;
	strcpy(ssign,"Unsigned");
  } else{
    perror("illegal sign option read");
    exit(1);
  }
  res=fscanf(cfp, "ENDIAN %s\n",buf);
  if(!(strcmp(buf,"LSB"))){
  	end = 0;
  } else if(strcmp(buf,"MSB")) {
	end = 1;
  } else{
    perror("illegal endianness option read");
    exit(1);
  }
  strcpy(send,buf);
  res=fscanf(cfp,  "WIDTH %d\n",&width);
  res=fscanf(cfp, "HEIGHT %d\n",&height);
  sprintf(dtype,"%s%s%u",ssign,send,bitpix/8);
  /* printout variables content for debug */
  fprintf(stderr,"BITPIX = %d\n",bitpix);
  fprintf(stderr,"SIGN = %d\n",sign);
  fprintf(stderr,"ENDIAN = %d\n",end);
  fprintf(stderr,"WIDTH = %d\n",width);/* Sample == n. of columns OO */
  fprintf(stderr,"HEIGHT = %d\n",height); /* Line == n. of rows */
  fprintf(stderr,"PDS4 <data_type> = %s\n",dtype);
  fclose(cfp);
  return 0;
}
