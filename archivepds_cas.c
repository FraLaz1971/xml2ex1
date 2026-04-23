#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/tree.h>
#include "pds.h"
#include <stdlib.h>

int main(int argc, char **argv)
{	int verbose = 1;
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
    unsigned int width = 0;  /* Sample == n. of columns */
    unsigned int height = 0; /* Line == n. of rows */
    const char* confname;
    int j;
    char buf[1024], key[256], val[1024];
    FILE *cfp;
    int i, res, numproducts, npi, npoattr, nosc, nci, nrlir;
	/* npi = n. of processing instructions
	 * numproducts = n. of products
	 * npoattr = n. of <Product_Observational> attributes
	 * nosc = n. of OBSERVING_SYSTEM_COMPONENTs
	 * nci = n. of citation_information
	 * nrlir = n. of reference_list internal reference items
	 */
	struct PDS pds; struct PRODUCT_OBSERVATIONAL po; struct IDENTIFICATION_AREA ia;
	struct OBSERVATION_AREA oa;
	struct FILE_AREA_OBSERVATIONAL fao;
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
	struct REFERENCE_LIST reference_list;
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
		fprintf(stderr,"e.g: %s arpds.conf 1 image.raw \n",argv[0]);
		return 1;
	}
	confname=argv[1];
	numproducts = atoi(argv[2]); /* number of products pointed by the label */
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
	mission_area.leaves=(struct ELEMENT *)malloc(MAXLEAV*sizeof(struct ELEMENT));
	for(i=0;i<MAXLEAV;i++)
		mission_area.leaves[i].leaves=(struct ELEMENT *)malloc(MAXLEAV*sizeof(struct ELEMENT));/* Mission_Area               -->mission_area
																									psa:Mission_Information  -->mission_area.leaves[0]
																										psa:Mission_Phase    -->mission_area.leaves[0].leaves[0]
																											psa:name      -->mission_area.leaves[0].leaves[0].leaves[0]
																											psa:id        -->mission_area.leaves[0].leaves[0].leaves[1]
																									psa:Sub-Instrument    -->mission_area.leaves[1]
																									psa:Observation_Context  -->mission_area.leaves[2]
																									psa:Processing_Context   -->mission_area.leaves[3]
																									em16_tgo_cas:Cassis_Data -->mission_area.leaves[4]
		*/

	mission_area.leaves[0].leaves[0].leaves=(struct ELEMENT *)malloc(MAXLEAV*sizeof(struct ELEMENT)); /* psa:Mission_Phase */
	mission_area.leaves[0].leaves[0].leaves[0].leaves=(struct ELEMENT *)malloc(MAXLEAV*sizeof(struct ELEMENT)); /* psa:name
																											 psa:id   */
	mission_area.leaves[2].leaves[0].attributes=(struct ATTRIBUTE *)malloc(MAXLEAV*sizeof(struct ATTRIBUTE));

	for(i=0;i<MAXLEAV;i++)
		mission_area.leaves[4].leaves[i].leaves=(struct ELEMENT *)malloc(MAXLEAV*sizeof(struct ELEMENT)); /* */

	for(i=0;i<MAXLEAV;i++){
		observing_system.osc[i].name=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
		observing_system.osc[i].type=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	}
	observing_system.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	discipline_area.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	discipline_area.leaves[0].leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	discipline_area.leaves[0].leaves[0].leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	discipline_area.leaves[0].leaves[0].leaves[0].leaves=(struct ELEMENT *)malloc(256*sizeof(struct ELEMENT));
	reference_list.irefs = (struct INTERNAL_REFERENCE *)malloc(MAXLEAV*sizeof(struct INTERNAL_REFERENCE));
	file.leaves=(struct ELEMENT*)malloc(MAXLEAV*sizeof(struct ELEMENT));
	for(i=0;i<MAXLEAV;i++)
		array2d.leaves[i].leaves=(struct ELEMENT*)malloc(MAXLEAV*sizeof(struct ELEMENT));
	file.leaves[3].attributes=(struct ATTRIBUTE *)malloc(sizeof(struct ATTRIBUTE));
	for(i=0;i<MAXLEAV;i++) /* max number of products files = MAXLEAV */
		array2d.leaves[i].leaves=(struct ELEMENT*)malloc(MAXFNAML*sizeof(struct ELEMENT));
	prodfnam=(char **)malloc(numproducts*sizeof(char *));
	for(i=0;i<numproducts;i++)
		prodfnam[i]=(char *)malloc(MAXFNAML);
	po.attributes=(struct ATTRIBUTE *)malloc(MAXLEAV*sizeof(struct ATTRIBUTE));
	pds.products=(FILE**)malloc(numproducts*sizeof(FILE *));
	/*observing_system.osc=(struct OBSERVING_SYSTEM_COMPONENT*)malloc(2*sizeof(struct OBSERVING_SYSTEM_COMPONENT));*/
	modification_history.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	modification_history.leaves[0].leaves=(struct ELEMENT *)malloc(3*sizeof(struct ELEMENT));
	primary_result_summary.leaves=(struct ELEMENT *)malloc(4*sizeof(struct ELEMENT));
	primary_result_summary.leaves[3].leaves=(struct ELEMENT *)malloc(3*sizeof(struct ELEMENT));

	for(i=0;i<2;i++){
		observing_system.osc[i].name=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
		observing_system.osc[i].type=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	}
	oa.target = (struct TARGET_IDENTIFICATION *)malloc(sizeof(struct TARGET_IDENTIFICATION));
	oa.target[0].iref = (struct INTERNAL_REFERENCE *)malloc(sizeof(struct INTERNAL_REFERENCE));
