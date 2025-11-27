#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAXSIZE  10000
#define MAXWIDTH  10000
#define MAXHEIGHT 10000 
#define MAXCHARS  60000 
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif
enum file_format{ASCII,BINARY,ASCII_TABLE,BINARY_TABLE,FLOAT_BINARY,DOUBLE_BINARY,ASCIIERR,GPTABLE,
	FITS_IMAGE,FITS_ASCII_TABLE,FITS_BINARY_TABLE,PDS4,NETCDF,PS,PNG,GIF};

int main(int argc, char **argv){
  FILE *ifp, *ofp;
  int i, j, res, pos,debug=0;
  unsigned int height;
  unsigned int width;
  char *ifname;
  char ofname[256];
  char base[256];
  char *result_ptr;
  char line[MAXCHARS]; char *sep=" ";/* field separator */
  char *val; char msg[1024];
  unsigned char **arr;
  unsigned char **temp_arr;
  int ftype;
  if(argc<2){
    fprintf(stderr,"usage:%s <infile.asc>\n",argv[0]);
    return 1;
  }
  ifname=argv[1];
  width=MAXWIDTH;
  height=MAXHEIGHT;
  arr=(unsigned char**)malloc(height*sizeof(unsigned char*));
  for(i=0; i<height; i++)
	arr[i]=(unsigned char*)malloc(width*sizeof(unsigned char));
    fprintf(stderr,"opening image file %s for reading\n", ifname);
        fprintf(stderr,"data format is ASCII\n");
		ifp = fopen(ifname, "r");
		if (ifp == NULL) {
	        fprintf(stderr,"ERROR: no such file %s", ifname);
	        return 0;
		}
		i=0;j=0;
		while (fgets(line,MAXCHARS,ifp)){ /* loop on the ascii file rows*/
			val = strtok(line," ");
			while(val&&(val[0]!='\n')){
				if(debug)fprintf(stderr, "val: %d len: %ld\n" , val[0], strlen(val));
				arr[i][j]=(unsigned char)atoi(val);
				if (debug) fprintf(stderr,"%d ",arr[i][j]);
				val = strtok(NULL, " ");
				j++;
				if(i==0) width = j;
			}
			if (debug) fprintf(stderr,"\n");
			j=0;
			i++;
		}
        fprintf(stderr,"ended reading image data\n");
		height = i;	
		fprintf(stderr,"image width: %u height: %u \n", width,height);
		temp_arr=(unsigned char**)realloc(arr,height*sizeof(unsigned char*));
        fprintf(stderr,"re-allocated the rows pointers\n");
		if (temp_arr == NULL) {
			// If realloc fails, the original memory block is not freed and is still valid
			printf("Memory reallocation failed, original pointer still valid\n");
			free(arr); // Free the original block before exiting to prevent leak
			return 1;
		} else {
			// If realloc succeeds, update the original pointer to the new memory location
			arr = temp_arr;
		}
		for(i=0; i<height; i++){
			temp_arr[i]=(unsigned char*)realloc(arr[i],width*sizeof(unsigned char));
			if (temp_arr[i] == NULL) {
			// If realloc fails, the original memory block is not freed and is still valid
			printf("Memory reallocation failed, original pointer still valid\n");
			free(temp_arr[i]); // Free the original block before exiting to prevent leak
			return 1;
			} else {
			// If realloc succeeds, update the original pointer to the new memory location
			arr[i] = temp_arr[i];
			if(debug)fprintf(stderr,"re-allocated the row %u\n",i);
			}
		}
		fprintf(stderr,"re-allocated all the rows %u\n",i);
		fclose(ifp);
        fprintf(stderr,"image input file closed\n");
  

  /* create the output file name */
    result_ptr=strstr(ifname,".");
    pos=result_ptr-ifname;
    strncpy(base,ifname,pos);
    base[pos]='\0';
    snprintf(ofname,255,"%s%4s",base,".img");
    fprintf(stderr,"output file name: %s\n",ofname);

  /* write row data array 8bits on a binary file */
  ofp=fopen(ofname,"wb");
  for(i=0;i<height;i++){
    for(j=0;j<width;j++){
	  res=fwrite(&arr[i][j],1,1,ofp);
    }
  }
  fclose(ofp);
  return 0;
}
