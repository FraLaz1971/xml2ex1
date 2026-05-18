#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/tree.h>
#include "pds.h"
#include "cassis.h"

/** create_conf_main() is the main method of the create_conf program */
int create_conf_main(char iconf[MAXFNAML], char oconf[MAXFNAML])
{	int verbose = 1;
	int res;
	res=create_conf(iconf, oconf);
	return res;
}
/** 
the real c language main() function 

usage: ./create_conf <infile.conf> <outfile.conf>

e.g: ./create_conf arpds.conf arpds_002.conf

*  
takes in input an old configuration file and saves as output the new configuration file
*
* */
int main(int argc, char **argv)
{	
	char iconf[MAXFNAML]; char oconf[MAXFNAML];
	int res;
	if(argc<3){
		fprintf(stderr,"usage:%s <infile.conf> <outfile.conf>\n",argv[0]);
		fprintf(stderr,"e.g: %s arpds.conf arpds_002.conf \n",argv[0]);
		return 1;
	}
	
	strcpy(iconf,argv[1]);strcpy(oconf,argv[2]);
	
	res = create_conf_main((char *)iconf, (char *)oconf);
	return res;
	
}

/* function to create a new input configuration file */

int create_conf(char iconf[MAXFNAML], char oconf[MAXFNAML]){
	int verbose = 1;
	/* struct containing the PamDataset metadata */
	struct PAMDATASET mypam;

	/* struct containing the PDS(4) metadata */
	struct PDS mypds;
	/* current value of KEY and VALUE */
    char key[256];
    char val[MAXFNAML];
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
    unsigned int width = 0;  /** unsigned int width Sample == n. of columns */
    unsigned int height = 0; /** unsigned int width Line == n. of rows */
    const char* confname;
    int j;
    char buf[MAXFNAML];
    int i, res, numproducts, npi, npoattr, nosc, nci, nrlir;
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
	struct PAMDATASET pam_ds;

    FILE *icfp, *ocfp;
	FILE *lp; /* pds4 label file handler */
	char **prodfnam;

	/** allocate memory for metadata structures (classes) */
	
	mypds.po = (struct PRODUCT_OBSERVATIONAL*)malloc(sizeof(struct PRODUCT_OBSERVATIONAL));
	mypds.po->ia = (struct IDENTIFICATION_AREA*)malloc(sizeof(struct IDENTIFICATION_AREA));
	mypds.po->oa = (struct OBSERVATION_AREA*)malloc(sizeof(struct OBSERVATION_AREA));
	mypds.po->rl = (struct REFERENCE_LIST*)malloc(sizeof(struct REFERENCE_LIST));
	mypds.po->fao = (struct FILE_AREA_OBSERVATIONAL*)malloc(sizeof(struct FILE_AREA_OBSERVATIONAL));
	
	/* allocate memory old part */

	/* allocate memory for nodes */
	oa.target=(struct TARGET_IDENTIFICATION*)malloc(3*sizeof(struct TARGET_IDENTIFICATION));
	oa.target->iref=(struct INTERNAL_REFERENCE*)malloc(sizeof(struct INTERNAL_REFERENCE));
	array2d.leaves[0].attributes=(struct ATTRIBUTE *)malloc(sizeof(struct ATTRIBUTE));	
	po.attributes=(struct ATTRIBUTE *)malloc(MAXLEAV*sizeof(struct ATTRIBUTE));
	logical_identifier.leaves=(struct ELEMENT *)malloc(5*sizeof(struct ELEMENT));
	for(i=0;i<5;i++)
		logical_identifier.leaves[i].leaves=(struct ELEMENT *)malloc(5*sizeof(struct ELEMENT));
	modification_history.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	modification_history.leaves[0].leaves=(struct ELEMENT *)malloc(3*sizeof(struct ELEMENT));
	primary_result_summary.leaves=(struct ELEMENT *)malloc(4*sizeof(struct ELEMENT));
	primary_result_summary.leaves[3].leaves=(struct ELEMENT *)malloc(3*sizeof(struct ELEMENT));
	mission_area.leaves=(struct ELEMENT *)malloc(MAXLEAV*sizeof(struct ELEMENT));
	for(i=0;i<MAXLEAV;i++)
		mission_area.leaves[i].leaves=(struct ELEMENT *)malloc(MAXLEAV*sizeof(struct ELEMENT));

	mission_area.leaves[0].leaves[0].leaves=(struct ELEMENT *)malloc(MAXLEAV*sizeof(struct ELEMENT)); /** psa:Mission_Phase */
	mission_area.leaves[0].leaves[0].leaves[0].leaves=(struct ELEMENT *)malloc(MAXLEAV*sizeof(struct ELEMENT)); /**  psa:name
	
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
	for(i=0;i<MAXLEAV;i++) /** max number of products files = MAXLEAV */
		array2d.leaves[i].leaves=(struct ELEMENT*)malloc(MAXFNAML*sizeof(struct ELEMENT));
	prodfnam=(char **)malloc(numproducts*sizeof(char *));
	for(i=0;i<numproducts;i++)
		prodfnam[i]=(char *)malloc(MAXFNAML);
	po.attributes=(struct ATTRIBUTE *)malloc(MAXLEAV*sizeof(struct ATTRIBUTE));
	pds.products=(FILE**)malloc(numproducts*sizeof(FILE *));
	/** observing_system.osc=(struct OBSERVING_SYSTEM_COMPONENT*)malloc(2*sizeof(struct OBSERVING_SYSTEM_COMPONENT)); */
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

	/** set default values */
   strcpy(moddate,"def1" );
   strcpy(svers, "def2");
   strcpy(descr, "def3");
   strcpy(tstart, "1970-01-01T00:00:00Z");
   strcpy(tstop, "1970-01-01T00:00:00Z");
	/** read configuration file */


	/** open  input configuration file for reading */
  icfp=fopen(iconf, "r");
  if(icfp==NULL){
     perror("cannot open input configuration file for reading");
     exit(1);
  }

  	/** open  output configuration file for writing */
  ocfp=fopen(oconf, "w");
  if(ocfp==NULL){
     perror("cannot open output configuration file for writing");
     exit(1);
  }

  /** read input configuration file content and save it into variables in volatile memory (RAM) */

    res=fscanf(icfp, "MISSION_NAME %s\n",mypam.mission);
    for(j=0;j<strlen(mypam.mission);j++)
 		if(mypam.mission[j]=='>') mypam.mission[j]=' ';
    res=fscanf(icfp, "SPACECRAFT_NAME %s\n",mypam.spacecraft);
    res=fscanf(icfp, "TARGET_NAME %s\n",mypam.target);
    res=fscanf(icfp, "INSTRUMENT_ID %s\n",mypam.instrid);
    res=fscanf(icfp, "PRODUCT_TYPE %s\n",mypam.prodtype);

     res=fscanf(icfp, "BITPIX %hu\n",&bitpix);
   res=fscanf(icfp, "SIGN %10s\n",buf);
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
   res=fscanf(icfp, "ENDIAN %3s\n",buf);
   if(!(strcmp(buf,"LSB"))){
   	end = 0;
   } else if(strcmp(buf,"MSB")) {
 	end = 1;
   } else{
     perror("illegal endianness option read");
     exit(1);
   }
   strcpy(send,buf);
   res=fscanf(icfp,  "WIDTH %u\n",&width);
   res=fscanf(icfp, "HEIGHT %u\n",&height);
   if(bitpix>8) {
 	sprintf(dtype,"%s%s%u",ssign,send,(unsigned int)bitpix/8);
  } else if (bitpix==8){
 	 (sign==1)?strcpy(dtype,"SignedByte"):strcpy(dtype,"UnsignedByte");
  }
   sprintf(swidth,"%u",width);
   sprintf(sheight,"%u",height);
     /** read Processing Instructions keys */
 	  res=fscanf(icfp,"NPI %d\n",&npi);
   if (verbose) fprintf(stderr,"n. of Processing Instructions: %d\n",npi);
   for(i=0;i<npi;i++){
 	res=fscanf(icfp,"%5s %s\n",key,pds.xml_model[i]);
 	for(j=0;j<strlen(pds.xml_model[i]);j++)
 		if(pds.xml_model[i][j]=='>') pds.xml_model[i][j]=' ';
 	if (verbose) fprintf(stderr,"key: %5s  value:%s\n",key,pds.xml_model[i]);
   }
   res=fscanf(icfp,"NPOATTR %d\n",&npoattr);
   if (verbose) fprintf(stderr,"n. of <Product_Observational> attributes: %d\n",npoattr);
   for(i=0;i<npoattr;i++){
 	res=fscanf(icfp,"%15s %s\n",key,po.attributes[i].name);
 	res=fscanf(icfp,"%16s %s\n",key,po.attributes[i].value);
 	for(j=0;j<strlen(po.attributes[i].value);j++)
 		if(po.attributes[i].value[j]=='>') po.attributes[i].value[j]=' ';
 	if (verbose) fprintf(stderr,"%16s  %s\n",po.attributes[i].name,po.attributes[i].value);
   }
   if (verbose) fprintf(stderr,"going to read configuration file about Identification_Area\n");
 	/** read Identification_Area keys */


   res=fscanf(icfp,"PROD_LID %s\n",prodlid[0]);
   if (verbose) fprintf(stderr,"read PROD_LID\n");
   strcpy(logical_identifier.leaves[0].value,prodlid[0]);
   res=fscanf(icfp,"PROD_VID %s\n",prodvid[0]);
   if (verbose) fprintf(stderr,"read PROD_VID\n");
   strcpy(logical_identifier.leaves[1].value,prodvid[0]);
   res=fscanf(icfp,"TITLE %s\n",stitle);
   if (verbose) fprintf(stderr,"read TITLE\n");
   for(j=0;j<strlen(stitle);j++)
 		if(stitle[j]=='>') stitle[j]=' ';
   strcpy(logical_identifier.leaves[2].value,stitle);
   res=fscanf(icfp,"IM_VERS %s\n",imvers);
   if (verbose) fprintf(stderr,"read IM_VERS\n");
   strcpy(logical_identifier.leaves[3].value,imvers);
   res=fscanf(icfp,"PROD_CLASS %s\n",prodclass);
   if (verbose) fprintf(stderr,"read PROD_CLASS\n");
   strcpy(logical_identifier.leaves[4].value,prodclass);

   if (verbose) fprintf(stderr,"going to read configuration file about Citation_Information\n");
 	/* read Citation_Information keys */
   res=fscanf(icfp,"NCI %d\n",&nci);
   if (verbose) fprintf(stderr,"Number of Citation Information = %d\n", nci);
   citation_information.leaves=(struct ELEMENT *)malloc(nci*sizeof(struct ELEMENT));
   for(i=0;i<nci;i++){
 	citation_information.leaves[i].leaves=(struct ELEMENT *)malloc(8*sizeof(struct ELEMENT));
 	res=fscanf(icfp,"%s %s\n",key,citation_information.leaves[i].leaves[0].value);
 	for(j=0;j<strlen(citation_information.leaves[i].leaves[0].value);j++)
 		if(citation_information.leaves[i].leaves[0].value[j]=='>') citation_information.leaves[i].leaves[0].value[j]=' ';
 	if(verbose)fprintf(stderr,"read author list: %s\n",citation_information.leaves[i].leaves[0].value);
 	res=fscanf(icfp,"%s %s\n",key,citation_information.leaves[i].leaves[1].value);
 	if(verbose)fprintf(stderr,"read publication year %s\n",citation_information.leaves[i].leaves[1].value);
 	res=fscanf(icfp,"%s %s\n",key,citation_information.leaves[i].leaves[2].value);
 	if(verbose)fprintf(stderr,"read doi: %s\n",citation_information.leaves[i].leaves[2].value);
 	res=fscanf(icfp,"%s %s\n",key,citation_information.leaves[i].leaves[3].value);
 	if(verbose)fprintf(stderr,"read key 0: %s\n",citation_information.leaves[i].leaves[3].value);
 	res=fscanf(icfp,"%s %s\n",key,citation_information.leaves[i].leaves[4].value);
 	if(verbose)fprintf(stderr,"read key 1: %s\n",citation_information.leaves[i].leaves[4].value);
 	res=fscanf(icfp,"%s %s\n",key,citation_information.leaves[i].leaves[5].value);
 	if(verbose)fprintf(stderr,"read key 2: %s\n",citation_information.leaves[i].leaves[5].value);
 	res=fscanf(icfp,"%s %s\n",key,citation_information.leaves[i].leaves[6].value);
 	if(verbose)fprintf(stderr,"read key 3: %s\n",citation_information.leaves[i].leaves[6].value);
 	res=fscanf(icfp,"%s %s\n",key,citation_information.leaves[i].leaves[7].value);
 	for(j=0;j<strlen(citation_information.leaves[i].leaves[7].value);j++)
 		if(citation_information.leaves[i].leaves[7].value[j]=='>') citation_information.leaves[i].leaves[7].value[j]=' ';
 		if(verbose)fprintf(stderr,"read description: %s\n",citation_information.leaves[i].leaves[7].value);
   }
   if (verbose) fprintf(stderr,"going to read configuration file about Modification_History\n");
   /** read Modification_History keys */
   res=fscanf(icfp,"MOD_DATE %s\n",moddate);
   res=fscanf(icfp,"VERSID %s\n",svers);
   res=fscanf(icfp,"DESCR %s\n",descr);
 	for(j=0;j<strlen(descr);j++)
 		if(descr[j]=='>') descr[j]=' ';
   if (verbose) fprintf(stderr,"read MOD_DATE = %s\n", moddate);
   if (verbose) fprintf(stderr,"read VERSID = %s\n", svers);
   if (verbose) fprintf(stderr,"read DESCR = %s\n", descr);
   /** read Observation Area keys */
   if (verbose) fprintf(stderr,"going to read configuration file about Observation_Area\n");
   fprintf(stderr,"going to read configuration file about Time_Coordinates\n");
   res=fscanf(icfp,"TSTART %s\n",tstart);
   res=fscanf(icfp,"TSTOP %s\n",tstop);
   /****************************************/
   /** printout variables content for debug */
   /****************************************/
   /* new keys 2026-05-14 */
   if (verbose) fprintf(stderr,"Now printing variables content for debug\n");
   if (verbose) fprintf(stderr,"MISSION_NAME = %s\n",mypam.mission);
   if (verbose) fprintf(stderr,"SPACECRAFT_NAME = %s\n",mypam.spacecraft);
   if (verbose) fprintf(stderr,"TARGET_NAME = %s\n",mypam.target);
   if (verbose) fprintf(stderr,"INSTRUMENT_ID = %s\n",mypam.instrid);
   if (verbose) fprintf(stderr,"PRODUCT_TYPE = %s\n",mypam.prodtype);
   if (verbose) fprintf(stderr,"BITPIX = %d\n",bitpix);
   if (verbose) fprintf(stderr,"SIGN = %d\n",sign);
   if (verbose) fprintf(stderr,"ENDIAN = %d\n",end);
   if (verbose) fprintf(stderr,"WIDTH = %u\n",width);   /* Sample == n. of columns */
   if (verbose) fprintf(stderr,"HEIGHT = %u\n",height); /* Line == n. of rows */
   if (verbose) fprintf(stderr,"PDS4 <data_type> = %s\n",dtype);
   if (verbose) fprintf(stderr,"TSTART = %s\n",tstart);/** TSTART: timestamp of measurements start */
   if (verbose) fprintf(stderr,"TSTOP = %s\n",tstop);  /** TSTOP: timestamp of measurements stop  */
   if (verbose) fprintf(stderr,"going to read configuration file about Primary_Result_Summary\n");
   res=fscanf(icfp,"PURP %s\n",purp);
   res=fscanf(icfp,"PROC_LEV %s\n",proclev);
   res=fscanf(icfp,"RES_DESCR %s\n",resdescr);
   for(j=0;j<strlen(resdescr);j++)
 		if(resdescr[j]=='>') resdescr[j]=' ';
   res=fscanf(icfp,"LAMBDA %s\n",lambda);
   res=fscanf(icfp,"DOMAIN %s\n",domain);
   res=fscanf(icfp,"DISCIPL %s\n",discipl);
   if (verbose) fprintf(stderr,"going to read configuration file about Investigation_Area\n");
   res=fscanf(icfp,"INVEST_AREA %s\n",iaa.name.value);
   for(j=0;j<strlen(iaa.name.value);j++)
 		if(iaa.name.value[j]=='>') iaa.name.value[j]=' ';
   if (verbose) fprintf(stderr, "read INVEST_AREA res = %d value = %s\n", res, iaa.name.value);
   res=fscanf(icfp,"INVEST_TYPE %s\n",iaa.type.value);
   if (verbose) fprintf(stderr, "read INVEST_TYPE res = %d value = %s\n", res, iaa.type.value);
   res=fscanf(icfp,"INV_LID %s\n",invlid);
   res=fscanf(icfp,"REFTYPE %s\n",reftype);
   if (verbose) fprintf(stderr,"going to read configuration file about Observing_System\n");
   res=fscanf(icfp,"OSNAME %s\n",observing_system.value);
   res=fscanf(icfp,"NOSC_COMP %d\n",&nosc);
   for(i=0;i<nosc;i++){
 	res=fscanf(icfp,"%11s %s\n",key,onamval[i]);
 	for(j=0;j<strlen(onamval[i]);j++)
 		if(onamval[i][j]=='>') onamval[i][j]=' ';
 	if(verbose)fprintf(stderr,"OSC[%d] n. value: %s\n",i,onamval[i]);
 	res=fscanf(icfp,"%s %s\n",key,otypval[i]);
 	if(verbose)fprintf(stderr,"OSC[%d] n. type: %s\n",i,otypval[i]);
 	res=fscanf(icfp,"%s %s\n",key,observing_system.osc[i].descr);
 	for(j=0;j<strlen(observing_system.osc[i].descr);j++)
 		if(observing_system.osc[i].descr[j]=='>') observing_system.osc[i].descr[j]=' ';
 	if(verbose)fprintf(stderr,"OSC[%d] n. description: %s\n",i,observing_system.osc[i].descr);
 	res=fscanf(icfp,"%s %s\n",key,observing_system.osc[i].ir.lid_reference.value);
 	if(verbose)fprintf(stderr,"OSC[%d] n. lid_reference: %s\n",i,observing_system.osc[i].ir.lid_reference.value);
 	res=fscanf(icfp,"%s %s\n",key,observing_system.osc[i].ir.reference_type.value);
 	if(verbose)fprintf(stderr,"OSC[%d] n. reference_type: %s\n",i,observing_system.osc[i].ir.reference_type.value);
   }
   /** target identification */
 	oa.target[0].iref[0].lid_reference.value[0] = 'A';
 	oa.target[0].iref[0].reference_type.value[0] = 'B';
 	oa.target[0].iref[0].lid_reference.name[0] = 'C';
 	oa.target[0].iref[0].reference_type.name[0] = 'D';
 	res=fscanf(icfp,"TARGET_NAME %s\n",oa.target[0].name.value); /*  */
 	res=fscanf(icfp,"TARGET_TYPE %s\n",oa.target[0].type.value); /*  */
 	res=fscanf(icfp,"%s %s\n",key,oa.target[0].iref[0].lid_reference.value);
 	if(verbose)fprintf(stderr,"TARGET[%d] n. lid_reference: %s\n",0,oa.target[0].iref[0].lid_reference.value);
 	res=fscanf(icfp,"%s %s\n",key,oa.target[0].iref[0].reference_type.value);
 	if(verbose)fprintf(stderr,"TARGET[%d] n. reference_type: %s\n",0,oa.target[0].iref[0].reference_type.value);
   /** read Mission Area configuration */
 	res=fscanf(icfp,"MISSION %s\n",mission); /* MISSION */
 	if(verbose)fprintf(stderr,"MISSION: %s\n",mission);
 	/** START reading Mission Information
 	 * res=fscanf(icfp,"MISS_ID %s\n",miss_id); /* MISS_ID (element name)
 	psa:Mission_Information
 */
 	strcpy((char*)miss_id, "psa:Mission_Information");
 	if(verbose)fprintf(stderr,"Before PHASE\n");
 	/* <psa:Mission_Information>  ---> mission_area.leaves[0] */
 												  /** MissInf.MissPhase.PhaseName */
 	res=fscanf(icfp,"PHASE_NAME %s\n",mission_area.leaves[0].leaves[0].leaves[0].value); /* mission phase name */
 	for(j=0;j<strlen(mission_area.leaves[0].leaves[0].leaves[0].value);j++)
 		if(mission_area.leaves[0].leaves[0].leaves[0].value[j]=='>') mission_area.leaves[0].leaves[0].leaves[0].value[j]=' ';
 	if (verbose) fprintf(stderr, "read PHASE_NAME res = %d value = %s\n", res, mission_area.leaves[0].leaves[0].leaves[0].value);
 												/** MissInf.MissPhase.PhaseID */
 	res=fscanf(icfp,"PHASE_ID %s\n",mission_area.leaves[0].leaves[0].leaves[1].value); /* mission phase id */
 	for(j=0;j<strlen(mission_area.leaves[0].leaves[0].leaves[1].value);j++)
 		if(mission_area.leaves[0].leaves[0].leaves[1].value[j]=='>') mission_area.leaves[0].leaves[0].leaves[1].value[j]=' ';
 	if (verbose) fprintf(stderr, "read PHASE_ID res = %d value = %s\n", res, mission_area.leaves[0].leaves[0].leaves[1].value);
 	res=fscanf(icfp,"CLOCK_START %s\n",mission_area.leaves[0].leaves[0].value); /* clock tstart */
 	if (verbose) fprintf(stderr, "read CLOCK_START res = %d value = %s\n", res, mission_area.leaves[0].leaves[0].value);
 	res=fscanf(icfp,"CLOCK_STOP %s\n",mission_area.leaves[0].leaves[1].value); /* clock tsop */
 	if (verbose) fprintf(stderr, "read CLOCK_STOP res = %d value = %s\n", res, mission_area.leaves[0].leaves[1].value);
 												/** MissInf.StartOrbit */
 	res=fscanf(icfp,"START_ORB %s\n",mission_area.leaves[0].leaves[1].value); /* start orbit */
 	if (verbose) fprintf(stderr, "read START_ORB res = %d value = %s\n", res, mission_area.leaves[0].leaves[1].value);
 												/** MissInf.StopOrbit */
 	res=fscanf(icfp,"STOP_ORB %s\n",mission_area.leaves[0].leaves[2].value); /* stop orbit */
 	if (verbose) fprintf(stderr, "read STOP_ORB res = %d value = %s\n", res, mission_area.leaves[0].leaves[2].value);
 	/* END reading Mission Information */
 	
 	/** START reading Sub Instrument */
 //	res=fscanf(icfp,"SU %s\n",su); /* SUB INSTRUMENT (element name) */
 	strcpy((char*)su, "psa:Sub-Instrument");
 										/* Sub-Instrument.Identifier */
 	res=fscanf(icfp,"SU_ID %s\n",mission_area.leaves[1].leaves[0].value); /* <psa:identifier>  */
 	if (verbose) fprintf(stderr, "read SU_ID res = %d value = %s\n", res, mission_area.leaves[1].leaves[0].value);
 											/* Sub-Instrument.Name */
 	res=fscanf(icfp,"SU_NAME %s\n",mission_area.leaves[1].leaves[1].value); /* <psa:name>  */
 	for(j=0;j<strlen(mission_area.leaves[1].leaves[1].value);j++)
 		if(mission_area.leaves[1].leaves[1].value[j]=='>') mission_area.leaves[1].leaves[1].value[j]=' ';
 	if (verbose) fprintf(stderr, "read SU_NAME res = %d value = %s\n", res, mission_area.leaves[1].leaves[1].value);
 												/* Sub-Instrument.Type */
 	res=fscanf(icfp,"SU_TYPE %s\n",mission_area.leaves[1].leaves[2].value); /* <psa:type>  */
 	if (verbose) fprintf(stderr, "read SU_TYPE res = %d value = %s\n", res, mission_area.leaves[1].leaves[2].value);
 		/* END reading Sub Instrument */
 	/** START reading <psa:Observation_Context> */
 	res=fscanf(icfp,"OBSCON_IPM %s\n",mission_area.leaves[2].leaves[0].value);   /* <psa:instrument_pointing_mode>  */
 	if (verbose) fprintf(stderr, "read OBSCON_IPM res = %d value = %s\n", res, mission_area.leaves[2].leaves[0].value);
 	res=fscanf(icfp,"OBSCON_IPD %s\n",mission_area.leaves[2].leaves[1].value);   /* <psa:instrument_pointing_description> */
 	if (verbose) fprintf(stderr, "read OBSCON_IPD res = %d value = %s\n", res, mission_area.leaves[2].leaves[1].value);
 	res=fscanf(icfp,"OBSCON_OID %s\n",mission_area.leaves[2].leaves[2].value);   /* <psa:observation_identifier> */
 	if (verbose) fprintf(stderr, "read OBSCON_OID res = %d value = %s\n", res, mission_area.leaves[2].leaves[2].value);
 	res=fscanf(icfp,"OBSCON_OTYPE %s\n",mission_area.leaves[2].leaves[3].value); /* <psa:observation_type> */
 	if (verbose) fprintf(stderr, "read OBSCON_OTYPE res = %d value = %s\n", res, mission_area.leaves[2].leaves[3].value);
 /* END reading <psa:Observation_Context> */

 	/** START reading <psa:Processing_Context> */
/**	res=fscanf(icfp,"PRODID %s\n",mission_area.leaves[3].leaves[0].value); /* product id */
/**	if (verbose) fprintf(stderr, "read PRODID res = %d value = %s\n", res, mission_area.leaves[3].leaves[0].value); */
 	res=fscanf(icfp,"SWNAME %s\n",mission_area.leaves[3].leaves[0].value); /* <psa:psa:processing_software_title> (software name) */
 	for(j=0;j<strlen(mission_area.leaves[3].leaves[0].value);j++)
 		if(mission_area.leaves[3].leaves[0].value[j]=='>') mission_area.leaves[3].leaves[0].value[j]=' ';
 	if (verbose) fprintf(stderr, "read SWNAME res = %d value = %s\n", res, mission_area.leaves[3].leaves[1].value);
 	res=fscanf(icfp,"SWVERS %s\n",mission_area.leaves[3].leaves[1].value); /* <psa:processing_software_version>  */
 	if (verbose) fprintf(stderr, "read SWVERS res = %d value = %s\n", res, mission_area.leaves[3].leaves[1].value);

 /** END reading <psa:Processing_Context> */

 	/** read Cassis_Data */
 //	res=fscanf(icfp,"DATA_NAME %s\n",dname); /* instrument:DATA (element name) */
 	strcpy((char*)dname, "em16_tgo_cas:Cassis_Data");
 	res=fscanf(icfp,"INSTR_IFOV %s\n",mission_area.leaves[4].leaves[0].leaves[0].value); /* <em16_tgo_cas:instrument_ifov>  */
 	if (verbose) fprintf(stderr, "read INSTR_IFOV res = %d value = %s\n", res, mission_area.leaves[4].leaves[0].leaves[0].value);
 			                                            /* CassisData.InstrumentInformation.ifov_unit */
 	res=fscanf(icfp,"IFOV_UNIT %s\n",mission_area.leaves[4].leaves[0].leaves[1].value); /* instrument_ifov_unit_description>  */
 	if (verbose) fprintf(stderr, "read IFOV_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[0].leaves[1].value);
 			                                            /* CassisData.InstrumentInformation.filters_available */
 	res=fscanf(icfp,"FILTERS %s\n",mission_area.leaves[4].leaves[0].leaves[2].value); /* <em16_tgo_cas:filters_available>  */
 	for(j=0;j<strlen(mission_area.leaves[4].leaves[0].leaves[2].value);j++)
 		if(mission_area.leaves[4].leaves[0].leaves[2].value[j]=='>') mission_area.leaves[4].leaves[0].leaves[2].value[j]=' ';
 	if (verbose) fprintf(stderr, "read FILTERS res = %d value = %s\n", res, mission_area.leaves[4].leaves[0].leaves[2].value);
 							/*	<em16_tgo_cas:HK_Derived_Data> */
 			                                            /* CassisData.HK_Derived_Data.filter */
 	res=fscanf(icfp,"HK_FILTER %s\n",mission_area.leaves[4].leaves[1].leaves[0].value); /* <em16_tgo_cas:filter>  */
 	if (verbose) fprintf(stderr, "read HK_FILTER res = %d value = %s\n", res, mission_area.leaves[4].leaves[1].leaves[0].value);
 							/*	<em16_tgo_cas:HK_Derived_Data> */

 							/* <em16_tgo_cas:Telescope_Information> */
 												/* CassisData.Telescope_Information.focal_length */
 	res=fscanf(icfp,"FOCAL_LENGTH %s\n",mission_area.leaves[4].leaves[2].leaves[0].value); /* <em16_tgo_cas:focal_length>  */
 	if (verbose) fprintf(stderr, "read FOCAL_LENGTH res = %d value = %s\n", res, mission_area.leaves[4].leaves[2].leaves[0].value);
 												/* CassisData.Telescope_Information.focal_unit */
 	res=fscanf(icfp,"FOCAL_UNIT %s\n",mission_area.leaves[4].leaves[2].leaves[1].value); /* <em16_tgo_cas:focal_length_unit_description>  */
 	if (verbose) fprintf(stderr, "read FOCAL_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[2].leaves[1].value);
 												/* CassisData.Telescope_Information.focal_number */
 	res=fscanf(icfp,"F_NUMBER %s\n",mission_area.leaves[4].leaves[2].leaves[2].value); /* <em16_tgo_cas:focal_length_unit_description>  */
 	if (verbose) fprintf(stderr, "read F_NUMBER res = %d value = %s\n", res, mission_area.leaves[4].leaves[2].leaves[2].value);
 												/* CassisData.Telescope_Information.telescope_description */
 	res=fscanf(icfp,"TELESCOPE %s\n",mission_area.leaves[4].leaves[2].leaves[3].value); /* <em16_tgo_cas:telescope_description>  */
 	for(j=0;j<strlen(mission_area.leaves[4].leaves[2].leaves[3].value);j++)
 		if(mission_area.leaves[4].leaves[2].leaves[3].value[j]=='>') mission_area.leaves[4].leaves[2].leaves[3].value[j]=' ';
 	if (verbose) fprintf(stderr, "read TELESCOPE res = %d value = %s\n", res, mission_area.leaves[4].leaves[2].leaves[3].value);
 	 							/* <em16_tgo_cas:Detector_Information> */
 												/* CassisData.Detector_Information.cassis_description */
 	res=fscanf(icfp,"CAS_DESCR %s\n",mission_area.leaves[4].leaves[3].leaves[0].value); /* <em16_tgo_cas:cassis_description>  */
 	for(j=0;j<strlen(mission_area.leaves[4].leaves[3].leaves[0].value);j++)
 		if(mission_area.leaves[4].leaves[3].leaves[0].value[j]=='>') mission_area.leaves[4].leaves[3].leaves[0].value[j]=' ';
 	if (verbose) fprintf(stderr, "read CAS_DESCR res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[0].value);
 												/* CassisData.Detector_Information.pixel_height */
 	res=fscanf(icfp,"PIX_HEIGHT %s\n",mission_area.leaves[4].leaves[3].leaves[1].value); /* <em16_tgo_cas:pixel_height>  */
 	if (verbose) fprintf(stderr, "read PIX_HEIGHT res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[1].value);
 												/* CassisData.Detector_Information.pixel_height_unit */
 	res=fscanf(icfp,"PIX_HE_UNIT %s\n",mission_area.leaves[4].leaves[3].leaves[2].value); /* <em16_tgo_cas:pixel_height_unit_description>  */
 	if (verbose) fprintf(stderr, "read PIX_HE_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[2].value);
 												/* CassisData.Detector_Information.pixel_width */
 	res=fscanf(icfp,"PIX_WIDTH %s\n",mission_area.leaves[4].leaves[3].leaves[3].value); /* <em16_tgo_cas:pixel_width>  */
 	if (verbose) fprintf(stderr, "read PIX_WIDTH res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[3].value);
 												/* CassisData.Detector_Information.pixel_width_unit */
 	res=fscanf(icfp,"PIX_WI_UNIT %s\n",mission_area.leaves[4].leaves[3].leaves[4].value); /* <em16_tgo_cas:pixel_width_unit_description>  */
 	if (verbose) fprintf(stderr, "read PIX_WI_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[4].value);
 												/* CassisData.Detector_Information.detector_description */
 	res=fscanf(icfp,"DET_DESCR %s\n",mission_area.leaves[4].leaves[3].leaves[5].value); /* <em16_tgo_cas:detector_description>  */
 	for(j=0;j<strlen(mission_area.leaves[4].leaves[3].leaves[5].value);j++)
 		if(mission_area.leaves[4].leaves[3].leaves[5].value[j]=='>') mission_area.leaves[4].leaves[3].leaves[5].value[j]=' ';
 	if (verbose) fprintf(stderr, "read DET_DESCR res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[5].value);
 												/* CassisData.Detector_Information.read_noise */
 	res=fscanf(icfp,"NOISE %s\n",mission_area.leaves[4].leaves[3].leaves[6].value); /* <em16_tgo_cas:read_noise>  */
 	if (verbose) fprintf(stderr, "read NOISE res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[6].value);
 												/* CassisData.Detector_Information.read_noise_unit */
 	res=fscanf(icfp,"NOISE_UNIT %s\n",mission_area.leaves[4].leaves[3].leaves[7].value); /* <em16_tgo_cas:read_noise_unit_description>  */
 	if (verbose) fprintf(stderr, "read NOISE_UNIT res = %d value = %s\n", res, mission_area.leaves[4].leaves[3].leaves[7].value);
 	/* STOP reading Cassis_Data */
 	res=fscanf(icfp,"AV_INT %s\n",mission_area.leaves[2].leaves[0].value); /*  */
 	res=fscanf(icfp,"CAL_TYPE %s\n",mission_area.leaves[2].leaves[0].value); /*  */
 	res=fscanf(icfp,"MEAS_RANGE_IDX %s\n",mission_area.leaves[2].leaves[1].value); /* */
 	res=fscanf(icfp,"MEAS_RANGE %s\n",mission_area.leaves[2].leaves[2].value); /*  */
 	res=fscanf(icfp,"REF_FRAME %s\n",mission_area.leaves[2].leaves[3].value);/*  */
 	res=fscanf(icfp,"SPICE_FRAME %s\n",mission_area.leaves[2].leaves[4].value); /*  */

 	/** read Instrument Status*/
 	res=fscanf(icfp,"STATUS_NAME %s\n",istatus); /*  */
 	res=fscanf(icfp,"SC_STATUS %s\n",mission_area.leaves[3].leaves[0].value); /*  */
 	/* STOP Instrument Status*/
 	res=fscanf(icfp,"NRLIR %d\n",&nrlir); /*  */
 /** NRLIR = number of REFERENCE_LIST Internal Reference items
  ** reference_list.irefs[i] */
   for(i=0;i<nrlir;i++){
 	res=fscanf(icfp,"%11s %s\n",key,reference_list.irefs[i].lid_reference.value);
 	res=fscanf(icfp,"%15s %s\n",key,reference_list.irefs[i].reference_type.value);

   res=fscanf(icfp,"SPE_PRODID %s\n",reference_list.spe.external_source_product_identifier.value);
   if (verbose) fprintf(stderr, "read SPE_PRODID res = %d value = %s\n", res, reference_list.spe.external_source_product_identifier.value);
   res=fscanf(icfp,"SPE_REFTYPE %s\n",reference_list.spe.reference_type.value);
   if (verbose) fprintf(stderr, "read SPE_REFTYPE res = %d value = %s\n", res, reference_list.spe.reference_type.value);
   res=fscanf(icfp,"SPE_CURFAC %s\n",reference_list.spe.curating_facility.value);
 	for(j=0;j<strlen(reference_list.spe.curating_facility.value);j++)
 		if(reference_list.spe.curating_facility.value[j]=='>') reference_list.spe.curating_facility.value[j]=' ';
   if (verbose) fprintf(stderr, "read SPE_CURFAC res = %d value = %s\n", res, reference_list.spe.curating_facility.value);
   for(j=0;j<strlen(reference_list.spe.curating_facility.value);j++)
 		if(reference_list.spe.curating_facility.value[j]=='>') reference_list.spe.curating_facility.value[j]=' ';
   if (verbose)   fprintf(stderr, "now value = %s\n", reference_list.spe.curating_facility.value);
   }
 for(i=0;i<numproducts;i++){
   res=fscanf(icfp,"%11s %s\n",key,file.leaves[1].value); /* <local_identifier> LOCAL_ID_NN */
   fprintf(stderr, "read LOCAL_ID key = %s res = %d value = %s\n",key, res, file.leaves[1].value);
   res=fscanf(icfp,"%11s %s\n",key,file.leaves[2].value); /* <creation_date_time> CREATION_NN */
   fprintf(stderr, "read CREATION key = %s res = %d value = %s\n",key, res, file.leaves[2].value);
   res=fscanf(icfp,"%12s %s\n",key,file.leaves[3].value); /* <file_size unit="byte"> FILE_SIZE_NN */
   fprintf(stderr, "read FILE_SIZE key = %s res = %d value = %s\n",key, res, file.leaves[3].value);
   res=fscanf(icfp,"%12s %s\n",key,file.leaves[4].value); /* <md5_checksum> MD5_CHECK_NN */
   fprintf(stderr, "read MD5_CHECK key = %s res = %d value = %s\n",key, res, file.leaves[4].value);
   res=fscanf(icfp,"%10s %s\n",key,file.leaves[5].value); /* <comment> COMMENT_NN */
   for(j=0;j<strlen(file.leaves[5].value);j++)
 		if(file.leaves[5].value[j]=='>') file.leaves[5].value[j]=' ';
   fprintf(stderr, "read COMMENT key = %s res = %d value = %s\n",key, res, file.leaves[5].value);
   }


/** write output configuration file */
strcpy(key, "MISSION_NAME"); strcpy(val, mypam.mission);
if (key) {fprintf(ocfp, "%s %s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SPACECRAFT_NAME"); strcpy(val, mypam.spacecraft);
if (key) {fprintf(ocfp, "%s %s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "TARGET_NAME"); strcpy(val, mypam.target);
if (key) {fprintf(ocfp, "%s %s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "INSTRUMENT_ID"); strcpy(val, mypam.instrid);
if (key) {fprintf(ocfp, "%s %s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PRODUCT_TYPE"); strcpy(val, mypam.prodtype);
if (key) {fprintf(ocfp, "%s %s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "BITPIX"); strcpy(val, "    8");
if (key) {fprintf(ocfp, "%s %s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SIGN"); strcpy(val, ssign);
if (key) {fprintf(ocfp, "%s %s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "ENDIAN"); strcpy(val, send);
if (key) {fprintf(ocfp, "%s %s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "WIDTH"); strcpy(val, "  500");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "HEIGHT"); strcpy(val, " 320");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "NPI"); strcpy(val, " 6");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PI_00"); strcpy(val, " href=\"https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1L00.sch\">schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PI_01"); strcpy(val, " href=\"https://psa.esa.int/psa/v1/PDS4_PSA_1L00_1401.sch\">schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PI_02"); strcpy(val, " href=\"https://pds.nasa.gov/pds4/geom/v1/PDS4_GEOM_1L00_1970.sch\">schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PI_03"); strcpy(val, " href=\"https://pds.nasa.gov/pds4/disp/v1/PDS4_DISP_1L00_1510.sch\">schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PI_04"); strcpy(val, " href=\"https://psa.esa.int/psa/em16/tgo/cas/v1/PDS4_EM16_TGO_CAS_1L00_1200.sch\">schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PI_05"); strcpy(val, " href=\"https://psa.esa.int/psa/mission/em16/v1/PDS4_EM16_1L00_1300.sch\">schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "NPOATTR"); strcpy(val, " 7");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_00_NAME"); strcpy(val, " xsi:schemaLocation");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_00_VALUE"); strcpy(val, " http://pds.nasa.gov/pds4/pds/v1>https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1L00.xsd>http://psa.esa.int/psa/v1>https://psa.esa.int/psa/v1/PDS4_PSA_1L00_1401.xsd>http://pds.nasa.gov/pds4/geom/v1>https://pds.nasa.gov/pds4/geom/v1/PDS4_GEOM_1L00_1970.xsd>http://pds.nasa.gov/pds4/disp/v1>https://pds.nasa.gov/pds4/disp/v1/PDS4_DISP_1L00_1510.xsd>http://psa.esa.int/psa/em16/tgo/cas/v1>https://psa.esa.int/psa/em16/tgo/cas/v1/PDS4_EM16_TGO_CAS_1L00_1200.xsd>http://psa.esa.int/psa/mission/em16/v1>https://psa.esa.int/psa/mission/em16/v1/PDS4_EM16_1L00_1300.xsd");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_01_NAME"); strcpy(val, " xmlns");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_01_VALUE"); strcpy(val, " http://pds.nasa.gov/pds4/pds/v1");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_02_NAME"); strcpy(val, " xmlns:em16_tgo_cas");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_02_VALUE"); strcpy(val, " http://psa.esa.int/psa/em16/tgo/cas/v1");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_03_NAME"); strcpy(val, " xmlns:geom");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_03_VALUE"); strcpy(val, " http://pds.nasa.gov/pds4/geom/v1");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_04_NAME"); strcpy(val, " xmlns:disp");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_04_VALUE"); strcpy(val, " http://pds.nasa.gov/pds4/disp/v1");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_05_NAME"); strcpy(val, " xmlns:psa");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_05_VALUE"); strcpy(val, " http://psa.esa.int/psa/v1");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_06_NAME"); strcpy(val, " xmlns:xsi");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PO_ATTR_06_VALUE"); strcpy(val, " http://www.w3.org/2001/XMLSchema-instance");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PROD_LID"); strcpy(val, " urn:esa:psa:em16_tgo_cas:data_derived:cas_der_sc_20250416t233856-20250416t233900-33001-50-pan-1409764716-29-0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PROD_VID"); strcpy(val, " 4.0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "TITLE"); strcpy(val, " CaSSIS>experiment");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "IM_VERS"); strcpy(val, " 1.21.0.0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PROD_CLASS"); strcpy(val, " Product_Observational");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "NCI"); strcpy(val, " 0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "MOD_DATE"); strcpy(val, " 2025-04-17");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "VERSID"); strcpy(val, " 4.0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "DESCR"); strcpy(val, "PDS4>product>label>generated>by>SETM>libraries");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "TSTART"); strcpy(val, " 2025-08-03T00:08:00Z");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "TSTOP"); strcpy(val, " 2025-08-03T23:59:59Z");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PURP"); strcpy(val, " Science");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PROC_LEV"); strcpy(val, " Derived");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "RES_DESCR"); strcpy(val, " Summary>Of>Results");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "LAMBDA"); strcpy(val, " Visible");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "DOMAIN"); strcpy(val, " Surface");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "DISCIPL"); strcpy(val, " Imaging");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "INVEST_AREA"); strcpy(val, " ExoMars>2016");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "INVEST_TYPE"); strcpy(val, " Mission");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "INV_LID"); strcpy(val, " urn:esa:psa:context:investigation:mission.em16");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "REFTYPE"); strcpy(val, " data_to_investigation");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "OSNAME"); strcpy(val, " Observing>System>for>TGO+CASSIS");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "NOSC_COMP"); strcpy(val, " 2");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPNAME_00"); strcpy(val, " TRACE>GAS>ORBITER");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPTYPE_00"); strcpy(val, " Host");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPDESCR_00"); strcpy(val, " Mars>Orbiter");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPLID_00"); strcpy(val, " urn:esa:psa:context:instrument_host:spacecraft.tgo");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPREFTYPE_00"); strcpy(val, " is_instrument_host");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPNAME_01"); strcpy(val, " CaSSIS");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPTYPE_01"); strcpy(val, " Instrument");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPDESCR_01"); strcpy(val, " Mars>Camera");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPLID_01"); strcpy(val, " urn:esa:psa:context:instrument:tgo.cassis");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMPREFTYPE_01"); strcpy(val, " is_instrument");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "TARGET_NAME"); strcpy(val, " mars");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "TARGET_TYPE"); strcpy(val, " Planet");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "TARGET_LID"); strcpy(val, " urn:nasa:pds:context:target:planet.mars");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "TARGET_REFTYPE"); strcpy(val, " data_to_target");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "MISSION"); strcpy(val, " EM:EXOMARS");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PHASE_NAME"); strcpy(val, " Science>Phase");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PHASE_ID"); strcpy(val, " psp");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "CLOCK_START"); strcpy(val, " 1/0755740806:00000");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "CLOCK_STOP"); strcpy(val, " 1/0755827197:00000");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "START_ORB"); strcpy(val, " 33001");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "STOP_ORB"); strcpy(val, " 33002");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SU_ID"); strcpy(val, " SCI");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SU_NAME"); strcpy(val, " CASSIS>Science");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SU_TYPE"); strcpy(val, " Imager");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "OBSCON_IPM"); strcpy(val, " Surface");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "OBSCON_IPD"); strcpy(val, " TARGETED");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "OBSCON_OID"); strcpy(val, " 1409764716");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "OBSCON_OTYPE"); strcpy(val, " INDIVIDUAL");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SWNAME"); strcpy(val, " EM16>Data>Processing>System");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SWVERS"); strcpy(val, " 5.6.0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "INSTR_IFOV"); strcpy(val, " 1.142E-5");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "IFOV_UNIT"); strcpy(val, " rad/px");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "FILTERS"); strcpy(val, " BLU>RED>NIR>PAN");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "HK_FILTER"); strcpy(val, " PAN");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "FOCAL_LENGTH"); strcpy(val, " 0.876");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "FOCAL_UNIT"); strcpy(val, " M");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "F_NUMBER"); strcpy(val, " 6.49");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "TELESCOPE"); strcpy(val, " Three-mirror>anastigmat>with>powered>fold>mirror");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "CAS_DESCR"); strcpy(val, " 2D>Array");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PIX_HEIGHT"); strcpy(val, " 10.0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PIX_HE_UNIT"); strcpy(val, " MICRON");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PIX_WIDTH"); strcpy(val, " 10.0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "PIX_WI_UNIT"); strcpy(val, " MICRON");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "DET_DESCR"); strcpy(val, " SI>CMOS>HYBRID>(OSPREY>2K)");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "NOISE"); strcpy(val, " 61.0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "NOISE_UNIT"); strcpy(val, " ELECTRON");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "AV_INT"); strcpy(val, " 1");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "CAL_TYPE"); strcpy(val, " ground");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "MEAS_RANGE_IDX"); strcpy(val, " 4");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "MEAS_RANGE"); strcpy(val, " 128");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "REF_FRAME"); strcpy(val, " scf");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SPICE_FRAME"); strcpy(val, " TGO_SPACECRAFT");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "STATUS_NAME"); strcpy(val, " urn:esa:psa:em16_spice:spice_kernels:mk_em16");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SC_STATUS"); strcpy(val, " orbit");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "NRLIR"); strcpy(val, " 3");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "RLIR_LID_00"); strcpy(val, " urn:esa:psa:em16_tgo_cas:document:cassis_eaicd_v2");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "RLIR_REFTYPE_00"); strcpy(val, " data_to_document");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "RLIR_LID_01"); strcpy(val, " urn:esa:psa:em16_spice:spice_kernels:mk_em16");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "RLIR_REFTYPE_01"); strcpy(val, " data_to_spice_kernel");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "RLIR_LID_02"); strcpy(val, " urn:esa:psa:em16_tgo_cas:browse_raw:cas_raw_sc_browse_20250416t233856-20250416t233900-33001-50-pan-1409764716-29-0::4.0");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "RLIR_REFTYPE"); strcpy(val, "_02 data_to_browse");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SPE_PRODID"); strcpy(val, " SCI__DCAS__0180E932_2025-106T23-35-39__00003.EXM");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SPE_REFTYPE"); strcpy(val, " data_to_telemetry_source_product");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "SPE_CURFAC"); strcpy(val, " ExoMars>16>Science>Operations>Center");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "LOCAL_ID_00"); strcpy(val, " cas_raw_sc_20250416T233856-20250416T233900-33001-50-PAN-1409764716-29-0__4_0.dat");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "CREATION_00"); strcpy(val, " 2025-04-17T07:02:54.893Z");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "FILE_SIZE_00"); strcpy(val, " 160000");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "MD5_CHECK_00"); strcpy(val, " 98888d232f30c21515c056cb619cd053");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}
strcpy(key, "COMMENT_00"); strcpy(val, " Image>Data>File");
if (key) {fprintf(ocfp, "%s%s\n", key, (val && strlen(val) > 0) ? (char *)val : "n/a");}



/* close input configuration file */
  fclose(icfp);
/* close output configuration file */
  fclose(ocfp);

  	/* deallocate memory */
	free(mypds.po->ia);
	free(mypds.po->oa);
	free(mypds.po->rl);
	free(mypds.po->fao);
	free(mypds.po);

	return 0;
}



int read_pam_metadata(xmlNode * a_node) {
    xmlNode *cur_node = NULL;
	xmlChar *key;
	xmlChar *val;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            // Cerchiamo il tag (elemento) <MDI> / 
            /** search <MDI> tag (element) */
            if (xmlStrEqual(cur_node->name, (const xmlChar *)"MDI")) {
                // Leggiamo l'attributo xml "key" / 
                /** read the key xml attribute */
                key = xmlGetProp(cur_node, (const xmlChar *)"key");
                // Leggiamo il contenuto testuale / 
                /** read the text content */
                val = xmlNodeGetContent(cur_node);

                if (key) {
                    printf("%s %s\n", key, (val && xmlStrlen(val) > 0) ? (char *)val : "n/a");
                }

                xmlFree(key);
                xmlFree(val);
            }
        }
        // Ricorsione sui figli del nodo corrente / 
            /** recursion on siblings of the current node */
        read_pam_metadata(cur_node->children);
    }
}
/** takes in input the PamDataset xml file name
 returns 0 if no errors */
int process_pam(char pamfile[MAXFNAML]) {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;


    // Inizializza la libreria e leggi il file
    /**  Initialize the libxml2 library and read the input file */
    LIBXML_TEST_VERSION
    doc = xmlReadFile(pamfile, NULL, 0);

    if (doc == NULL) {
        fprintf(stderr, "Error: impossible to analyze the file %s\n", pamfile);
        return 1;
    }

    // Ottieni l'elemento radice <PAMDataset>
    /** Obtain the root element <PAMDataset> */
    root_element = xmlDocGetRootElement(doc);

    if (root_element != NULL) {
        fprintf(stderr,"Extracting Metadata from: %s\n", pamfile);
        fprintf(stderr,"----------------------------------------\n");
        read_pam_metadata(root_element);
    }

    // Clean / Pulizia
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}