/*	reference_list.spe.name;
	reference_list.spe.value;
	reference_list.spe.external_source_product_identifier.value;
	reference_list.spe.external_source_product_identifier.name;
	reference_list.spe.reference_type.name;
	reference_list.spe.reference_type.value;
	reference_list.spe.curating_facility.name;
	reference_list.spe.curating_facility.value;
	NRL_IR = number of REFERENCE_LIST Internal Reference items */
	/*set default values */
   strcpy(moddate,"def1" );
   strcpy(svers, "def2");
   strcpy(descr, "def3");
   strcpy(tstart, "1970-01-01T00:00:00Z");
   strcpy(tstop, "1970-01-01T00:00:00Z");
	/*read configutation file */
  cfp=fopen(confname, "r");
  if(cfp==NULL){
     perror("configuration file not present");
     exit(1);
  }
  /* read configuration file content into variables in memory */
  res=fscanf(cfp, "BITPIX %hu\n",&bitpix);
  res=fscanf(cfp, "SIGN %10s\n",buf);
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
  strcpy(send,buf);
  res=fscanf(cfp,  "WIDTH %u\n",&width);
  res=fscanf(cfp, "HEIGHT %u\n",&height);
  if(bitpix>8) {
	sprintf(dtype,"%s%s%u",ssign,send,(unsigned int)bitpix/8);
 } else if (bitpix==8){
	 (sign==1)?strcpy(dtype,"SignedByte"):strcpy(dtype,"UnsignedByte");
 }
  sprintf(swidth,"%u",width);
  sprintf(sheight,"%u",height);
    /* Observation Area */
	  res=fscanf(cfp,"NPI %d\n",&npi);
  if (verbose) fprintf(stderr,"n. of Processing Instructions: %d\n",npi);
  for(i=0;i<npi;i++){
	res=fscanf(cfp,"%5s %s\n",key,pds.xml_model[i]);
	for(j=0;j<strlen(pds.xml_model[i]);j++)
		if(pds.xml_model[i][j]=='>') pds.xml_model[i][j]=' ';
	if (verbose) fprintf(stderr,"key: %5s  value:%s\n",key,pds.xml_model[i]);
  }
  res=fscanf(cfp,"NPOATTR %d\n",&npoattr);
  if (verbose) fprintf(stderr,"n. of <Product_Observational> attributes: %d\n",npoattr);
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
  if (verbose) fprintf(stderr,"Number of Citation Information = %d\n", nci);
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
  if (verbose) fprintf(stderr,"read MOD_DATE = %s\n", moddate);
  if (verbose) fprintf(stderr,"read VERSID = %s\n", svers);
  if (verbose) fprintf(stderr,"read DESCR = %s\n", descr);
  /* Observation Area */
  if (verbose) fprintf(stderr,"going to read configuration file about Observation_Area\n");
  fprintf(stderr,"going to read configuration file about Time_Coordinates\n");
  res=fscanf(cfp,"TSTART %s\n",tstart);
  res=fscanf(cfp,"TSTOP %s\n",tstop);
  /* printout variables content for debug */
  if (verbose) fprintf(stderr,"Now printing variables content for debug\n");
  if (verbose) fprintf(stderr,"BITPIX = %d\n",bitpix);
  if (verbose) fprintf(stderr,"SIGN = %d\n",sign);
  if (verbose) fprintf(stderr,"ENDIAN = %d\n",end);
  if (verbose) fprintf(stderr,"WIDTH = %u\n",width);/* Sample == n. of columns */
  if (verbose) fprintf(stderr,"HEIGHT = %u\n",height); /* Line == n. of rows */
  if (verbose) fprintf(stderr,"PDS4 <data_type> = %s\n",dtype);
  if (verbose) fprintf(stderr,"TSTART = %s\n",tstart);/* Sample == n. of columns */
  if (verbose) fprintf(stderr,"TSTOP = %s\n",tstop);/* Sample == n. of columns */
  if (verbose) fprintf(stderr,"going to read configuration file about Primary_Result_Summary\n");
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
  if (verbose) fprintf(stderr,"going to read configuration file about Observing_System\n");
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
	oa.target[0].iref[0].lid_reference.value[0] = 'A';
	oa.target[0].iref[0].reference_type.value[0] = 'B';
	oa.target[0].iref[0].lid_reference.name[0] = 'C';
	oa.target[0].iref[0].reference_type.name[0] = 'D';
	res=fscanf(cfp,"TARGET_NAME %s\n",oa.target[0].name.value); /*  */
	res=fscanf(cfp,"TARGET_TYPE %s\n",oa.target[0].type.value); /*  */
	res=fscanf(cfp,"%s %s\n",key,oa.target[0].iref[0].lid_reference.value);
	if(verbose)fprintf(stderr,"TARGET[%d] n. lid_reference: %s\n",0,oa.target[0].iref[0].lid_reference.value);
	res=fscanf(cfp,"%s %s\n",key,oa.target[0].iref[0].reference_type.value);
	if(verbose)fprintf(stderr,"TARGET[%d] n. reference_type: %s\n",0,oa.target[0].iref[0].reference_type.value);
  /* read Mission Area configuration */
	res=fscanf(cfp,"MISSION %s\n",mission); /* MISSION */
	if(verbose)fprintf(stderr,"MISSION: %s\n",mission);
	/* START reading Mission Information
	 * res=fscanf(cfp,"MISS_ID %s\n",miss_id); /* MISS_ID (element name)
	psa:Mission_Information
*/
	strcpy((char*)miss_id, "psa:Mission_Information");
	if(verbose)fprintf(stderr,"Before PHASE\n");
	/* <psa:Mission_Information>  ---> mission_area.leaves[0] */
												  /* MissInf.MissPhase.PhaseName */
	res=fscanf(cfp,"PHASE_NAME %s\n",mission_area.leaves[0].leaves[0].leaves[0].value); /* mission phase name */
	for(j=0;j<strlen(mission_area.leaves[0].leaves[0].leaves[0].value);j++)
		if(mission_area.leaves[0].leaves[0].leaves[0].value[j]=='>') mission_area.leaves[0].leaves[0].leaves[0].value[j]=' ';
	if (verbose) fprintf(stderr, "read PHASE_NAME res = %d value = %s\n", res, mission_area.leaves[0].leaves[0].leaves[0].value);
												/* MissInf.MissPhase.PhaseID */
	res=fscanf(cfp,"PHASE_ID %s\n",mission_area.leaves[0].leaves[0].leaves[1].value); /* mission phase id */
	for(j=0;j<strlen(mission_area.leaves[0].leaves[0].leaves[1].value);j++)
		if(mission_area.leaves[0].leaves[0].leaves[1].value[j]=='>') mission_area.leaves[0].leaves[0].leaves[1].value[j]=' ';
	if (verbose) fprintf(stderr, "read PHASE_ID res = %d value = %s\n", res, mission_area.leaves[0].leaves[0].leaves[1].value);
	res=fscanf(cfp,"CLOCK_START %s\n",mission_area.leaves[0].leaves[0].value); /* clock tstart */
	if (verbose) fprintf(stderr, "read CLOCK_START res = %d value = %s\n", res, mission_area.leaves[0].leaves[0].value);
	res=fscanf(cfp,"CLOCK_STOP %s\n",mission_area.leaves[0].leaves[1].value); /* clock tsop */
	if (verbose) fprintf(stderr, "read CLOCK_STOP res = %d value = %s\n", res, mission_area.leaves[0].leaves[1].value);
												/* MissInf.StartOrbit */
	res=fscanf(cfp,"START_ORB %s\n",mission_area.leaves[0].leaves[1].value); /* start orbit */
	if (verbose) fprintf(stderr, "read START_ORB res = %d value = %s\n", res, mission_area.leaves[0].leaves[1].value);
												/* MissInf.StopOrbit */
	res=fscanf(cfp,"STOP_ORB %s\n",mission_area.leaves[0].leaves[2].value); /* stop orbit */
	if (verbose) fprintf(stderr, "read STOP_ORB res = %d value = %s\n", res, mission_area.leaves[0].leaves[2].value);
	/* END reading Mission Information */

	/* START reading Sub Instrument */
//	res=fscanf(cfp,"SU %s\n",su); /* SUB INSTRUMENT (element name) */
	strcpy((char*)su, "psa:Sub-Instrument");
										/* Sub-Instrument.Identifier */
	res=fscanf(cfp,"SU_ID %s\n",mission_area.leaves[1].leaves[0].value); /* <psa:identifier>  */
	if (verbose) fprintf(stderr, "read SU_ID res = %d value = %s\n", res, mission_area.leaves[1].leaves[0].value);
											/* Sub-Instrument.Name */
	res=fscanf(cfp,"SU_NAME %s\n",mission_area.leaves[1].leaves[1].value); /* <psa:name>  */
	for(j=0;j<strlen(mission_area.leaves[1].leaves[1].value);j++)
		if(mission_area.leaves[1].leaves[1].value[j]=='>') mission_area.leaves[1].leaves[1].value[j]=' ';
	if (verbose) fprintf(stderr, "read SU_NAME res = %d value = %s\n", res, mission_area.leaves[1].leaves[1].value);
												/* Sub-Instrument.Type */
	res=fscanf(cfp,"SU_TYPE %s\n",mission_area.leaves[1].leaves[2].value); /* <psa:type>  */
	if (verbose) fprintf(stderr, "read SU_TYPE res = %d value = %s\n", res, mission_area.leaves[1].leaves[2].value);
		/* END reading Sub Instrument */
/*
OBSCON_IPM Surface
OBSCON_IPD TARGETED
OBSCON_OID 1409764716
OBSCON_OTYPE INDIVIDUAL
 */
	/* START reading <psa:Observation_Context> */
	res=fscanf(cfp,"OBSCON_IPM %s\n",mission_area.leaves[2].leaves[0].value);   /* <psa:instrument_pointing_mode>  */
	if (verbose) fprintf(stderr, "read OBSCON_IPM res = %d value = %s\n", res, mission_area.leaves[2].leaves[0].value);
	res=fscanf(cfp,"OBSCON_IPD %s\n",mission_area.leaves[2].leaves[1].value);   /* <psa:instrument_pointing_description> */
	if (verbose) fprintf(stderr, "read OBSCON_IPD res = %d value = %s\n", res, mission_area.leaves[2].leaves[1].value);
	res=fscanf(cfp,"OBSCON_OID %s\n",mission_area.leaves[2].leaves[2].value);   /* <psa:observation_identifier> */
	if (verbose) fprintf(stderr, "read OBSCON_OID res = %d value = %s\n", res, mission_area.leaves[2].leaves[2].value);
	res=fscanf(cfp,"OBSCON_OTYPE %s\n",mission_area.leaves[2].leaves[3].value); /* <psa:observation_type> */
	if (verbose) fprintf(stderr, "read OBSCON_OTYPE res = %d value = %s\n", res, mission_area.leaves[2].leaves[3].value);
/* END reading <psa:Observation_Context> */

	/* START reading <psa:Processing_Context> */
//	res=fscanf(cfp,"PRODID %s\n",mission_area.leaves[3].leaves[0].value); /* product id */
//	if (verbose) fprintf(stderr, "read PRODID res = %d value = %s\n", res, mission_area.leaves[3].leaves[0].value);
	res=fscanf(cfp,"SWNAME %s\n",mission_area.leaves[3].leaves[0].value); /* <psa:psa:processing_software_title> (software name) */
	for(j=0;j<strlen(mission_area.leaves[3].leaves[0].value);j++)
		if(mission_area.leaves[3].leaves[0].value[j]=='>') mission_area.leaves[3].leaves[0].value[j]=' ';
	if (verbose) fprintf(stderr, "read SWNAME res = %d value = %s\n", res, mission_area.leaves[3].leaves[1].value);
	res=fscanf(cfp,"SWVERS %s\n",mission_area.leaves[3].leaves[2].value); /* <psa:processing_software_version>  */
	if (verbose) fprintf(stderr, "read SWVERS res = %d value = %s\n", res, mission_area.leaves[3].leaves[2].value);

