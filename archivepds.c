#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/tree.h>
#include "pds.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
	int verbose = 1;
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, pi=NULL;/* node pointers */
    xmlNodePtr p1 = NULL; /* node pointers */
    xmlDtdPtr dtd = NULL;       /* DTD pointer */
    xmlAttr *attribute = NULL;
    xmlAttrPtr attr = NULL;
    xmlNs *namespace_decl = NULL;
    unsigned short bitpix = 0; /* size of a pixel in bits */
    unsigned char sign = 0; /* 0=unsigned  1=signed */
    int end=0; /* 0=LSB 1=MSB */
    char ssign[16];
    char send[16];
    char dtype[1024];
    char dname[64];
    char swidth[32];
    char sheight[32];
	char prodlid[MAXLEAV][256];
	char prodvid[MAXLEAV][256];
	char stitle[1024];
	char imvers[32];
    char moddate[16];
	char prodclass[64];
	char svers[16];
	char descr[1024];
	char tstart[32];
	char tstop[32];
	char purp[32];
	char proclev[16];
	char resdescr[64];
	char lambda[64];
	char domain[64];
	char discipl[64];
	char invarea[64];
	char invtype[64];
	char invlid[256];
	char reftype[64];
	char mission[64];
	char miss_id[64];
	char su[64];
	char istatus[64];
	char onamval[MAXLEAV][1024];
	char otypval[MAXLEAV][1024];
    unsigned int width = 0;  /* Sample == n. of columns OO */
    unsigned int height = 0; /* Line == n. of rows */
    const char* confname;
    int j;
    char buf[1024], key[256], val[1024];
    FILE *cfp;
    int i, res, numproducts, npi, npoattr, nosc, nci;
	struct PDS pds; struct PRODUCT_OBSERVATIONAL po; struct IDENTIFICATION_AREA ia;
	struct OBSERVATION_AREA oa; struct FILE_AREA_OBSERVATIONAL fao;
	struct ELEMENT logical_identifier;
	struct ELEMENT version_id;
	struct ELEMENT title;
	struct ELEMENT information_model_version;
	struct ELEMENT product_class;
	struct ELEMENT citation_information;
	struct ELEMENT modification_history;
	struct ELEMENT primary_result_summary;
	struct ELEMENT mission_area;
	struct ELEMENT discipline_area;
	struct OBSERVING_SYSTEM observing_system;
	struct ELEMENT file;	
	struct TIME_COORDINATES timecoord;
	struct INTERNAL_REFERENCE iref;
	struct INVESTIGATION_AREA iaa;
	struct ARRAY_2D_IMAGE array2d;
	FILE *lp; /* pds4 label file handler */
	char **prodfnam;
	if(argc<3){
		fprintf(stderr,"usage:%s <file.conf> <nproducts> <prodname1> <prodname2> ... <prodnamen>\n",argv[0]);
		fprintf(stderr,"e.g: %s arpdf.conf 1 image.raw \n",argv[0]);
		return 1;
	}
	numproducts = atoi(argv[2]); /* number of products pointed by the label */
	prodfnam=(char **)malloc(numproducts*sizeof(char *));
	for(i=0;i<numproducts;i++)
		prodfnam[i]=(char *)malloc(MAXFNAML);
	pds.products=(FILE**)malloc(numproducts*sizeof(FILE *)); 
	if(verbose)fprintf(stderr,"main() numproducts = %d\n",numproducts);
	for(i=0;i<numproducts;i++){
		pds.products[i] = (FILE*)malloc(2048);
		if(argv[i+2]!=NULL){
			strcpy((char*)prodfnam[i],argv[i+3]);
		} else {
			perror("ERROR: filename lacking, exiting");
			exit(1);
		}
	}
