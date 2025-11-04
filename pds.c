#include <stdlib.h>
#include "pds.h"
int verbose = 0;

int init_pds(FILE *xmlp,struct PDS *pds,char **prodfnam, char **argv){
	int i;
	xmlp = stderr;
	pds->version=4;
	pds->numproducts=atoi(argv[1]);
	fprintf(stderr,"init_pds() initing structures and data\n");
	fprintf(stderr,"init_pds() pds version is %d\n",pds->version);
	fprintf(stderr,"init_pds() pds number of products is %d\n",pds->numproducts);
	for(i=0;i<pds->numproducts;i++)
		fprintf(stderr,"init_pds() pds product n. %d : %s\n",i,pds->pfnames[i]);
	return 0;
}
int read_pds(FILE *ixmlp,struct PDS *pds){
	return 0;
}
int write_pds(FILE *oxmlp,struct PDS *pds){
	return 0;
}
int close_pds(FILE *xmlp,struct PDS *pds){
	return 0;
}