/* END reading <psa:Processing_Context> */

	/* START reading Cassis_Data */
//	res=fscanf(cfp,"DATA_NAME %s\n",dname); /* instrument:DATA (element name) */
	strcpy((char*)dname, "em16_tgo_cas:Cassis_Data");
/*
 * <em16_tgo_cas:Cassis_Data> --> mission_area.leaves[4]
 *
INSTR_IFOV 1.142E-5
IFOV_DESCR rad/px
FILTERS BLU>RED>NIR>PAN
HK_FILTER PAN
FOCAL_LENGTH 0.876
FOCAL_DESCR M
F_NUMBER 6.49
TELESCOPE Three-mirror>anastigmat>with>powered>fold>mirror
CAS_DESCR 2D>Array
PIX_HEIGHT 10.0
PIX_HE_UNIT MICRON
PIX_WIDTH 10.0
PIX_WI_UNIT MICRON
DET_DESCR SI>CMOS>HYBRID>(OSPREY>2K)
NOISE 61.0
NOISE_UNIT ELECTRON

 */
			/*                 <em16_tgo_cas:Instrument_Information> */
			                                            /* CassisData.InstrumentInformation.instrument_ifov */
	res=fscanf(cfp,"INSTR_IFOV %s\n",mission_area.leaves[4].leaves[0].leaves[0].value); /* <em16_tgo_cas:instrument_ifov>  */
	if (verbose) fprintf(stderr, "read INSTR_IFOV res = %d value = %s\n", res, mission_area.leaves[4].leaves[0].leaves[0].value);
			                                            /* CassisData.InstrumentInformation.ifov_unit */
	res=fscanf(cfp,"IFOV_UNIT %s\n",mission_area.leaves[4].leaves[0].leaves[1].value); /* instrument_ifov_unit_description>  */
	if (verbose) fprintf(stderr, "read IFOV_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[0].leaves[1].value);
			                                            /* CassisData.InstrumentInformation.filters_available */
	res=fscanf(cfp,"FILTERS %s\n",mission_area.leaves[4].leaves[0].leaves[2].value); /* <em16_tgo_cas:filters_available>  */
	for(j=0;j<strlen(mission_area.leaves[4].leaves[0].leaves[2].value);j++)
		if(mission_area.leaves[4].leaves[0].leaves[2].value[j]=='>') mission_area.leaves[4].leaves[0].leaves[2].value[j]=' ';
	if (verbose) fprintf(stderr, "read FILTERS res = %d value = %s\n", res, mission_area.leaves[4].leaves[0].leaves[2].value);
							/*	<em16_tgo_cas:HK_Derived_Data> */
			                                            /* CassisData.HK_Derived_Data.filter */
	res=fscanf(cfp,"HK_FILTER %s\n",mission_area.leaves[4].leaves[1].leaves[0].value); /* <em16_tgo_cas:filter>  */
	if (verbose) fprintf(stderr, "read HK_FILTER res = %d value = %s\n", res, mission_area.leaves[4].leaves[1].leaves[0].value);
							/*	<em16_tgo_cas:HK_Derived_Data> */

							/* <em16_tgo_cas:Telescope_Information> */
												/* CassisData.Telescope_Information.focal_length */
	res=fscanf(cfp,"FOCAL_LENGTH %s\n",mission_area.leaves[4].leaves[2].leaves[0].value); /* <em16_tgo_cas:focal_length>  */
	if (verbose) fprintf(stderr, "read FOCAL_LENGTH res = %d value = %s\n", res, mission_area.leaves[4].leaves[2].leaves[0].value);
												/* CassisData.Telescope_Information.focal_unit */
	res=fscanf(cfp,"FOCAL_UNIT %s\n",mission_area.leaves[4].leaves[2].leaves[1].value); /* <em16_tgo_cas:focal_length_unit_description>  */
	if (verbose) fprintf(stderr, "read FOCAL_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[2].leaves[1].value);
												/* CassisData.Telescope_Information.focal_number */
	res=fscanf(cfp,"F_NUMBER %s\n",mission_area.leaves[4].leaves[2].leaves[2].value); /* <em16_tgo_cas:focal_length_unit_description>  */
	if (verbose) fprintf(stderr, "read F_NUMBER res = %d value = %s\n", res, mission_area.leaves[4].leaves[2].leaves[2].value);
												/* CassisData.Telescope_Information.telescope_description */
	res=fscanf(cfp,"TELESCOPE %s\n",mission_area.leaves[4].leaves[2].leaves[3].value); /* <em16_tgo_cas:telescope_description>  */
	for(j=0;j<strlen(mission_area.leaves[4].leaves[2].leaves[3].value);j++)
		if(mission_area.leaves[4].leaves[2].leaves[3].value[j]=='>') mission_area.leaves[4].leaves[2].leaves[3].value[j]=' ';
	if (verbose) fprintf(stderr, "read TELESCOPE res = %d value = %s\n", res, mission_area.leaves[4].leaves[2].leaves[3].value);
	 							/* <em16_tgo_cas:Detector_Information> */
												/* CassisData.Detector_Information.cassis_description */
	res=fscanf(cfp,"CAS_DESCR %s\n",mission_area.leaves[4].leaves[3].leaves[0].value); /* <em16_tgo_cas:cassis_description>  */
	for(j=0;j<strlen(mission_area.leaves[4].leaves[3].leaves[0].value);j++)
		if(mission_area.leaves[4].leaves[3].leaves[0].value[j]=='>') mission_area.leaves[4].leaves[3].leaves[0].value[j]=' ';
	if (verbose) fprintf(stderr, "read CAS_DESCR res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[0].value);
												/* CassisData.Detector_Information.pixel_height */
	res=fscanf(cfp,"PIX_HEIGHT %s\n",mission_area.leaves[4].leaves[3].leaves[1].value); /* <em16_tgo_cas:pixel_height>  */
	if (verbose) fprintf(stderr, "read PIX_HEIGHT res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[1].value);
												/* CassisData.Detector_Information.pixel_height_unit */
	res=fscanf(cfp,"PIX_HE_UNIT %s\n",mission_area.leaves[4].leaves[3].leaves[2].value); /* <em16_tgo_cas:pixel_height_unit_description>  */
	if (verbose) fprintf(stderr, "read PIX_HE_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[2].value);
												/* CassisData.Detector_Information.pixel_width */
	res=fscanf(cfp,"PIX_WIDTH %s\n",mission_area.leaves[4].leaves[3].leaves[3].value); /* <em16_tgo_cas:pixel_width>  */
	if (verbose) fprintf(stderr, "read PIX_WIDTH res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[3].value);
												/* CassisData.Detector_Information.pixel_width_unit */
	res=fscanf(cfp,"PIX_WI_UNIT %s\n",mission_area.leaves[4].leaves[3].leaves[4].value); /* <em16_tgo_cas:pixel_width_unit_description>  */
	if (verbose) fprintf(stderr, "read PIX_WI_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[4].value);
												/* CassisData.Detector_Information.detector_description */
	res=fscanf(cfp,"DET_DESCR %s\n",mission_area.leaves[4].leaves[3].leaves[5].value); /* <em16_tgo_cas:detector_description>  */
	for(j=0;j<strlen(mission_area.leaves[4].leaves[3].leaves[5].value);j++)
		if(mission_area.leaves[4].leaves[3].leaves[5].value[j]=='>') mission_area.leaves[4].leaves[3].leaves[5].value[j]=' ';
	if (verbose) fprintf(stderr, "read DET_DESCR res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[5].value);
												/* CassisData.Detector_Information.read_noise */
	res=fscanf(cfp,"NOISE %s\n",mission_area.leaves[4].leaves[3].leaves[6].value); /* <em16_tgo_cas:read_noise>  */
	if (verbose) fprintf(stderr, "read NOISE res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[6].value);
												/* CassisData.Detector_Information.read_noise_unit */
	res=fscanf(cfp,"NOISE_UNIT %s\n",mission_area.leaves[4].leaves[3].leaves[7].value); /* <em16_tgo_cas:read_noise_unit_description>  */
	if (verbose) fprintf(stderr, "read NOISE_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[7].value);
	/* STOP reading Cassis_Data */
	/*  */
	res=fscanf(cfp,"AV_INT %s\n",mission_area.leaves[2].leaves[0].value); /*  */
	res=fscanf(cfp,"CAL_TYPE %s\n",mission_area.leaves[2].leaves[0].value); /*  */
	res=fscanf(cfp,"MEAS_RANGE_IDX %s\n",mission_area.leaves[2].leaves[1].value); /* */
	res=fscanf(cfp,"MEAS_RANGE %s\n",mission_area.leaves[2].leaves[2].value); /*  */
	res=fscanf(cfp,"REF_FRAME %s\n",mission_area.leaves[2].leaves[3].value);/*  */
	res=fscanf(cfp,"SPICE_FRAME %s\n",mission_area.leaves[2].leaves[4].value); /*  */

	/*START Instrument Status*/
	res=fscanf(cfp,"STATUS_NAME %s\n",istatus); /*  */
	res=fscanf(cfp,"SC_STATUS %s\n",mission_area.leaves[3].leaves[0].value); /*  */
	/*STOP Instrument Status*/
	res=fscanf(cfp,"NRLIR %d\n",&nrlir); /*  */
/* NRLIR = number of REFERENCE_LIST Internal Reference items
 * reference_list.irefs[i] */
  for(i=0;i<nrlir;i++){
	res=fscanf(cfp,"%11s %s\n",key,reference_list.irefs[i].lid_reference.value);
	res=fscanf(cfp,"%15s %s\n",key,reference_list.irefs[i].reference_type.value);
/*	for(j=0;j<strlen(reference_list.irefs[i].lid_reference.value);j++){
		if(reference_list.irefs[i].lid_reference.value[j]=='>') reference_list.irefs[i].lid_reference.value[j]=' ';
		if(reference_list.irefs[i].reference_type.value[j]=='>') reference_list.irefs[i].reference_type.value[j]=' ';
	}*/
  }
  /*<Source_Product_External> input
   <Source_Product_External>
            <external_source_product_identifier>SCI__DCAS__0180E932_2025-106T23-35-39__00003.EXM</external_source_product_identifier>
            <reference_type>data_to_telemetry_source_product</reference_type>
            <curating_facility>ExoMars 16 Science Operations Center</curating_facility>
	</Source_Product_External>
	SPE_PRODID SCI__DCAS__0180E932_2025-106T23-35-39__00003.EXM
	SPE_REFTYPE data_to_telemetry_source_product
	SPE_CURFAC ExoMars>16>Science>Operations>Center
   */
  res=fscanf(cfp,"SPE_PRODID %s\n",reference_list.spe.external_source_product_identifier.value);
  if (verbose) fprintf(stderr, "read SPE_PRODID res = %d value = %s\n", res, reference_list.spe.external_source_product_identifier.value);
  res=fscanf(cfp,"SPE_REFTYPE %s\n",reference_list.spe.reference_type.value);
  if (verbose) fprintf(stderr, "read SPE_REFTYPE res = %d value = %s\n", res, reference_list.spe.reference_type.value);
  res=fscanf(cfp,"SPE_CURFAC %s\n",reference_list.spe.curating_facility.value);
	for(j=0;j<strlen(reference_list.spe.curating_facility.value);j++)
		if(reference_list.spe.curating_facility.value[j]=='>') reference_list.spe.curating_facility.value[j]=' ';
  if (verbose) fprintf(stderr, "read SPE_CURFAC res = %d value = %s\n", res, reference_list.spe.curating_facility.value);
  for(j=0;j<strlen(reference_list.spe.curating_facility.value);j++)
		if(reference_list.spe.curating_facility.value[j]=='>') reference_list.spe.curating_facility.value[j]=' ';
  if (verbose)   fprintf(stderr, "now value = %s\n", reference_list.spe.curating_facility.value);
/*
         <File>
            <file_name>cas_raw_sc_20250416T233856-20250416T233900-33001-50-PAN-1409764716-29-0__4_0.dat</file_name>
            <local_identifier>cas_raw_sc_20250416T233856-20250416T233900-33001-50-PAN-1409764716-29-0__4_0.dat</local_identifier>
            <creation_date_time>2025-04-17T07:02:54.893Z</creation_date_time>
            <file_size unit="byte">931840</file_size>
            <md5_checksum>c72ce8fa7a76c3dcd3fbb33ea72b8bad</md5_checksum>
            <comment>Image Data File</comment>
        </File>

 */
for(i=0;i<numproducts;i++){
  res=fscanf(cfp,"%11s %s\n",key,file.leaves[1].value); /* <local_identifier> LOCAL_ID_NN */
  fprintf(stderr, "read LOCAL_ID key = %s res = %d value = %s\n",key, res, file.leaves[1].value);
  res=fscanf(cfp,"%11s %s\n",key,file.leaves[2].value); /* <creation_date_time> CREATION_NN */
  fprintf(stderr, "read CREATION key = %s res = %d value = %s\n",key, res, file.leaves[2].value);
  res=fscanf(cfp,"%12s %s\n",key,file.leaves[3].value); /* <file_size unit="byte"> FILE_SIZE_NN */
  fprintf(stderr, "read FILE_SIZE key = %s res = %d value = %s\n",key, res, file.leaves[3].value);
  res=fscanf(cfp,"%12s %s\n",key,file.leaves[4].value); /* <md5_checksum> MD5_CHECK_NN */
  fprintf(stderr, "read MD5_CHECK key = %s res = %d value = %s\n",key, res, file.leaves[4].value);
  res=fscanf(cfp,"%10s %s\n",key,file.leaves[5].value); /* <comment> COMMENT_NN */
  for(j=0;j<strlen(file.leaves[5].value);j++)
		if(file.leaves[5].value[j]=='>') file.leaves[5].value[j]=' ';
  fprintf(stderr, "read COMMENT key = %s res = %d value = %s\n",key, res, file.leaves[5].value);
	/*
			<psa:Sub-Instrument>
                <psa:identifier>SCI</psa:identifier>
                <psa:name>CASSIS Science</psa:name>
                <psa:type>Imager</psa:type>
            </psa:Sub-Instrument>
SI_ID SCI
SI_NAME CASSIS>Science
SI_TYPE Imager

	 */
//   res=fscanf(cfp,"SI_ID %s\n",file.leaves[].value); /* <psa:identifier> SI_ID */
//   fprintf(stderr, "read SI_ID res = %d value = %s\n", res, file.leaves[].value);
//   res=fscanf(cfp,"SI_NAME %s\n",file.leaves[].value); /* <psa:name> SI_NAME */
//   fprintf(stderr, "read SI_NAME res = %d value = %s\n", res, file.leaves[].value);
//   for(j=0;j<strlen(file.leaves[].value);j++)
// 		if(file.leaves[].value[j]=='>') file.leaves[].value[j]=' ';
//   res=fscanf(cfp,"SI_TYPE %s\n",file.leaves[1].value); /* <psa:type> SI_TYPE */
//   fprintf(stderr, "read SI_TYPE res = %d value = %s\n", res, file.leaves[].value);
  }
/* close input configuration file */
  fclose(cfp);

	/* setup data structures and links */
	po.ia=&ia; po.oa=&oa; po.fao=&fao; pds.po=&po;
	if(verbose)fprintf(stderr,"main() numproducts = %d\n",numproducts);
	for(i=0;i<numproducts;i++){
		pds.products[i] = (FILE*)malloc(MAXFNAML);
		if(argv[i+2]!=NULL){
			strcpy((char*)prodfnam[i],argv[i+3]);
		} else {
			perror("ERROR: filename lacking, exiting");
			exit(1);
		}
	}

	if(verbose)fprintf(stderr,"main() starting copying strings\n");
/*	strcpy((char*)pds.xmlintest,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>");
	strcpy((char*)pds.xml_model[0],"href=\"https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1L00.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[1],"href=\"https://psa.esa.int/psa/v1/PDS4_PSA_1L00_1401.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[2],"href=\"https://pds.nasa.gov/pds4/geom/v1/PDS4_GEOM_1L00_1970.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[3],"href=\"https://pds.nasa.gov/pds4/disp/v1/PDS4_DISP_1L00_1510.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[4],"href=\"https://psa.esa.int/psa/em16/tgo/cas/v1/PDS4_EM16_TGO_CAS_1L00_1200.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[5],"href=\"https://psa.esa.int/psa/mission/em16/v1/PDS4_EM16_1L00_1300.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
*/	strcpy((char*)po.name,"Product_Observational");
	strcpy((char*)ia.name,"Identification_Area");
	strcpy((char*)oa.name,"Observation_Area");
	strcpy((char*)fao.name,"File_Area_Observational");
	strcpy((char*)logical_identifier.name,"logical_identifier");
	strcpy((char*)logical_identifier.value,"urn:esa:psa:em16_tgo_cas:data_derived:cas_der_sc_20250416t233856-20250416t233900-33001-50-pan-1409764716-29-0");
	strcpy((char*)version_id.name,"version_id");
	strcpy((char*)version_id.value,"1.0");
	strcpy((char*)title.name,"title");
	strcpy((char*)title.value,"Test to develop EXOMARS 2016 CaSSIS TDM PDS4 labels");
	strcpy((char*)information_model_version.name,"information_model_version");
	strcpy((char*)information_model_version.value,"1.21.0.0");
	strcpy((char*)product_class.name,"product_class");
	strcpy((char*)product_class.value,"Product_Observational");
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
	if(verbose)fprintf(stderr,"main() before Observation_Area\n");
	strcpy((char*)timecoord.name,"Time_Coordinates");
	strcpy((char*)timecoord.tstart.name,"start_date_time");
	strcpy((char*)timecoord.tstart.value,tstart);
	strcpy((char*)timecoord.tstop.name,"stop_date_time");
	strcpy((char*)timecoord.tstop.value,tstop);
	strcpy((char*)primary_result_summary.name,"Primary_Result_Summary");
	strcpy((char*)primary_result_summary.leaves[0].name,"purpose");
	strcpy((char*)primary_result_summary.leaves[0].value,"Science");
	strcpy((char*)primary_result_summary.leaves[1].name,"processing_level");
	strcpy((char*)primary_result_summary.leaves[1].value,proclev);
	strcpy((char*)primary_result_summary.leaves[2].name,"description");
	strcpy((char*)primary_result_summary.leaves[2].value,resdescr);
	strcpy((char*)primary_result_summary.leaves[3].name,"Science_Facets");
	strcpy((char*)primary_result_summary.leaves[3].leaves[0].name,"wavelength_range");
	strcpy((char*)primary_result_summary.leaves[3].leaves[0].value,"Visible");
	strcpy((char*)primary_result_summary.leaves[3].leaves[1].name,"domain");
	strcpy((char*)primary_result_summary.leaves[3].leaves[1].value,"Surface");
	strcpy((char*)primary_result_summary.leaves[3].leaves[2].name,"discipline_name");
	strcpy((char*)primary_result_summary.leaves[3].leaves[2].value,"Geosciences");
    if(verbose)fprintf(stderr,"main() before Observing System\n");	
	strcpy((char*)observing_system.name,"Observing_System");
	strcpy((char*)observing_system.leaves[0].name,"name");
	strcpy((char*)observing_system.osc[0].ename,"Observing_System_Component");
	strcpy((char*)observing_system.osc[0].name->name,"name");
	strcpy((char*)observing_system.osc[0].name->value,onamval[0]);
	strcpy((char*)observing_system.osc[0].type->name,"type");
	strcpy((char*)observing_system.osc[0].type->value,otypval[0]);
	strcpy((char*)observing_system.osc[0].ir.name,"Internal_Reference");
	strcpy((char*)observing_system.osc[0].ir.lid_reference.name,"lid_reference");
	strcpy((char*)observing_system.osc[0].ir.lid_reference.value,observing_system.osc[0].ir.lid_reference.value);
	strcpy((char*)observing_system.osc[0].ir.reference_type.name,"reference_type");
	strcpy((char*)observing_system.osc[0].ir.reference_type.value,observing_system.osc[0].ir.reference_type.value);
	strcpy((char*)observing_system.osc[1].ename,"Observing_System_Component");
	strcpy((char*)observing_system.osc[1].name->name,"name");
	strcpy((char*)observing_system.osc[1].name->value,onamval[1]);
	strcpy((char*)observing_system.osc[1].type->name,"type");
	strcpy((char*)observing_system.osc[1].type->value,otypval[1]);
	strcpy((char*)observing_system.osc[1].ir.name,"Internal_Reference");
	strcpy((char*)observing_system.osc[1].ir.lid_reference.name,"lid_reference");
	strcpy((char*)observing_system.osc[1].ir.lid_reference.value,observing_system.osc[1].ir.lid_reference.value);
	strcpy((char*)observing_system.osc[1].ir.reference_type.name,"reference_type");
	strcpy((char*)observing_system.osc[1].ir.reference_type.name,observing_system.osc[1].ir.reference_type.value);
    if(verbose)fprintf(stderr,"main() before Target_Identification\n");

	strcpy((char*)oa.target[0].ename,"Target_Identification");
	strcpy((char*)oa.target[0].name.name,"name");
	strcpy((char*)oa.target[0].type.name,"type");
	strcpy((char*)oa.target[0].iref[0].name, "Internal_Reference");
	strcpy((char*)oa.target[0].iref->lid_reference.name, "lid_reference");
	strcpy((char*)oa.target[0].iref->reference_type.name, "reference_type");
    if(verbose)fprintf(stderr,"oa.target[0].iref[0].lid_reference.value = %s\n", oa.target[0].iref[0].lid_reference.value);
    if(verbose)fprintf(stderr,"oa.target[0].iref[0].reference_type.value = %s\n", oa.target[0].iref[0].reference_type.value);

	/*
	strcpy((char*)oa.target[0].type.value,oa.target[0].type.value);
	strcpy((char*)oa.target[0].iref[0]->,oa.target[0].iref[0]->lid_reference.value);
	res=fscanf(cfp,"TARGET_NAME %s\n",oa.target[0].name.value);
	res=fscanf(cfp,"TARGET_TYPE %s\n",oa.target[0].type.value);
	res=fscanf(cfp,"%s %s\n",key,oa.target[0].iref[0]->lid_reference.value);

	strcpy((char*)oa.target[0].iref[0].lid_reference.name, "lid_reference");
	strcpy((char*)oa.target[0].iref[0].reference_type.name, "reference_type");*/
//	if(verbose)fprintf(stderr,"TARGET[%d] n. lid_reference: %s\n",0,oa.target[0].iref->lid_reference.value);
//	if(verbose)fprintf(stderr,"TARGET[%d] n. reference_type: %s\n",0,oa.target[0].iref->reference_type.value);
//	if(verbose)fprintf(stderr,"TARGET[%d] n. reference_type: %s\n",1,oa.target[1].iref->reference_type.value);
/* <psa:Mission_Information>
                <psa:Mission_Phase>
                    <psa:name>Science Phase</psa:name>
                    <psa:id>psp</psa:id>
                </psa:Mission_Phase>
                <psa:start_orbit_number>33001</psa:start_orbit_number>
                <psa:stop_orbit_number>33001</psa:stop_orbit_number>
            </psa:Mission_Information>
            <psa:Sub-Instrument>
                <psa:identifier>SCI</psa:identifier>
                <psa:name>CASSIS Science</psa:name>
                <psa:type>Imager</psa:type>
            </psa:Sub-Instrument>
            <psa:Observation_Context>
                <psa:instrument_pointing_mode>Surface</psa:instrument_pointing_mode>
                <psa:instrument_pointing_description>TARGETED</psa:instrument_pointing_description>
                <psa:observation_identifier>1409764716</psa:observation_identifier>
                <psa:observation_type>INDIVIDUAL</psa:observation_type>
            </psa:Observation_Context>
            <psa:Processing_Context>
                <psa:processing_software_title>EM16 Data Processing System</psa:processing_software_title>
                <psa:processing_software_version>5.6.0</psa:processing_software_version>
            </psa:Processing_Context>
            <em16_tgo_cas:Cassis_Data>
            */
	strcpy((char*)mission_area.name,"Mission_Area");
	strcpy((char*)mission_area.leaves[0].name,"psa:Mission_Information");
	strcpy((char*)mission_area.leaves[0].leaves[0].name,"psa:Mission_Phase");
	strcpy((char*)mission_area.leaves[0].leaves[0].leaves[0].name,"psa:name");
//	strcpy((char*)mission_area.leaves[0].leaves[0].leaves[0].value,"Science Phase");
	strcpy((char*)mission_area.leaves[0].leaves[0].leaves[1].name,"psa:id");
//	strcpy((char*)mission_area.leaves[0].leaves[0].leaves[1].value,"psp");
	strcpy((char*)mission_area.leaves[0].leaves[1].name,"psa:start_orbit_number");
	strcpy((char*)mission_area.leaves[0].leaves[2].name,"psa:stop_orbit_number");
	strcpy((char*)mission_area.leaves[1].name,"psa:Sub-Instrument");
	strcpy((char*)mission_area.leaves[1].leaves[0].name,"psa:identifier");
	strcpy((char*)mission_area.leaves[1].leaves[1].name,"psa:name");
	strcpy((char*)mission_area.leaves[1].leaves[2].name,"psa:type");
	strcpy((char*)mission_area.leaves[2].name,"psa:Observation_Context");
	strcpy((char*)mission_area.leaves[2].leaves[0].name,"psa:instrument_pointing_mode");
	strcpy((char*)mission_area.leaves[2].leaves[1].name,"psa:instrument_pointing_description");
	strcpy((char*)mission_area.leaves[2].leaves[2].name,"psa:observation_identifier");
	strcpy((char*)mission_area.leaves[2].leaves[3].name,"psa:observation_type");
	strcpy((char*)mission_area.leaves[3].name,"psa:Processing_Context");
	strcpy((char*)mission_area.leaves[3].leaves[0].name,"psa:processing_software_title");
	strcpy((char*)mission_area.leaves[3].leaves[1].name,"psa:processing_software_version");
	strcpy((char*)mission_area.leaves[4].name,"em16_tgo_cas:Cassis_Data");
	strcpy((char*)mission_area.leaves[4].leaves[0].name,"em16_tgo_cas:Instrument_Information");
	strcpy((char*)mission_area.leaves[4].leaves[0].leaves[0].name,"em16_tgo_cas:instrument_ifov");
	strcpy((char*)mission_area.leaves[4].leaves[0].leaves[1].name,"em16_tgo_cas:instrument_ifov_unit_description");
	strcpy((char*)mission_area.leaves[4].leaves[0].leaves[2].name,"em16_tgo_cas:filters_available");
	strcpy((char*)mission_area.leaves[4].leaves[1].name,"em16_tgo_cas:HK_Derived_Data");
	strcpy((char*)mission_area.leaves[4].leaves[1].leaves[0].name,"em16_tgo_cas:filter");
	strcpy((char*)mission_area.leaves[4].leaves[2].name,"em16_tgo_cas:Telescope_Information");
	strcpy((char*)mission_area.leaves[4].leaves[2].leaves[0].name,"em16_tgo_cas:focal_length_unit_description");
	strcpy((char*)mission_area.leaves[4].leaves[2].leaves[1].name,"em16_tgo_cas:f_number");
	strcpy((char*)mission_area.leaves[4].leaves[2].leaves[2].name,"em16_tgo_cas:filter");
	strcpy((char*)mission_area.leaves[4].leaves[2].leaves[3].name,"em16_tgo_cas:telescope_description");
	strcpy((char*)mission_area.leaves[4].leaves[3].name,"em16_tgo_cas:Detector_Information");
	strcpy((char*)mission_area.leaves[4].leaves[3].leaves[0].name,"em16_tgo_cas:cassis_description");
	strcpy((char*)mission_area.leaves[4].leaves[3].leaves[1].name,"em16_tgo_cas:pixel_height");
	strcpy((char*)mission_area.leaves[4].leaves[3].leaves[2].name,"em16_tgo_cas:pixel_height_unit_description");
	strcpy((char*)mission_area.leaves[4].leaves[3].leaves[3].name,"em16_tgo_cas:pixel_width");
	strcpy((char*)mission_area.leaves[4].leaves[3].leaves[4].name,"em16_tgo_cas:pixel_width_unit_description");
	strcpy((char*)mission_area.leaves[4].leaves[3].leaves[5].name,"em16_tgo_cas:detector_description");
	strcpy((char*)mission_area.leaves[4].leaves[3].leaves[6].name,"em16_tgo_cas:read_noise");
	strcpy((char*)mission_area.leaves[4].leaves[3].leaves[7].name,"em16_tgo_cas:read_noise_unit_description");

	if(verbose)fprintf(stderr,"main() before Investigation Area\n");
	strcpy((char*)iaa.ename,"Investigation_Area");
	strcpy((char*)iaa.name.name,"name");
//	strcpy((char*)iaa.name.value,"ExoMars 2016");
	strcpy((char*)iaa.type.name,"type");
	strcpy((char*)iaa.type.value,"Mission");
    if(verbose)fprintf(stderr,"main() before Internal_Reference\n");	
	strcpy((char*)iref.name,"Internal_Reference");
	strcpy((char*)iref.lid_reference.name,"lid_reference");
	strcpy((char*)iref.lid_reference.value,invlid);
	strcpy((char*)iref.reference_type.name,"reference_type");
	strcpy((char*)iref.reference_type.value,"data_to_investigation");
	/* Reference_List */
    if(verbose)fprintf(stderr,"main() before Reference List\n");
	strcpy((char*)reference_list.ename,"Reference_List");
	strcpy((char*)reference_list.irefs[0].name,"Internal_Reference");
	strcpy((char*)reference_list.irefs[0].lid_reference.name,"lid_reference");
	strcpy((char*)reference_list.irefs[0].reference_type.name,"reference_type");
	strcpy((char*)reference_list.irefs[1].name,"Internal_Reference");
	strcpy((char*)reference_list.irefs[1].lid_reference.name,"lid_reference");
	strcpy((char*)reference_list.irefs[1].reference_type.name,"reference_type");
	strcpy((char*)reference_list.irefs[2].name,"Internal_Reference");
	strcpy((char*)reference_list.irefs[2].lid_reference.name,"lid_reference");
	strcpy((char*)reference_list.irefs[2].reference_type.name,"reference_type");
	/* Source_Product_External (SPE)*/
	strcpy((char*)reference_list.spe.name, "Source_Product_External");
	strcpy((char*)reference_list.spe.external_source_product_identifier.name, "external_source_product_identifier");
	strcpy((char*)reference_list.spe.reference_type.name, "reference_type");
	strcpy((char*)reference_list.spe.curating_facility.name, "curating_facility");
/*  reference_list.spe.external_source_product_identifier.value;
  reference_list.spe.reference_type.value;
  reference_list.spe.curating_facility.value;
  <File>
            <file_name>cas_raw_sc_20250416T233856-20250416T233900-33001-50-PAN-1409764716-29-0__4_0.dat</file_name>
            <local_identifier>cas_raw_sc_20250416T233856-20250416T233900-33001-50-PAN-1409764716-29-0__4_0.dat</local_identifier>
            <creation_date_time>2025-04-17T07:02:54.893Z</creation_date_time>
            <file_size unit="byte">931840</file_size>
            <md5_checksum>c72ce8fa7a76c3dcd3fbb33ea72b8bad</md5_checksum>
            <comment>Image Data File</comment>
        </File>
*/
    if(verbose)fprintf(stderr,"main() before File \n");
	strcpy((char*)file.name,"File");
	strcpy((char*)file.leaves[0].name,"file_name");
	strcpy((char*)file.leaves[0].value,prodfnam[0]);
	strcpy((char*)file.leaves[1].name,"local_identifier");
	strcpy((char*)file.leaves[2].name,"creation_date_time");
	strcpy((char*)file.leaves[3].name,"file_size");
	strcpy((char*)file.leaves[3].attributes[0].name,"unit");
	strcpy((char*)file.leaves[3].attributes[0].value,"byte");
	strcpy((char*)file.leaves[4].name,"md5_checksum");
	strcpy((char*)file.leaves[5].name,"comment");
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
	/*oa.leaves[0]=observing_system;
	 * oa.leaves[1]=comment;
	 */
	strcpy((char*)oa.leaves[1].name,"comment");
	strcpy((char*)oa.leaves[1].value,"Observational Intent");

	if (init_pds(lp,&pds, prodfnam,argv)) perror("error in initing pds product archiving"); 
        if(verbose)fprintf(stderr,"main() after init_pds()\n");
    LIBXML_TEST_VERSION;

	doc = xmlNewDoc(BAD_CAST "1.0");

    if(verbose)fprintf(stderr,"main() opening <Product_Observational> Root node element step 0 \n");
    root_node = xmlNewNode(NULL, BAD_CAST po.name);    
    xmlDocSetRootElement(doc, root_node);
    attribute = root_node->properties;
	attr = xmlSetProp(root_node, (const xmlChar *)po.attributes[0].name,(const xmlChar *)po.attributes[0].value); /* product observational attribute */
	attr = xmlSetProp(root_node, (const xmlChar *)po.attributes[1].name,(const xmlChar *)po.attributes[1].value); /* product observational attribute */
	attr = xmlSetProp(root_node, (const xmlChar *)po.attributes[2].name,(const xmlChar *)po.attributes[2].value); /* product observational attribute */
	attr = xmlSetProp(root_node, (const xmlChar *)po.attributes[3].name,(const xmlChar *)po.attributes[3].value); /* product observational attribute */
	attr = xmlSetProp(root_node, (const xmlChar *)po.attributes[4].name,(const xmlChar *)po.attributes[4].value); /* product observational attribute */
	attr = xmlSetProp(root_node, (const xmlChar *)po.attributes[5].name,(const xmlChar *)po.attributes[5].value); /* product observational attribute */
	attr = xmlSetProp(root_node, (const xmlChar *)po.attributes[6].name,(const xmlChar *)po.attributes[6].value); /* product observational attribute */
    	 // Create the xml-model processing instruction
    pi = xmlNewPI(BAD_CAST "xml-model", BAD_CAST pds.xml_model[0]);
    // Add it before the root element
    xmlAddPrevSibling(root_node, pi);
    pi = xmlNewPI(BAD_CAST "xml-model", BAD_CAST pds.xml_model[1]);
    // Add it before the root element
    xmlAddPrevSibling(root_node, pi);
    pi = xmlNewPI(BAD_CAST "xml-model", BAD_CAST pds.xml_model[2]);
    // Add it before the root element
    xmlAddPrevSibling(root_node, pi);
    pi = xmlNewPI(BAD_CAST "xml-model", BAD_CAST pds.xml_model[3]);
    // Add it before the root element
    xmlAddPrevSibling(root_node, pi);
    pi = xmlNewPI(BAD_CAST "xml-model", BAD_CAST pds.xml_model[4]);
     // Add it before the root element
    xmlAddPrevSibling(root_node, pi);
   pi = xmlNewPI(BAD_CAST "xml-model", BAD_CAST pds.xml_model[5]);
    // Add it before the root element
    xmlAddPrevSibling(root_node, pi);

    if(verbose)fprintf(stderr,"main() opening <Identification_Area> node element step 1 \n");
    xmlNewChild(root_node, NULL, BAD_CAST pds.po->ia->name, BAD_CAST NULL); /* Identification_Area node element */
    p1=root_node->children;
    xmlNewChild(p1, NULL, BAD_CAST logical_identifier.name, BAD_CAST logical_identifier.value);
    xmlNewChild(p1, NULL, BAD_CAST version_id.name, BAD_CAST version_id.value);
    xmlNewChild(p1, NULL, BAD_CAST title.name, BAD_CAST title.value);
    xmlNewChild(p1, NULL, BAD_CAST information_model_version.name, BAD_CAST information_model_version.value);
    xmlNewChild(p1, NULL, BAD_CAST product_class.name, BAD_CAST product_class.value);
    p1=xmlNewChild(p1, NULL, BAD_CAST modification_history.name, BAD_CAST NULL);
    p1=xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0].name, BAD_CAST NULL);
    xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0].leaves[0].name, BAD_CAST modification_history.leaves[0].leaves[0].value);
    xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0].leaves[1].name, BAD_CAST modification_history.leaves[0].leaves[1].value);
    xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0].leaves[2].name, BAD_CAST modification_history.leaves[0].leaves[2].value);
    p1=p1->parent;p1=p1->parent;p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Observation_Area> node element 1.5\n");
    p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->oa->name, BAD_CAST NULL); /* Observation_Area node element */
	p1 = xmlNewChild(p1, NULL, BAD_CAST oa.leaves[1].name, BAD_CAST oa.leaves[1].value); /* comment */
	p1=p1->parent;
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
	p1=p1->parent;p1=p1->parent; /* close Investigation_Area */
    if(verbose)fprintf(stderr,"main() opening <Observing_System> 2.5\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.name, BAD_CAST NULL); /* <Observing_System>*/
//	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.leaves[0].name, BAD_CAST BAD_CAST observing_system.leaves[0].value); //<Observing_System_Component>
	for(i=0;i<nosc;i++){
    if(verbose)fprintf(stderr,"main() opening <Observing_System_Component> n. %d\n", i);
		p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].ename, BAD_CAST NULL); //<Observing_System_Component>
		xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].name->name, BAD_CAST observing_system.osc[i].name->value);
		xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].type->name, BAD_CAST observing_system.osc[i].type->value);
		xmlNewChild(p1, NULL, BAD_CAST "description", BAD_CAST observing_system.osc[i].descr);
		p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].ir.name, BAD_CAST NULL); /* Investigation_Area.Internal_Reference */
		xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].ir.lid_reference.name, BAD_CAST observing_system.osc[i].ir.lid_reference.value);
		xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[i].ir.reference_type.name, BAD_CAST observing_system.osc[i].ir.reference_type.value);
		p1=p1->parent;p1=p1->parent;
	}
	p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Target_Identification> 3\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST oa.target[0].ename, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].name.name, BAD_CAST oa.target[0].name.value);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].type.name, BAD_CAST oa.target[0].type.value); /* IA.Internal_Reference */
	p1=xmlNewChild(p1, NULL, BAD_CAST oa.target[0].iref[0].name, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].iref->lid_reference.name, BAD_CAST oa.target[0].iref->lid_reference.value);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].iref->reference_type.name, BAD_CAST oa.target[0].iref->reference_type.value);
	p1=p1->parent;	p1=p1->parent;

    if(verbose)fprintf(stderr,"main() opening <Mission_Area> 3.25\n");