/* allocate memory for nodes */
	oa.target=(struct TARGET_IDENTIFICATION*)malloc(3*sizeof(struct TARGET_IDENTIFICATION));
	oa.target->iref=(struct INTERNAL_REFERENCE*)malloc(sizeof(struct INTERNAL_REFERENCE));
	array2d.leaves[0].attributes=(struct ATTRIBUTE *)malloc(sizeof(struct ATTRIBUTE));	
	po.attributes=(struct ATTRIBUTE *)malloc(MAXLEAV*sizeof(struct ATTRIBUTE));
	logical_identifier.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	logical_identifier.leaves[0].leaves=(struct ELEMENT *)malloc(5*sizeof(struct ELEMENT));
	modification_history.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	modification_history.leaves[0].leaves=(struct ELEMENT *)malloc(3*sizeof(struct ELEMENT));
	primary_result_summary.leaves=(struct ELEMENT *)malloc(4*sizeof(struct ELEMENT));
	primary_result_summary.leaves[3].leaves=(struct ELEMENT *)malloc(3*sizeof(struct ELEMENT));
	mission_area.leaves=(struct ELEMENT *)malloc(5*sizeof(struct ELEMENT));
	for(i=0;i<5;i++)
		mission_area.leaves[i].leaves=(struct ELEMENT *)malloc(6*sizeof(struct ELEMENT));
	mission_area.leaves[2].leaves[0].attributes=(struct ATTRIBUTE *)malloc(sizeof(struct ATTRIBUTE));
	for(i=0;i<2;i++){
		observing_system.osc[i].name=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
		observing_system.osc[i].type=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	}
	observing_system.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	discipline_area.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	discipline_area.leaves[0].leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	discipline_area.leaves[0].leaves[0].leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	discipline_area.leaves[0].leaves[0].leaves[0].leaves=(struct ELEMENT *)malloc(256*sizeof(struct ELEMENT));
	file.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	for(i=0;i<6;i++)
		array2d.leaves[i].leaves=(struct ELEMENT*)malloc(MAXLEAV*sizeof(struct ELEMENT));
	/*read configutation file */
  confname=argv[1];
  cfp=fopen(confname, "r");
  if(cfp==NULL){
     perror("configuration file not present");
     exit(1);
  }
 /* read configuration file content into variables in memory */
  res=fscanf(cfp, "BITPIX %hu\n",&bitpix);
  res=fscanf(cfp, "SIGN %10s\n",buf);
  fprintf(stderr, "read SIGN: %s\n",buf);
  if(!(strcmp(buf,"signed"))){
  	sign = 1;
	strcpy(ssign,"Signed");
  } else if(!strcmp(buf,"unsigned")) {
	sign = 0;
	strcpy(ssign,"Unsigned");
  } else{
    perror("illegal sign option read");
    exit(1);
  }
  res=fscanf(cfp, "ENDIAN %3s\n",buf);
  if(!(strcmp(buf,"LSB"))){
  	end = 0;
  } else if(strcmp(buf,"MSB")) {
	end = 1;
  } else{
    perror("illegal endianness option read");
    exit(1);
  }
  strcpy(send,buf);  res=fscanf(cfp,  "WIDTH %u\n",&width);
  res=fscanf(cfp, "HEIGHT %u\n",&height);
  if(bitpix>8) {
	sprintf(dtype,"%s%s%u",ssign,send,(unsigned int)bitpix/8);
 } else if (bitpix==8){
	 (sign==1)?strcpy(dtype,"SignedByte"):strcpy(dtype,"UnsignedByte");
 }
  sprintf(swidth,"%u",width);
  sprintf(sheight,"%u",height);
  res=fscanf(cfp,"NPI %d\n",&npi);
  if (verbose) fprintf(stderr,"n. of processing instructions: %d\n",npi);
  for(i=0;i<npi;i++){
	res=fscanf(cfp,"%5s %s\n",key,pds.xml_model[i]);
	for(j=0;j<strlen(pds.xml_model[i]);j++)		
		if(pds.xml_model[i][j]=='>') pds.xml_model[i][j]=' ';
	if (verbose) fprintf(stderr,"key: %5s  value:%s\n",key,pds.xml_model[i]);
  }
  res=fscanf(cfp,"NPOATTR %d\n",&npoattr);
  if (verbose) fprintf(stderr,"n. of prod. obs. attributes: %d\n",npoattr);
  for(i=0;i<npoattr;i++){
	res=fscanf(cfp,"%15s %s\n",key,po.attributes[i].name);
	res=fscanf(cfp,"%16s %s\n",key,po.attributes[i].value);
	for(j=0;j<strlen(po.attributes[i].value);j++)
		if(po.attributes[i].value[j]=='>') po.attributes[i].value[j]=' ';
	if (verbose) fprintf(stderr,"%16s  %s\n",po.attributes[i].name,po.attributes[i].value);
  }
  if (verbose) fprintf(stderr,"going to read configuration file about Identification_Area\n");
	/*Identification_Area*/
  res=fscanf(cfp,"PROD_LID %s\n",prodlid[0]);
  strcpy(logical_identifier.leaves[0].value,prodlid[0]);
  res=fscanf(cfp,"PROD_VID %s\n",prodvid[0]);
  strcpy(logical_identifier.leaves[1].value,prodlid[0]);
  res=fscanf(cfp,"TITLE %s\n",stitle);
  for(j=0;j<strlen(stitle);j++)
		if(stitle[j]=='>') stitle[j]=' ';
  strcpy(logical_identifier.leaves[2].value,stitle);
  res=fscanf(cfp,"IM_VERS %s\n",imvers);
  strcpy(logical_identifier.leaves[3].value,imvers);
  res=fscanf(cfp,"PROD_CLASS %s\n",prodclass);
  strcpy(logical_identifier.leaves[4].value,prodclass);
  if (verbose) fprintf(stderr,"going to read configuration file about Citation_Information\n");
	/*Citation_Information*/
  res=fscanf(cfp,"NCI %d\n",&nci);
  citation_information.leaves=(struct ELEMENT *)malloc(nci*sizeof(struct ELEMENT));
  for(i=0;i<nci;i++){
	citation_information.leaves[i].leaves=(struct ELEMENT *)malloc(8*sizeof(struct ELEMENT));
	res=fscanf(cfp,"%s %s\n",key,citation_information.leaves[i].leaves[0].value);
	for(j=0;j<strlen(citation_information.leaves[i].leaves[0].value);j++)
		if(citation_information.leaves[i].leaves[0].value[j]=='>') citation_information.leaves[i].leaves[0].value[j]=' ';
	if(verbose)fprintf(stderr,"read author list: %s\n",citation_information.leaves[i].leaves[0].value);
	res=fscanf(cfp,"%s %s\n",key,citation_information.leaves[i].leaves[1].value);
	if(verbose)fprintf(stderr,"read publication year %s\n",citation_information.leaves[i].leaves[1].value);
	res=fscanf(cfp,"%s %s\n",key,citation_information.leaves[i].leaves[2].value);
	if(verbose)fprintf(stderr,"read doi: %s\n",citation_information.leaves[i].leaves[2].value);
	res=fscanf(cfp,"%s %s\n",key,citation_information.leaves[i].leaves[3].value);
	if(verbose)fprintf(stderr,"read key 0: %s\n",citation_information.leaves[i].leaves[3].value);
	res=fscanf(cfp,"%s %s\n",key,citation_information.leaves[i].leaves[4].value);
	if(verbose)fprintf(stderr,"read key 1: %s\n",citation_information.leaves[i].leaves[4].value);
	res=fscanf(cfp,"%s %s\n",key,citation_information.leaves[i].leaves[5].value);
	if(verbose)fprintf(stderr,"read key 2: %s\n",citation_information.leaves[i].leaves[5].value);
	res=fscanf(cfp,"%s %s\n",key,citation_information.leaves[i].leaves[6].value);
	if(verbose)fprintf(stderr,"read key 3: %s\n",citation_information.leaves[i].leaves[6].value);
	res=fscanf(cfp,"%s %s\n",key,citation_information.leaves[i].leaves[7].value);
	for(j=0;j<strlen(citation_information.leaves[i].leaves[7].value);j++)
		if(citation_information.leaves[i].leaves[7].value[j]=='>') citation_information.leaves[i].leaves[7].value[j]=' ';
		if(verbose)fprintf(stderr,"read description: %s\n",citation_information.leaves[i].leaves[7].value);
  }
  if (verbose) fprintf(stderr,"going to read configuration file about Modification_History\n");
  /* Modification_History*/
  res=fscanf(cfp,"MOD_DATE %s\n",moddate);
  res=fscanf(cfp,"VERSID %s\n",svers);
  res=fscanf(cfp,"DESCR %s\n",descr);
	for(j=0;j<strlen(descr);j++)
		if(descr[j]=='>') descr[j]=' ';
  /* Observation Area */
  if (verbose) fprintf(stderr,"going to read configuration file about Observation_Area\n");
  fprintf(stderr,"going to read configuration file about Time_Coordinates\n");
  res=fscanf(cfp,"TSTART %s\n",tstart);
  res=fscanf(cfp,"TSTOP %s\n",tstop);
  fprintf(stderr,"going to read configuration file about Primary_Result_Summary\n");
  res=fscanf(cfp,"PURP %s\n",purp);
  res=fscanf(cfp,"PROC_LEV %s\n",proclev);
  res=fscanf(cfp,"RES_DESCR %s\n",resdescr);
  for(j=0;j<strlen(resdescr);j++)
		if(resdescr[j]=='>') resdescr[j]=' ';
  res=fscanf(cfp,"LAMBDA %s\n",lambda);
  res=fscanf(cfp,"DOMAIN %s\n",domain);
  res=fscanf(cfp,"DISCIPL %s\n",discipl);
  if (verbose) fprintf(stderr,"going to read configuration file about Investigation_Area\n");
  res=fscanf(cfp,"INVEST_AREA %s\n",invarea);
  res=fscanf(cfp,"INVEST_TYPE %s\n",invtype);
  res=fscanf(cfp,"INV_LID %s\n",invlid);
  res=fscanf(cfp,"REFTYPE %s\n",reftype);
  res=fscanf(cfp,"OSNAME %s\n",observing_system.value);
  res=fscanf(cfp,"NOSC_COMP %d\n",&nosc);
  for(i=0;i<nosc;i++){ 
	res=fscanf(cfp,"%11s %s\n",key,onamval[i]);
	for(j=0;j<strlen(onamval[i]);j++)
		if(onamval[i][j]=='>') onamval[i][j]=' ';
	if(verbose)fprintf(stderr,"OSC[%d] n. value: %s\n",i,onamval[i]);
	res=fscanf(cfp,"%s %s\n",key,otypval[i]);
	if(verbose)fprintf(stderr,"OSC[%d] n. type: %s\n",i,otypval[i]);
	res=fscanf(cfp,"%s %s\n",key,observing_system.osc[i].descr);
	for(j=0;j<strlen(observing_system.osc[i].descr);j++)
		if(observing_system.osc[i].descr[j]=='>') observing_system.osc[i].descr[j]=' ';
	if(verbose)fprintf(stderr,"OSC[%d] n. description: %s\n",i,observing_system.osc[i].descr);	
	res=fscanf(cfp,"%s %s\n",key,observing_system.osc[i].ir.lid_reference.value);
	if(verbose)fprintf(stderr,"OSC[%d] n. lid_reference: %s\n",i,observing_system.osc[i].ir.lid_reference.value);	
	res=fscanf(cfp,"%s %s\n",key,observing_system.osc[i].ir.reference_type.value);
	if(verbose)fprintf(stderr,"OSC[%d] n. reference_type: %s\n",i,observing_system.osc[i].ir.reference_type.value);	
  } 
  /* target identification */
	res=fscanf(cfp,"TARGET_NAME %s\n",oa.target[0].name.value); /*  */
	res=fscanf(cfp,"TARGET_TYPE %s\n",oa.target[0].type.value); /*  */
	res=fscanf(cfp,"%s %s\n",key,oa.target[0].iref->lid_reference.value);
	if(verbose)fprintf(stderr,"TARGET[%d] n. lid_reference: %s\n",0,oa.target[0].iref->lid_reference.value);	
	res=fscanf(cfp,"%s %s\n",key,oa.target[0].iref->reference_type.value);
	if(verbose)fprintf(stderr,"TARGET[%d] n. reference_type: %s\n",0,oa.target[0].iref->reference_type.value);	
  /* read Mission Area configuration */
	res=fscanf(cfp,"MISSION %s\n",mission); /* MISSION */
	if(verbose)fprintf(stderr,"MISSION: %s\n",mission);	
	/* START Mission Information */
	res=fscanf(cfp,"MISS_ID %s\n",miss_id); /* MISS_ID (element name) */
	res=fscanf(cfp,"PHASE %s\n",mission_area.leaves[0].leaves[2].value); /* mission phase */
	for(j=0;j<strlen(mission_area.leaves[0].leaves[0].value);j++)
		if(mission_area.leaves[0].leaves[0].value[j]=='>') mission_area.leaves[0].leaves[0].value[j]=' ';	
	res=fscanf(cfp,"CLOCK_START %s\n",mission_area.leaves[0].leaves[0].value); /* clock tstart */
	res=fscanf(cfp,"CLOCK_STOP %s\n",mission_area.leaves[0].leaves[1].value); /* clock tsop */
	res=fscanf(cfp,"PRODID %s\n",mission_area.leaves[0].leaves[3].value); /* product id */
	res=fscanf(cfp,"SWNAME %s\n",mission_area.leaves[0].leaves[4].value); /* software name */
	res=fscanf(cfp,"SWVERS %s\n",mission_area.leaves[0].leaves[5].value); /* software version */
	res=fscanf(cfp,"PHASE_ID %s\n",mission_area.leaves[0].leaves[6].value);/* mission phase id */
	res=fscanf(cfp,"START_ORB %s\n",mission_area.leaves[0].leaves[7].value); /* start orbit */
	res=fscanf(cfp,"STOP_ORB %s\n",mission_area.leaves[0].leaves[8].value); /* stop orbit */
	/* END Mission Information */
	/* START Sub Instrument */
	res=fscanf(cfp,"SU %s\n",su); /* SUB INSTRUMENT (element name) */
	res=fscanf(cfp,"SU_ID %s\n",mission_area.leaves[1].leaves[0].value); /*  */
	res=fscanf(cfp,"SU_NAME %s\n",mission_area.leaves[1].leaves[1].value); /*  */
	for(j=0;j<strlen(mission_area.leaves[1].leaves[1].value);j++)
		if(mission_area.leaves[1].leaves[1].value[j]=='>') mission_area.leaves[1].leaves[1].value[j]=' ';	
	res=fscanf(cfp,"SU_TYPE %s\n",mission_area.leaves[1].leaves[2].value); /*  */
		/* END Sub Instrument */

	/* START Data */
	res=fscanf(cfp,"DATA_NAME %s\n",dname); /* instrument:DATA (element name) */
	/* Data siblings */
	res=fscanf(cfp,"AV_INT %s\n",mission_area.leaves[2].leaves[0].value); /*  */
	res=fscanf(cfp,"CAL_TYPE %s\n",mission_area.leaves[2].leaves[0].value); /*  */
	res=fscanf(cfp,"MEAS_RANGE_IDX %s\n",mission_area.leaves[2].leaves[1].value); /* */
	res=fscanf(cfp,"MEAS_RANGE %s\n",mission_area.leaves[2].leaves[2].value); /*  */
	res=fscanf(cfp,"REF_FRAME %s\n",mission_area.leaves[2].leaves[3].value);/*  */
	res=fscanf(cfp,"SPICE_FRAME %s\n",mission_area.leaves[2].leaves[4].value); /*  */
	/* STOP Data */
	/*START Instrument Status*/
	res=fscanf(cfp,"STATUS_NAME %s\n",istatus); /*  */
	res=fscanf(cfp,"SC_STATUS %s\n",mission_area.leaves[3].leaves[0].value); /*  */
	/*STOP Instrument Status*/