/*
	res=fscanf(cfp,"MISSION %s\n",mission); // MISSION
	if(verbose)fprintf(stderr,"MISSION: %s\n",mission);
	// START Mission Information
	res=fscanf(cfp,"MISS_ID %s\n",miss_id); // MISS_ID (element name)
	res=fscanf(cfp,"PHASE %s\n",mission_area.leaves[0].leaves[2].value); // mission phase


	strcpy((char*)mission_area.name,"Mission_Area");
	strcpy((char*)mission_area.leaves[0].name,"psa:Mission_Information");
	strcpy((char*)mission_area.leaves[0].leaves[0].name,"psa:Mission_Phase");
	strcpy((char*)mission_area.leaves[0].leaves[0].leaves[0].name,"psa:name");
	strcpy((char*)mission_area.leaves[0].leaves[0].leaves[0].value,"Science Phase");
	strcpy((char*)mission_area.leaves[0].leaves[0].leaves[1].name,"psa:id");
	strcpy((char*)mission_area.leaves[0].leaves[0].leaves[1].value,"psp");

	            <psa:Mission_Information>
                <psa:Mission_Phase>
                    <psa:name>Science Phase</psa:name>
                    <psa:id>psp</psa:id>
                </psa:Mission_Phase>


*/
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.name, BAD_CAST NULL); /* mission area */
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].name, BAD_CAST NULL); /* psa:Mission_Information */
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[0].name, BAD_CAST NULL); /* psa:Mission_Phase */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[0].leaves[0].name, BAD_CAST mission_area.leaves[0].leaves[0].leaves[0].value); /* psa:name */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[0].leaves[1].name, BAD_CAST mission_area.leaves[0].leaves[0].leaves[1].value); /* psa:id */
		p1=p1->parent;
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[1].name, BAD_CAST mission_area.leaves[0].leaves[1].value); /* start_orbit_number */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[0].leaves[2].name, BAD_CAST mission_area.leaves[0].leaves[2].value); /* stop_orbit_number */
		p1=p1->parent;
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[1].name, BAD_CAST NULL); // psa:Sub-Instrument
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[1].leaves[0].name, BAD_CAST mission_area.leaves[1].leaves[0].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[1].leaves[1].name, BAD_CAST mission_area.leaves[1].leaves[1].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[1].leaves[2].name, BAD_CAST mission_area.leaves[1].leaves[2].value);
		p1=p1->parent;
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].name, BAD_CAST NULL); // psa:Observation_Context
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[0].name, BAD_CAST mission_area.leaves[2].leaves[0].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[1].name, BAD_CAST mission_area.leaves[2].leaves[1].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[2].name, BAD_CAST mission_area.leaves[2].leaves[2].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[2].leaves[3].name, BAD_CAST mission_area.leaves[2].leaves[3].value);
		p1=p1->parent;
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[3].name, BAD_CAST NULL); // psa:Processing_Context
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[3].leaves[0].name, BAD_CAST mission_area.leaves[3].leaves[0].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[3].leaves[1].name, BAD_CAST mission_area.leaves[3].leaves[1].value);
		p1=p1->parent;
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].name, BAD_CAST NULL); // em16_tgo_cas:Cassis_Data
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[0].name, BAD_CAST NULL); /* psa:Instrument_Information */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[0].leaves[0].name, BAD_CAST mission_area.leaves[4].leaves[0].leaves[0].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[0].leaves[1].name, BAD_CAST mission_area.leaves[4].leaves[0].leaves[1].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[0].leaves[2].name, BAD_CAST mission_area.leaves[4].leaves[0].leaves[2].value);
		p1=p1->parent;
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[1].name, BAD_CAST NULL); /* psa:HK_Derived_Data */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[1].leaves[0].name, BAD_CAST mission_area.leaves[4].leaves[1].leaves[0].value);
		p1=p1->parent;
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[2].name, BAD_CAST NULL); /* psa:Telescope_Information */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[2].leaves[0].name, BAD_CAST mission_area.leaves[4].leaves[2].leaves[0].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[2].leaves[1].name, BAD_CAST mission_area.leaves[4].leaves[2].leaves[1].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[2].leaves[2].name, BAD_CAST mission_area.leaves[4].leaves[2].leaves[2].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[2].leaves[3].name, BAD_CAST mission_area.leaves[4].leaves[2].leaves[3].value);
		p1=p1->parent;
		p1 = xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[3].name, BAD_CAST NULL); /* psa:Telescope_Information */
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[3].leaves[0].name, BAD_CAST mission_area.leaves[4].leaves[3].leaves[0].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[3].leaves[1].name, BAD_CAST mission_area.leaves[4].leaves[3].leaves[1].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[3].leaves[2].name, BAD_CAST mission_area.leaves[4].leaves[3].leaves[2].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[3].leaves[3].name, BAD_CAST mission_area.leaves[4].leaves[3].leaves[3].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[3].leaves[4].name, BAD_CAST mission_area.leaves[4].leaves[3].leaves[4].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[3].leaves[5].name, BAD_CAST mission_area.leaves[4].leaves[3].leaves[5].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[3].leaves[6].name, BAD_CAST mission_area.leaves[4].leaves[3].leaves[6].value);
		xmlNewChild(p1, NULL, BAD_CAST mission_area.leaves[4].leaves[3].leaves[7].name, BAD_CAST mission_area.leaves[4].leaves[3].leaves[7].value);
		p1=p1->parent;p1=p1->parent;p1=p1->parent;p1=p1->parent;

		if(verbose)fprintf(stderr,"main() opening <Reference_List> node element step 3.3\n");
		p1 = xmlNewChild(p1, NULL, BAD_CAST reference_list.ename, BAD_CAST NULL); /*  <Reference_List> */
		for(i=0;i<3;i++){
			p1 = xmlNewChild(p1, NULL, BAD_CAST reference_list.irefs[i].name, BAD_CAST NULL); // Internal_Reference
			xmlNewChild(p1, NULL, BAD_CAST reference_list.irefs[i].lid_reference.name, BAD_CAST BAD_CAST reference_list.irefs[i].lid_reference.value); // <lid_reference> element
			xmlNewChild(p1, NULL, BAD_CAST reference_list.irefs[i].reference_type.name, BAD_CAST BAD_CAST reference_list.irefs[i].reference_type.value); // <reference_type> element
			p1=p1->parent;
		}
			p1 = xmlNewChild(p1, NULL, BAD_CAST reference_list.spe.name, BAD_CAST NULL); // Source_Product_External
			xmlNewChild(p1, NULL, BAD_CAST reference_list.spe.external_source_product_identifier.name, BAD_CAST BAD_CAST reference_list.spe.external_source_product_identifier.value); // <external_source_product_identifie> element
			xmlNewChild(p1, NULL, BAD_CAST reference_list.spe.reference_type.name, BAD_CAST BAD_CAST reference_list.spe.reference_type.value); // <reference_type> element
			xmlNewChild(p1, NULL, BAD_CAST reference_list.spe.curating_facility.name, BAD_CAST BAD_CAST reference_list.spe.curating_facility.value); // <curating_facility> element
//			xmlNewChild(p1, NULL, BAD_CAST reference_list.spe.curating_facility.name, BAD_CAST BAD_CAST reference_list.spe.curating_facility.value); // <curating_facility> element
			p1=p1->parent;
/*	strcpy((char*)reference_list.spe.name, "Source_Product_External");
	strcpy((char*)reference_list.spe.external_source_product_identifier.name, "external_source_product_identifier");
	strcpy((char*)reference_list.spe.reference_type.name, "reference_type");
	strcpy((char*)reference_list.spe.curating_facility.name, "curating_facility"); */
		// reference_list.irefs[0].lid_reference.name

		p1=p1->parent; /* up just under the <Product_Observational> root */

		/* Discipline_Area */
/*	p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.name, BAD_CAST NULL);  <Discipline_Area> */
/*	p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.leaves[0].name, BAD_CAST NULL);  <geom:Geometry> */
/*	p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.leaves[0].leaves[0].name, BAD_CAST NULL); geom:SPICE_Kernel_Files*/
/*	p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.leaves[0].leaves[0].leaves[0].name, BAD_CAST NULL);<SKIdent>*/
/*	p1=p1->parent;p1=p1->parent;p1=p1->parent;p1=p1->parent;
	for(i=0;i<251;i++){
		p1 = xmlNewChild(p1, NULL, BAD_CAST discipline_area.leaves[0].leaves[0].leaves[0].leaves[i].name, BAD_CAST \
		discipline_area.leaves[0].leaves[0].leaves[0].leaves[i].value);
		p1=p1->parent;
	}
*/
    if(verbose)fprintf(stderr,"main() opening <File_Area_Observational> node element step 3.5\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->fao->name, BAD_CAST NULL); /* File_Area_Observational node element */
	p1 = xmlNewChild(p1, NULL, BAD_CAST file.name, BAD_CAST NULL); /* opening <File> element */
	xmlNewChild(p1, NULL, BAD_CAST file.leaves[0].name, BAD_CAST file.leaves[0].value); /* <file_name> element */
	/* add other elements */
// 	xmlNewChild(p1, NULL, BAD_CAST file.leaves[1].name, BAD_CAST "TBD1"); /* <local_identifier> element */
// 	xmlNewChild(p1, NULL, BAD_CAST file.leaves[2].name, BAD_CAST "TBD2"); /* <creation_date_time> element */
// 	xmlNewChild(p1, NULL, BAD_CAST file.leaves[3].name, BAD_CAST "TBD3"); /* <file_size> element */
// 	xmlNewChild(p1, NULL, BAD_CAST file.leaves[4].name, BAD_CAST "TBD4"); /* <md5_checksum> element */
// 	xmlNewChild(p1, NULL, BAD_CAST file.leaves[5].name, BAD_CAST "TBD5"); /* <comment> element */

	xmlNewChild(p1, NULL, BAD_CAST file.leaves[1].name, BAD_CAST file.leaves[1].value); /* <local_identifier> element */
	xmlNewChild(p1, NULL, BAD_CAST file.leaves[2].name, BAD_CAST file.leaves[2].value); /* <creation_date_time> element */
	p1 = xmlNewChild(p1, NULL, BAD_CAST file.leaves[3].name, BAD_CAST file.leaves[3].value); /* <file_size> element */
	attr = xmlSetProp(p1, (const xmlChar *)file.leaves[3].attributes[0].name,(const xmlChar *)file.leaves[3].attributes[0].value); /* unit="bytes" attribute */
	p1=p1->parent;
	xmlNewChild(p1, NULL, BAD_CAST file.leaves[4].name, BAD_CAST file.leaves[4].value); /* <md5_checksum> element */
	xmlNewChild(p1, NULL, BAD_CAST file.leaves[5].name, BAD_CAST file.leaves[5].value); /* <comment> element */
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
    if(verbose)fprintf(stderr,"main() opening <Axis_Array> Line/height/y/faster step 3.7\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4].name, BAD_CAST NULL);/*Line/height/y/slower <Axis_Array>*/
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4].leaves[0].name, BAD_CAST array2d.leaves[4].leaves[0].value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4].leaves[1].name, BAD_CAST array2d.leaves[4].leaves[1].value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4].leaves[2].name, BAD_CAST array2d.leaves[4].leaves[2].value);
	p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Axis_Array> Sample/width/x/slower step 4\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[5].name, BAD_CAST NULL);/*Sample/width/x/faster <Axis_Array>*/
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
	free(modification_history.leaves[0].leaves);
	free(modification_history.leaves);
	free(prodfnam);
	for(i=0;i<2;i++){
		free(observing_system.osc[i].name);
		free(observing_system.osc[i].type);
	}
	free(mission_area.leaves->leaves);
	free(mission_area.leaves);
	free(reference_list.irefs);
	free(file.leaves[3].attributes);
	free(file.leaves);
	for(i=0;i<6;i++)
		free(array2d.leaves[i].leaves);
	// prodfname freed 
	free(oa.target[0].iref);
	free(oa.target);
	free(po.attributes); // po.attributes freed
	fprintf(stderr,"main() ending program\n");
  return 0;
}