/*
AV_INT 1
CAL_TYPE ground
MEAS_RANGE_IDX 4
MEAS_RANGE 128
REF_FRAME scf
SPICE_FRAME MPO_SPACECRAFT
STATUS_NAME bc_mpo_simbio-sys:Instrument_Status
SC_STATUS orbit

	*/
  /* ended reading configuration file */
  /* printout variables content for debug */
  fprintf(stderr,"BITPIX = %d\n",bitpix);
  fprintf(stderr,"SIGN = %d\n",sign);
  fprintf(stderr,"ENDIAN = %d\n",end);
  fprintf(stderr,"WIDTH = %u\n",width);/* Sample == n. of columns */
  fprintf(stderr,"HEIGHT = %u\n",height); /* Line == n. of rows */
  fprintf(stderr,"PDS4 <data_type> = %s\n",dtype);
  fclose(cfp);

	/* setup data structures and links */
	po.ia=&ia; po.oa=&oa; po.fao=&fao; pds.po=&po;
	if(verbose)fprintf(stderr,"main() starting copying strings\n");
	strcpy((char*)pds.xmlintest,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
/*	strcpy((char*)pds.xml_model[0],"");
	strcpy((char*)pds.xml_model[1],"");
	strcpy((char*)pds.xml_model[2],"");
	strcpy((char*)pds.xml_model[3],"");
	strcpy((char*)pds.xml_model[4],"");
*/
/*	strcpy((char*)pds.xml_model[0],"href=\"https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1A10.sch\"  schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[1],"href=\"https://pds.nasa.gov/pds4/mission/mess/v1/PDS4_MESS_1B00_1020.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
*/
/*	strcpy((char*)pds.xml_model[0],"href=\"https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1L00.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[1],"href=\"https://psa.esa.int/psa/v1/PDS4_PSA_1L00_1401.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[2],"href=\"https://pds.nasa.gov/pds4/geom/v1/PDS4_GEOM_1L00_1970.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[3],"href=\"https://pds.nasa.gov/pds4/disp/v1/PDS4_DISP_1L00_1510.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[4],"href=\"https://psa.esa.int/psa/em16/tgo/cas/v1/PDS4_EM16_TGO_CAS_1L00_1200.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[5],"href=\"https://psa.esa.int/psa/mission/em16/v1/PDS4_EM16_1L00_1300.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
*/	strcpy((char*)po.name,"Product_Observational");
/*	strcpy((char*)po.attributes[0].name,"xmlns");
	strcpy((char*)po.attributes[0].value,"http://pds.nasa.gov/pds4/pds/v1");
	strcpy((char*)po.attributes[1].name,"xmlns:mess");
	strcpy((char*)po.attributes[1].value,"http://pds.nasa.gov/pds4/mission/mess/v1");
	strcpy((char*)po.attributes[2].name,"xmlns:xsi");
	strcpy((char*)po.attributes[2].value,"http://www.w3.org/2001/XMLSchema-instance");
	strcpy((char*)po.attributes[3].name,"xsi:schemaLocation");
	strcpy((char*)po.attributes[3].value,"http://pds.nasa.gov/pds4/pds/v1 https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1A10.xsd   http://pds.nasa.gov/pds4/mission/mess/v1 https://pds.nasa.gov/pds4/mission/mess/v1/PDS4_MESS_1B00_1020.xsd");
	strcpy((char*)po.attributes[0].name,"xsi:schemaLocation");
	strcpy((char*)po.attributes[0].value,"http://pds.nasa.gov/pds4/pds/v1 https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1L00.xsd  http://psa.esa.int/psa/v1  https://psa.esa.int/psa/v1/PDS4_PSA_1L00_1401.xsd  http://pds.nasa.gov/pds4/geom/v1  https://pds.nasa.gov/pds4/geom/v1/PDS4_GEOM_1L00_1970.xsd  http://pds.nasa.gov/pds4/disp/v1  https://pds.nasa.gov/pds4/disp/v1/PDS4_DISP_1L00_1510.xsd  http://psa.esa.int/psa/em16/tgo/cas/v1  https://psa.esa.int/psa/em16/tgo/cas/v1/PDS4_EM16_TGO_CAS_1L00_1200.xsd  http://psa.esa.int/psa/mission/em16/v1  https://psa.esa.int/psa/mission/em16/v1/PDS4_EM16_1L00_1300.xsd");
	strcpy((char*)po.attributes[1].name,"xmlns");
	strcpy((char*)po.attributes[1].value,"http://pds.nasa.gov/pds4/pds/v1");
	strcpy((char*)po.attributes[2].name,"xmlns:em16_tgo_cas");
	strcpy((char*)po.attributes[2].value,"http://psa.esa.int/psa/em16/tgo/cas/v1");
	strcpy((char*)po.attributes[3].name,"xmlns:geom");
	strcpy((char*)po.attributes[3].value,"http://pds.nasa.gov/pds4/geom/v1");
	strcpy((char*)po.attributes[4].name,"xmlns:disp");
	strcpy((char*)po.attributes[4].value,"http://pds.nasa.gov/pds4/disp/v1");
	strcpy((char*)po.attributes[5].name,"xmlns:psa");
	strcpy((char*)po.attributes[5].value,"http://psa.esa.int/psa/v1");
	strcpy((char*)po.attributes[6].name,"xmlns:xsi");
	strcpy((char*)po.attributes[6].value,"http://www.w3.org/2001/XMLSchema-instance");
*/	strcpy((char*)ia.name,"Identification_Area");
	strcpy((char*)oa.name,"Observation_Area");
	strcpy((char*)fao.name,"File_Area_Observational");
	strcpy((char*)logical_identifier.name,"logical_identifier");
	strcpy((char*)logical_identifier.value, prodlid[0]);
	strcpy((char*)version_id.name,"version_id");
	strcpy((char*)version_id.value,prodvid[0]);
	strcpy((char*)title.name,"title");
	strcpy((char*)title.value,stitle);
	strcpy((char*)information_model_version.name,"information_model_version");
	strcpy((char*)information_model_version.value,imvers);
	strcpy((char*)product_class.name,"product_class");
	strcpy((char*)product_class.value,prodclass);
	/*Citation_Information*/
	if(verbose)fprintf(stderr,"main() before Citation_Information\n");
	strcpy((char*)citation_information.name,"Citation_Information");
	for(i=0;i<nci;i++){ 
		strcpy((char*)citation_information.leaves[i].leaves[0].name,"author_list"); /* author list */
		strcpy((char*)citation_information.leaves[i].leaves[1].name,"publication_year"); /* publication year */
		strcpy((char*)citation_information.leaves[i].leaves[2].name,"doi"); /* doi */
		strcpy((char*)citation_information.leaves[i].leaves[3].name,"keyword"); /* keyword 0 */
		strcpy((char*)citation_information.leaves[i].leaves[4].name,"keyword"); /* keyword 1 */
		strcpy((char*)citation_information.leaves[i].leaves[5].name,"keyword"); /* keyword 2 */
		strcpy((char*)citation_information.leaves[i].leaves[6].name,"keyword"); /* keyword 3 */
		strcpy((char*)citation_information.leaves[i].leaves[7].name,"description"); /* cit. inf. description */
	}
	if(verbose)fprintf(stderr,"main() before Modification_History\n");
	strcpy((char*)modification_history.name,"Modification_History");
	strcpy((char*)modification_history.leaves[0].name,"Modification_Detail");
	strcpy((char*)modification_history.leaves[0].leaves[0].name,"modification_date");
	strcpy((char*)modification_history.leaves[0].leaves[0].value,moddate);
	strcpy((char*)modification_history.leaves[0].leaves[1].name,"version_id");
	strcpy((char*)modification_history.leaves[0].leaves[1].value,svers);
	strcpy((char*)modification_history.leaves[0].leaves[2].name,"description");
	strcpy((char*)modification_history.leaves[0].leaves[2].value,descr);
    if(verbose)fprintf(stderr,"main() after modification_history\n");
	strcpy((char*)timecoord.name,"Time_Coordinates");
	strcpy((char*)timecoord.tstart.name,"start_date_time");
	strcpy((char*)timecoord.tstart.value,tstart);
	strcpy((char*)timecoord.tstop.name,"stop_date_time");
	strcpy((char*)timecoord.tstop.value,tstop);
	strcpy((char*)primary_result_summary.name,"Primary_Result_Summary");
	strcpy((char*)primary_result_summary.leaves[0].name,"purpose");
	strcpy((char*)primary_result_summary.leaves[0].value,purp);
	strcpy((char*)primary_result_summary.leaves[1].name,"processing_level");
	strcpy((char*)primary_result_summary.leaves[1].value,proclev);
	strcpy((char*)primary_result_summary.leaves[2].name,"description");
	strcpy((char*)primary_result_summary.leaves[2].value,resdescr);
	strcpy((char*)primary_result_summary.leaves[3].name,"Science_Facets");
	strcpy((char*)primary_result_summary.leaves[3].leaves[0].name,"wavelength_range");
	strcpy((char*)primary_result_summary.leaves[3].leaves[0].value,lambda);
	strcpy((char*)primary_result_summary.leaves[3].leaves[1].name,"domain");
	strcpy((char*)primary_result_summary.leaves[3].leaves[1].value,domain);
	strcpy((char*)primary_result_summary.leaves[3].leaves[2].name,"discipline_name");
	strcpy((char*)primary_result_summary.leaves[3].leaves[2].value,discipl);
    if(verbose)fprintf(stderr,"main() before Observing System\n");	
/*	strcpy((char*)oa.name,"Observing System"); */
	strcpy((char*)observing_system.name,"Observing_System");
	strcpy((char*)observing_system.leaves[0].name,"name");
	strcpy((char*)observing_system.osc[0].ename,"Observing_System_Component");
	strcpy((char*)observing_system.osc[0].name->name,"name");
	strcpy((char*)observing_system.osc[0].name->value,onamval[0]); 
	strcpy((char*)observing_system.osc[0].type->name,"type");
	strcpy((char*)observing_system.osc[0].type->value,otypval[0]);
	strcpy((char*)observing_system.osc[0].ir.name,"Internal_Reference"); 
	strcpy((char*)observing_system.osc[0].ir.lid_reference.name,"lid_reference");
	strcpy((char*)observing_system.osc[0].ir.reference_type.name,"reference_type");
	strcpy((char*)observing_system.osc[1].ename,"Observing_System_Component");
	strcpy((char*)observing_system.osc[1].name->name,"name");
	strcpy((char*)observing_system.osc[1].name->value,onamval[1]); 
	strcpy((char*)observing_system.osc[1].type->name,"type");
	strcpy((char*)observing_system.osc[1].type->value,otypval[1]); 
	strcpy((char*)observing_system.osc[1].ir.name,"Internal_Reference"); 
	strcpy((char*)observing_system.osc[1].ir.lid_reference.name,"lid_reference");
	strcpy((char*)observing_system.osc[1].ir.reference_type.name,"reference_type");
    if(verbose)fprintf(stderr,"main() before Target_Identification\n");	
	strcpy((char*)oa.target[0].ename,"Target_Identification");
	strcpy((char*)oa.target[0].name.name,"name");
//	strcpy((char*)oa.target[0].name.value,"mercury");
	strcpy((char*)oa.target[0].type.name,"type");
//	strcpy((char*)oa.target[0].type.value,"Planet");
	strcpy((char*)oa.target[0].iref->name,"Internal_Reference"); 
	strcpy((char*)oa.target[0].iref->lid_reference.name,"lid_reference");
	strcpy((char*)oa.target[0].iref->reference_type.name,"reference_type");
	strcpy((char*)mission_area.name,"Mission_Area");
	if (verbose) fprintf(stderr,"strcmp(mission,\"mess:MESSENGER\"): %d\n",strcmp(mission,"mess:MESSENGER"));
	if (!strcmp(mission,"mess:MESSENGER") ){
		strcpy((char*)mission_area.leaves[0].name,"mess:MESSENGER");
		strcpy((char*)mission_area.leaves[0].leaves[0].name,"mess:mission_phase_name");
		strcpy((char*)mission_area.leaves[0].leaves[0].value,"Mercury Orbit Year 5");
		strcpy((char*)mission_area.leaves[0].leaves[1].name,"mess:spacecraft_clock_start_count");
		strcpy((char*)mission_area.leaves[0].leaves[1].value,"2/070170476");
		strcpy((char*)mission_area.leaves[0].leaves[2].name,"mess:spacecraft_clock_stop_count");
		strcpy((char*)mission_area.leaves[0].leaves[2].value,"2/070256656");
		strcpy((char*)mission_area.leaves[0].leaves[3].name,"mess:standard_data_product_id");
		strcpy((char*)mission_area.leaves[0].leaves[3].value,"mdisedr");
		strcpy((char*)mission_area.leaves[0].leaves[4].name,"mess:software_name");
		strcpy((char*)mission_area.leaves[0].leaves[4].value,"pipe-mdis2edr");
		strcpy((char*)mission_area.leaves[0].leaves[5].name,"mess:software_version_id");
		strcpy((char*)mission_area.leaves[0].leaves[5].value,"1.1");
//	 } else if (!strcmp(mission,"BC:BEPICOLOMBO")){
	 } else {
	if (verbose) fprintf(stderr,"strcmp(mission,\"BC:BEPICOLOMBO\"): %d\n",strcmp(mission,"BC:BEPICOLOMBO"));
	 	strcpy((char*)mission_area.leaves[0].name,miss_id);
		strcpy((char*)mission_area.leaves[0].leaves[2].name,"psa:mission_phase_name");
//		strcpy((char*)mission_area.leaves[0].leaves[0].value,"Mercury Orbit Year 5");
		strcpy((char*)mission_area.leaves[0].leaves[0].name,"psa:spacecraft_clock_start_count");
//		strcpy((char*)mission_area.leaves[0].leaves[1].value,"2/070170476");
		strcpy((char*)mission_area.leaves[0].leaves[1].name,"psa:spacecraft_clock_stop_count");
//		strcpy((char*)mission_area.leaves[0].leaves[2].value,"2/070256656");
		strcpy((char*)mission_area.leaves[0].leaves[3].name,"psa:standard_data_product_id");
//		strcpy((char*)mission_area.leaves[0].leaves[3].value,"mdisedr");
		strcpy((char*)mission_area.leaves[0].leaves[4].name,"psa:software_name");
//		strcpy((char*)mission_area.leaves[0].leaves[4].value,"pipe-mdis2edr");
		strcpy((char*)mission_area.leaves[0].leaves[5].name,"psa:software_version_id");
//		strcpy((char*)mission_area.leaves[0].leaves[5].value,"1.1");
		strcpy((char*)mission_area.leaves[0].leaves[6].name,"psa:mission_phase_identifier");
//		strcpy((char*)mission_area.leaves[0].leaves[6].value,"1.1");
		strcpy((char*)mission_area.leaves[0].leaves[7].name,"psa:start_orbit_number");
//		strcpy((char*)mission_area.leaves[0].leaves[7].value,"1.1");
		strcpy((char*)mission_area.leaves[0].leaves[8].name,"psa:stop_orbit_number");
//		strcpy((char*)mission_area.leaves[0].leaves[8].value,"1.1");
	 	strcpy((char*)mission_area.leaves[1].name,su); /* Sub Instrument */
		strcpy((char*)mission_area.leaves[1].leaves[0].name,"psa:identifier");
//		strcpy((char*)mission_area.leaves[1].leaves[0].value,"Mercury Orbit Year 5");
		strcpy((char*)mission_area.leaves[1].leaves[1].name,"psa:name");
//		strcpy((char*)mission_area.leaves[1].leaves[1].value,"2/070170476");
		strcpy((char*)mission_area.leaves[1].leaves[2].name,"psa:type");
//		strcpy((char*)mission_area.leaves[1].leaves[2].value,"2/070256656");
	 	strcpy((char*)mission_area.leaves[2].name,dname); /* Data */
		strcpy((char*)mission_area.leaves[2].leaves[0].name,"bc_mpo_sim:averaging_interval");
//		strcpy((char*)mission_area.leaves[2].leaves[0].value,"");
		strcpy((char*)mission_area.leaves[2].leaves[0].attributes[0].name,"unit");
		strcpy((char*)mission_area.leaves[2].leaves[0].attributes[0].value,"s");

		strcpy((char*)mission_area.leaves[2].leaves[1].name,"bc_mpo_sim:calibration_type");
//		strcpy((char*)mission_area.leaves[2].leaves[1].value,"");
		strcpy((char*)mission_area.leaves[2].leaves[2].name,"bc_mpo_sim:measurement_range_index");
//		strcpy((char*)mission_area.leaves[2].leaves[2].value,"");
		strcpy((char*)mission_area.leaves[2].leaves[3].name,"bc_mpo_sim:measurement_range");
//		strcpy((char*)mission_area.leaves[2].leaves[3].value,"");
		strcpy((char*)mission_area.leaves[2].leaves[4].name,"bc_mpo_sim:data_reference_frame_acronym");
//		strcpy((char*)mission_area.leaves[2].leaves[4].value,"");
		strcpy((char*)mission_area.leaves[2].leaves[5].name,"bc_mpo_sim:data_reference_spice_frame");
//		strcpy((char*)mission_area.leaves[2].leaves[5].value,"");
	 	strcpy((char*)mission_area.leaves[3].name,istatus);
		strcpy((char*)mission_area.leaves[3].leaves[0].name,"bc_mpo_sim:mpo_status");
//		strcpy((char*)mission_area.leaves[3].leaves[0].value,"");
	 	strcpy((char*)mission_area.leaves[4].name,"psa:Processing_Context");
		strcpy((char*)mission_area.leaves[4].leaves[0].name,"psa:processing_software_title");
		strcpy((char*)mission_area.leaves[4].leaves[0].value,(char*)mission_area.leaves[0].leaves[4].value);
		strcpy((char*)mission_area.leaves[4].leaves[1].name,"psa:processing_software_version");
		strcpy((char*)mission_area.leaves[4].leaves[1].value,(char*)mission_area.leaves[0].leaves[5].value);
	 }
	/* Discipline_Area*/
	strcpy((char*)discipline_area.name,"Discipline_Area");
	strcpy((char*)discipline_area.leaves[0].name,"geom:Geometry");
	strcpy((char*)discipline_area.leaves[0].leaves[0].name,"geom:SPICE_Kernel_Files");
	strcpy((char*)discipline_area.leaves[0].leaves[0].leaves[0].name,"geom:SPICE_Kernel_Identification");
	for(i=0;i<251;i++){
		strcpy((char*)discipline_area.leaves[0].leaves[0].leaves[0].leaves[i].name,"geom:spice_kernel_file_name");
		strcpy((char*)discipline_area.leaves[0].leaves[0].leaves[0].leaves[i].value,"TBC");
	}
    if(verbose)fprintf(stderr,"main() before Investigation Area\n");	
	strcpy((char*)iaa.ename,"Investigation_Area");
	strcpy((char*)iaa.name.name,"name");
	strcpy((char*)iaa.name.value,invarea);
	strcpy((char*)iaa.type.name,"type");
	strcpy((char*)iaa.type.value,invtype);
    if(verbose)fprintf(stderr,"main() before Internal_Reference\n");	
	strcpy((char*)iref.name,"Internal_Reference");
	strcpy((char*)iref.lid_reference.name,"lid_reference");
	strcpy((char*)iref.lid_reference.value,invlid);
	strcpy((char*)iref.reference_type.name,"reference_type");
	strcpy((char*)iref.reference_type.value,reftype);
	strcpy((char*)file.name,"File");
	strcpy((char*)file.leaves[0].name,"file_name");
	strcpy((char*)file.leaves[0].value,prodfnam[0]);
	if(verbose)fprintf(stderr,"main() before Array_2D_Image\n");	
	strcpy((char*)array2d.name,"Array_2D_Image");
	strcpy((char*)array2d.leaves[0].name,"offset"); 
	strcpy((char*)array2d.leaves[0].attributes->name,"unit"); 
	strcpy((char*)array2d.leaves[0].attributes->value,"byte");
	strcpy((char*)array2d.leaves[0].value,"0");
	strcpy((char*)array2d.leaves[1].name,"axes");
	strcpy((char*)array2d.leaves[1].value,"2");
	strcpy((char*)array2d.leaves[2].name,"axis_index_order");
	strcpy((char*)array2d.leaves[2].value,"Last Index Fastest");
	strcpy((char*)array2d.leaves[3].name,"Element_Array");
	strcpy((char*)array2d.leaves[3].leaves[0].name,"data_type");
	strcpy((char*)array2d.leaves[3].leaves[0].value,dtype); /* dtype from configuration file */
	strcpy((char*)array2d.leaves[4].name,"Axis_Array");
	strcpy((char*)array2d.leaves[4].leaves[0].name,"axis_name");
	strcpy((char*)array2d.leaves[4].leaves[0].value,"Line");
	strcpy((char*)array2d.leaves[4].leaves[1].name,"elements");
	strcpy((char*)array2d.leaves[4].leaves[1].value,sheight);
	strcpy((char*)array2d.leaves[4].leaves[2].name,"sequence_number");
	strcpy((char*)array2d.leaves[4].leaves[2].value,"1");
	strcpy((char*)array2d.leaves[5].name,"Axis_Array");
	strcpy((char*)array2d.leaves[5].leaves[0].name,"axis_name");
	strcpy((char*)array2d.leaves[5].leaves[0].value,"Sample");
	strcpy((char*)array2d.leaves[5].leaves[1].name,"elements");
	strcpy((char*)array2d.leaves[5].leaves[1].value,swidth);
	strcpy((char*)array2d.leaves[5].leaves[2].name,"sequence_number");
	strcpy((char*)array2d.leaves[5].leaves[2].value,"2");
	lp=stdout; 	pds.pfnames=prodfnam;
	oa.times=&timecoord;
	/*oa.leaves[0]=observing_system;*/
	if (init_pds(lp,&pds, prodfnam,argv)) perror("error in initing pds product archiving"); 
        if(verbose)fprintf(stderr,"main() after init_pds()\n");
    LIBXML_TEST_VERSION;

	doc = xmlNewDoc(BAD_CAST "1.0");

    if(verbose)fprintf(stderr,"main() opening <Product_Observational> Root node element step 0 \n");
    root_node = xmlNewNode(NULL, BAD_CAST po.name);    
    xmlDocSetRootElement(doc, root_node);
    attribute = root_node->properties;
    for(i=0;i<npoattr;i++)
		attr = xmlSetProp(root_node, (const xmlChar *)po.attributes[i].name,(const xmlChar *)po.attributes[i].value); /* product observational attribute */
    for(i=0;i<npi;i++){
      // Create the xml-model processing instruction
      pi = xmlNewPI(BAD_CAST "xml-model", BAD_CAST pds.xml_model[i]);
      // Add it before the root element
      xmlAddPrevSibling(root_node, pi);
    }
    if(verbose)fprintf(stderr,"main() opening <Identification_Area> node element step 1 \n");
    xmlNewChild(root_node, NULL, BAD_CAST pds.po->ia->name, BAD_CAST NULL); /* Identification_Area node element */
    p1=root_node->children;
    xmlNewChild(p1, NULL, BAD_CAST logical_identifier.name, BAD_CAST logical_identifier.value);
    xmlNewChild(p1, NULL, BAD_CAST version_id.name, BAD_CAST version_id.value);
    xmlNewChild(p1, NULL, BAD_CAST title.name, BAD_CAST title.value);
    xmlNewChild(p1, NULL, BAD_CAST information_model_version.name, BAD_CAST information_model_version.value);
    xmlNewChild(p1, NULL, BAD_CAST product_class.name, BAD_CAST product_class.value);
    /* Citation Information subtree */
    if(verbose)fprintf(stderr,"main() opening <Citation_Information> node element 1.15\n");
    p1=xmlNewChild(p1, NULL, BAD_CAST citation_information.name, BAD_CAST NULL);
    for(i=0;i<nci;i++){
	//citation_information.leaves[i].leaves
	xmlNewChild(p1, NULL, BAD_CAST citation_information.leaves[i].leaves[0].name, BAD_CAST citation_information.leaves[i].leaves[0].value);
	xmlNewChild(p1, NULL, BAD_CAST citation_information.leaves[i].leaves[1].name, BAD_CAST citation_information.leaves[i].leaves[1].value);
	xmlNewChild(p1, NULL, BAD_CAST citation_information.leaves[i].leaves[2].name, BAD_CAST citation_information.leaves[i].leaves[2].value);
	xmlNewChild(p1, NULL, BAD_CAST citation_information.leaves[i].leaves[3].name, BAD_CAST citation_information.leaves[i].leaves[3].value);
	xmlNewChild(p1, NULL, BAD_CAST citation_information.leaves[i].leaves[4].name, BAD_CAST citation_information.leaves[i].leaves[4].value);
	xmlNewChild(p1, NULL, BAD_CAST citation_information.leaves[i].leaves[5].name, BAD_CAST citation_information.leaves[i].leaves[5].value);
	xmlNewChild(p1, NULL, BAD_CAST citation_information.leaves[i].leaves[6].name, BAD_CAST citation_information.leaves[i].leaves[6].value);
	xmlNewChild(p1, NULL, BAD_CAST citation_information.leaves[i].leaves[7].name, BAD_CAST citation_information.leaves[i].leaves[7].value);
	}    
	p1=p1->parent;
	/* Modification History subtree */
    if(verbose)fprintf(stderr,"main() opening <Modification_History> node element 1.25\n");
    p1=xmlNewChild(p1, NULL, BAD_CAST modification_history.name, BAD_CAST NULL);
    p1=xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0].name, BAD_CAST NULL);
    xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0].leaves[0].name, BAD_CAST modification_history.leaves[0].leaves[0].value);
    xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0].leaves[1].name, BAD_CAST modification_history.leaves[0].leaves[1].value);
    xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0].leaves[2].name, BAD_CAST modification_history.leaves[0].leaves[2].value);
    p1=p1->parent;p1=p1->parent;p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Observation_Area> node element 1.5\n");
    p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->oa->name, BAD_CAST NULL); /* Observation_Area node element */
	p1 = xmlNewChild(p1, NULL, BAD_CAST timecoord.name, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST timecoord.tstart.name, BAD_CAST timecoord.tstart.value);
	xmlNewChild(p1, NULL, BAD_CAST timecoord.tstop.name, BAD_CAST timecoord.tstop.value);
	p1=p1->parent;
    if(verbose)fprintf(stderr,"main() Primary_Result_Summary node element 2.15\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST primary_result_summary.name, BAD_CAST NULL); /* Primary_Result_Summary */
	xmlNewChild(p1, NULL, BAD_CAST primary_result_summary.leaves[0].name, BAD_CAST primary_result_summary.leaves[0].value); /* purpose */
	xmlNewChild(p1, NULL, BAD_CAST primary_result_summary.leaves[1].name, BAD_CAST primary_result_summary.leaves[1].value); /* processing_level */
	xmlNewChild(p1, NULL, BAD_CAST primary_result_summary.leaves[2].name, BAD_CAST primary_result_summary.leaves[2].value); /* description */
	p1 = xmlNewChild(p1, NULL, BAD_CAST primary_result_summary.leaves[3].name, BAD_CAST NULL); /* Science_Facets */
	xmlNewChild(p1, NULL, BAD_CAST primary_result_summary.leaves[3].leaves[0].name, BAD_CAST primary_result_summary.leaves[3].leaves[0].value); /* wavelength_range */
	xmlNewChild(p1, NULL, BAD_CAST primary_result_summary.leaves[3].leaves[1].name, BAD_CAST primary_result_summary.leaves[3].leaves[1].value); /* domain */
	xmlNewChild(p1, NULL, BAD_CAST primary_result_summary.leaves[3].leaves[2].name, BAD_CAST primary_result_summary.leaves[3].leaves[2].value); /* discipline_name */
	p1=p1->parent;p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Investigation_Area>  node element 2.25\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST iaa.ename, BAD_CAST NULL); /* Investigation_Area node element */
	xmlNewChild(p1, NULL, BAD_CAST iaa.name.name, BAD_CAST iaa.name.value); /* IA name */
	xmlNewChild(p1, NULL, BAD_CAST iaa.type.name, BAD_CAST iaa.type.value); /* IA type */
	p1=xmlNewChild(p1, NULL, BAD_CAST iref.name, BAD_CAST NULL); /* IA.Internal_Reference */
	xmlNewChild(p1, NULL, BAD_CAST iref.lid_reference.name, BAD_CAST iref.lid_reference.value);
	xmlNewChild(p1, NULL, BAD_CAST iref.reference_type.name, BAD_CAST iref.reference_type.value);
	p1=p1->parent;p1=p1->parent; /* close IA */
    if(verbose)fprintf(stderr,"main() opening <Observing_System> 2.5\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.name, BAD_CAST NULL); /* <Observing_System>*/
//	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.leaves[0].name, BAD_CAST BAD_CAST observing_system.leaves[0].value); //<Observing_System_Component>
	for(i=0;i<nosc;i++){
	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].ename, BAD_CAST NULL); //<Observing_System_Component>
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].name->name, BAD_CAST observing_system.osc[i].name->value);
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].type->name, BAD_CAST observing_system.osc[i].type->value);
	xmlNewChild(p1, NULL, BAD_CAST "description", BAD_CAST observing_system.osc[i].descr);	
	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].ir.name, BAD_CAST NULL); /* IA.Internal_Reference */
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].ir.lid_reference.name, BAD_CAST observing_system.osc[i].ir.lid_reference.value);
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].ir.reference_type.name, BAD_CAST observing_system.osc[i].ir.reference_type.value);
	p1=p1->parent;p1=p1->parent;
	} 
	p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Target_Identification> 3\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST oa.target[0].ename, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].name.name, BAD_CAST oa.target[0].name.value);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].type.name, BAD_CAST oa.target[0].type.value);
	p1=xmlNewChild(p1, NULL, BAD_CAST oa.target[0].iref->name, BAD_CAST NULL); /* IA.Internal_Reference */
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].iref->lid_reference.name, BAD_CAST oa.target[0].iref->lid_reference.value);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].iref->reference_type.name, BAD_CAST oa.target[0].iref->reference_type.value);
	p1=p1->parent;p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Mission_Area> 3.25\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.name, BAD_CAST NULL); /* mission area */
	if (!strcmp(mission,"mess:MESSENGER")) {
	p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].name, BAD_CAST NULL); /* mess:MESSENGER */
	xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[0].name, BAD_CAST mission_area.leaves[0].leaves[0].value);
	xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[1].name, BAD_CAST mission_area.leaves[0].leaves[1].value);
	xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[2].name, BAD_CAST mission_area.leaves[0].leaves[2].value);
	xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[3].name, BAD_CAST mission_area.leaves[0].leaves[3].value);
	xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[4].name, BAD_CAST mission_area.leaves[0].leaves[4].value);
	xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[5].name, BAD_CAST mission_area.leaves[0].leaves[5].value); 
	p1=p1->parent;p1=p1->parent;p1=p1->parent;
	} else if (!strcmp(mission,"BC:BEPICOLOMBO")) {
//	} else  {
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].name, BAD_CAST NULL); /* psa:Mission_Information */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[0].name, BAD_CAST mission_area.leaves[0].leaves[0].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[1].name, BAD_CAST mission_area.leaves[0].leaves[1].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[2].name, BAD_CAST mission_area.leaves[0].leaves[2].value);
//		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[6].name, BAD_CAST mission_area.leaves[0].leaves[6].value);
		xmlNewChild(p1, NULL, BAD_CAST "psa:mission_phase_identifier", BAD_CAST mission_area.leaves[0].leaves[6].value);
		xmlNewChild(p1, NULL, BAD_CAST "psa:start_orbit_number", BAD_CAST mission_area.leaves[0].leaves[7].value);
		xmlNewChild(p1, NULL, BAD_CAST "psa:stop_orbit_number", BAD_CAST mission_area.leaves[0].leaves[8].value);
		p1=p1->parent;
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[1].name, BAD_CAST NULL); /* psa:Sub-Instrument */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[1].leaves[0].name, BAD_CAST mission_area.leaves[1].leaves[0].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[1].leaves[1].name, BAD_CAST mission_area.leaves[1].leaves[1].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[1].leaves[2].name, BAD_CAST mission_area.leaves[1].leaves[2].value);
		p1=p1->parent;
		//p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].name, BAD_CAST NULL); /* bc_mpo_sim:Data */
		//p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[0].name, BAD_CAST mission_area.leaves[2].leaves[0].value);
		//attr = xmlSetProp(p1, (const xmlChar *)mission_area.leaves[2].leaves[0].attributes[0].name, \
		//(const xmlChar *)mission_area.leaves[2].leaves[0].attributes[0].value); /* unit attribute */		
		//p1=p1->parent;
		//xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[1].name, BAD_CAST mission_area.leaves[2].leaves[1].value);
		//xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[2].name, BAD_CAST mission_area.leaves[2].leaves[2].value);
		//xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[3].name, BAD_CAST mission_area.leaves[2].leaves[3].value);
		//xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[4].name, BAD_CAST mission_area.leaves[2].leaves[4].value);
		//xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[5].name, BAD_CAST mission_area.leaves[2].leaves[5].value);
		//p1=p1->parent;
		//p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[3].name, BAD_CAST NULL); /* bc_mpo_sim:Instrument_Status */
		//xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[3].leaves[0].name, BAD_CAST mission_area.leaves[3].leaves[0].value);
		//p1=p1->parent;
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].name, BAD_CAST NULL); /* Processing Context */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[0].name, BAD_CAST mission_area.leaves[4].leaves[0].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[1].name, BAD_CAST mission_area.leaves[4].leaves[1].value);
		p1=p1->parent;p1=p1->parent;p1=p1->parent;	
	}
	/* Discipline_Area */
	
	//p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.name, BAD_CAST NULL); /* <Discipline_Area> */
	//p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.leaves[0].name, BAD_CAST NULL); /* <geom:Geometry> */
	//p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.leaves[0].leaves[0].name, BAD_CAST NULL);/*geom:SPICE_Kernel_Files*/
	//p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.leaves[0].leaves[0].leaves[0].name, BAD_CAST NULL);/*<SKIdent>*/
	//for(i=0;i<251;i++){
		//p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.leaves[0].leaves[0].leaves[0].leaves[i].name, BAD_CAST \
		//discipline_area.leaves[0].leaves[0].leaves[0].leaves[i].value);
		//p1=p1->parent;
	//}
	
    if(verbose)fprintf(stderr,"main() opening <File_Area_Observational> node element step 3.5\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->fao->name, BAD_CAST NULL); /* File_Area_Observational node element */
	p1 = xmlNewChild(p1, NULL, BAD_CAST file.name, BAD_CAST NULL); /* opening <File> element */
	xmlNewChild(p1, NULL, BAD_CAST file.leaves[0].name, BAD_CAST file.leaves[0].value); /* <file_name> element */
	p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.name, BAD_CAST NULL); /* opening <Array_2D_Image> node */
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[0].name, BAD_CAST array2d.leaves[0].value); /* offset */
	attr = xmlSetProp(p1, (const xmlChar *)array2d.leaves[0].attributes[0].name,(const xmlChar *)array2d.leaves[0].attributes[0].value); /* offset attribute */
	p1=p1->parent;
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[1].name, BAD_CAST array2d.leaves[1].value); /*(n. of) <axes> */
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[2].name, BAD_CAST array2d.leaves[2].value); /*<axis_index_order>*/
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[3].name, BAD_CAST NULL);/*<Element_Array>*/
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[3].leaves[0].name, BAD_CAST array2d.leaves[3].leaves[0].value);/*<data_type>*/
	p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Axis_Array> Line/height/y step 3.7\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4].name, BAD_CAST NULL);/*Line/height/y <Axis_Array>*/
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4].leaves[0].name, BAD_CAST array2d.leaves[4].leaves[0].value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4].leaves[1].name, BAD_CAST array2d.leaves[4].leaves[1].value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4].leaves[2].name, BAD_CAST array2d.leaves[4].leaves[2].value);
	p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Axis_Array> Sample/width/x step 4\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[5].name, BAD_CAST NULL);/*Sample/width/x <Axis_Array>*/
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[5].leaves[0].name, BAD_CAST array2d.leaves[5].leaves[0].value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[5].leaves[1].name, BAD_CAST array2d.leaves[5].leaves[1].value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[5].leaves[2].name, BAD_CAST array2d.leaves[5].leaves[2].value);
	p1=p1->parent;p1=p1->parent;p1=p1->parent;p1=p1->parent;
    /*
 *      * Dumping document to stdio or file
 *           
    xmlSaveFormatFileEnc(argc > 1 ? argv[1] : "-", doc, "UTF-8", 1);
*/
    fprintf(stderr,"main() ended xml structure before saving xml file step 5\n");
    xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
    xmlFreeDoc(doc);

    fprintf(stderr,"main() xml file save, freeing resources step 6\n");
    xmlCleanupParser();

	for(i=0;i<numproducts;i++){
		free(prodfnam[i]);
		free(pds.products[i]);
	}
	free(array2d.leaves[0].attributes);
	free(pds.products);
	free(primary_result_summary.leaves[3].leaves);
	free(primary_result_summary.leaves);
	free(citation_information.leaves[0].leaves);
	free(citation_information.leaves);
	free(logical_identifier.leaves[0].leaves);
	free(logical_identifier.leaves);
	free(modification_history.leaves[0].leaves);
	free(modification_history.leaves);
	free(prodfnam);
	free(observing_system.leaves);
	for(i=0;i<nosc;i++){
		free(observing_system.osc[i].name);
		free(observing_system.osc[i].type);
	}
	free(discipline_area.leaves[0].leaves[0].leaves[0].leaves);
	free(discipline_area.leaves[0].leaves[0].leaves);
	free(discipline_area.leaves[0].leaves);
	free(discipline_area.leaves);
	free(file.leaves);
	free(mission_area.leaves[2].leaves[0].attributes);
	for(i=0;i<5;i++)
		free(mission_area.leaves[i].leaves);
	free(mission_area.leaves);
	for(i=0;i<6;i++)
		free(array2d.leaves[i].leaves);
	// prodfname freed 
	free(oa.target->iref);
	free(oa.target);
	free(po.attributes); // po.attributes freed 
	fprintf(stderr,"main() ending program\n");
  return 0;
}
