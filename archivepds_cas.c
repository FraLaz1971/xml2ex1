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
    char swidth[32];
    char sheight[32];
    unsigned int width = 0;  /* Sample == n. of columns OO */
    unsigned int height = 0; /* Line == n. of rows */
    const char* confname;
    char buf[1024];
    FILE *cfp;
    int i, res, numproducts;
	struct PDS pds; struct PRODUCT_OBSERVATIONAL po; struct IDENTIFICATION_AREA ia;
	struct OBSERVATION_AREA oa; struct FILE_AREA_OBSERVATIONAL fao;
	struct ELEMENT logical_identifier;
	struct ELEMENT version_id;
	struct ELEMENT title;
	struct ELEMENT information_model_version;
	struct ELEMENT product_class;
	struct ELEMENT modification_history;
	struct ELEMENT primary_result_summary;
	struct ELEMENT mission_area;
	struct ELEMENT observing_system;
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
  /* printout variables content for debug */
  fprintf(stderr,"BITPIX = %d\n",bitpix);
  fprintf(stderr,"SIGN = %d\n",sign);
  fprintf(stderr,"ENDIAN = %d\n",end);
  fprintf(stderr,"WIDTH = %u\n",width);/* Sample == n. of columns */
  fprintf(stderr,"HEIGHT = %u\n",height); /* Line == n. of rows */
  fprintf(stderr,"PDS4 <data_type> = %s\n",dtype);
  fclose(cfp);

	numproducts = atoi(argv[2]); /* number of products pointed by the label */
	/* setup data structures and links */
	po.ia=&ia; po.oa=&oa; po.fao=&fao; pds.po=&po;
	prodfnam=(char **)malloc(numproducts*sizeof(char *));
	for(i=0;i<numproducts;i++)
		prodfnam[i]=(char *)malloc(MAXFNAML);
	po.attributes=(struct ATTRIBUTE *)malloc(MAXLEAV*sizeof(struct ATTRIBUTE));
	pds.products=(FILE**)malloc(numproducts*sizeof(FILE *)); 
	oa.target=(struct TARGET_IDENTIFICATION*)malloc(sizeof(struct TARGET_IDENTIFICATION));
	/*observing_system.osc=(struct OBSERVING_SYSTEM_COMPONENT*)malloc(2*sizeof(struct OBSERVING_SYSTEM_COMPONENT));*/
	array2d.leaves[0].attributes=(struct ATTRIBUTE *)malloc(sizeof(struct ATTRIBUTE));	
	modification_history.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	modification_history.leaves[0].leaves=(struct ELEMENT *)malloc(3*sizeof(struct ELEMENT));
	primary_result_summary.leaves=(struct ELEMENT *)malloc(4*sizeof(struct ELEMENT));
	primary_result_summary.leaves[3].leaves=(struct ELEMENT *)malloc(3*sizeof(struct ELEMENT));
	mission_area.leaves=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	mission_area.leaves->leaves=(struct ELEMENT *)malloc(6*sizeof(struct ELEMENT));
	for(i=0;i<2;i++){
		observing_system.osc[i].name=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
		observing_system.osc[i].type=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	}
	file.leaves=(struct ELEMENT*)malloc(MAXFNAML*sizeof(struct ELEMENT));
	for(i=0;i<6;i++)
		array2d.leaves[i].leaves=(struct ELEMENT*)malloc(MAXFNAML*sizeof(struct ELEMENT));
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
	if(verbose)fprintf(stderr,"main() starting copying strings\n");
	strcpy((char*)pds.xmlintest,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>");
	strcpy((char*)pds.xml_model[0],"href=\"https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1L00.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[1],"href=\"https://psa.esa.int/psa/v1/PDS4_PSA_1L00_1401.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[2],"href=\"https://pds.nasa.gov/pds4/geom/v1/PDS4_GEOM_1L00_1970.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[3],"href=\"https://pds.nasa.gov/pds4/disp/v1/PDS4_DISP_1L00_1510.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[4],"href=\"https://psa.esa.int/psa/em16/tgo/cas/v1/PDS4_EM16_TGO_CAS_1L00_1200.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)pds.xml_model[5],"href=\"https://psa.esa.int/psa/mission/em16/v1/PDS4_EM16_1L00_1300.sch\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)po.name,"Product_Observational");
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
	strcpy((char*)ia.name,"Identification_Area");
	strcpy((char*)oa.name,"Observation_Area");
	strcpy((char*)fao.name,"File_Area_Observational");
	strcpy((char*)logical_identifier.name,"logical_identifier");
	strcpy((char*)logical_identifier.value,"urn:esa:psa:em16_tgo_cas:data_raw:cas_raw_sc_20250416t233856-20250416t233900-33001-50-pan-1409764716-29-0");
	strcpy((char*)version_id.name,"version_id");
	strcpy((char*)version_id.value,"1.0");
	strcpy((char*)title.name,"title");
	strcpy((char*)title.value,"Test to develop BC SIMBIO-SYS STC TDM PDS4 labels");
	strcpy((char*)information_model_version.name,"information_model_version");
	strcpy((char*)information_model_version.value,"1.21.0.0");
	strcpy((char*)product_class.name,"product_class");
	strcpy((char*)product_class.value,"Product_Observational");
	if(verbose)fprintf(stderr,"main() before Modification_History\n");
	strcpy((char*)modification_history.name,"Modification_History");
	strcpy((char*)modification_history.leaves[0].name,"Modification_Detail");
	strcpy((char*)modification_history.leaves[0].leaves[0].name,"modification_date");
	strcpy((char*)modification_history.leaves[0].leaves[0].value,"2025-10-20");
	strcpy((char*)modification_history.leaves[0].leaves[1].name,"version_id");
	strcpy((char*)modification_history.leaves[0].leaves[1].value,"1.0");
	strcpy((char*)modification_history.leaves[0].leaves[2].name,"description");
	strcpy((char*)modification_history.leaves[0].leaves[2].value,"PDS4 product label generated by SETM libraries");
    if(verbose)fprintf(stderr,"main() after modification_history\n");
	strcpy((char*)timecoord.name,"Time_Coordinates");
	strcpy((char*)timecoord.tstart.name,"start_date_time");
	strcpy((char*)timecoord.tstart.value,tstart);
	strcpy((char*)timecoord.tstop.name,"stop_date_time");
	strcpy((char*)timecoord.tstop.value,tstop);
	strcpy((char*)primary_result_summary.name,"Primary_Result_Summary");
	strcpy((char*)primary_result_summary.leaves[0].name,"purpose");
	strcpy((char*)primary_result_summary.leaves[0].value,"Science");
	strcpy((char*)primary_result_summary.leaves[1].name,"processing_level");
	strcpy((char*)primary_result_summary.leaves[1].value,"Raw");
	strcpy((char*)primary_result_summary.leaves[2].name,"description");
	strcpy((char*)primary_result_summary.leaves[2].value,"Summary Of Results");
	strcpy((char*)primary_result_summary.leaves[3].name,"Science_Facets");
	strcpy((char*)primary_result_summary.leaves[3].leaves[0].name,"wavelength_range");
	strcpy((char*)primary_result_summary.leaves[3].leaves[0].value,"Visible");
	strcpy((char*)primary_result_summary.leaves[3].leaves[1].name,"domain");
	strcpy((char*)primary_result_summary.leaves[3].leaves[1].value,"Surface");
	strcpy((char*)primary_result_summary.leaves[3].leaves[2].name,"discipline_name");
	strcpy((char*)primary_result_summary.leaves[3].leaves[2].value,"Imaging");
    if(verbose)fprintf(stderr,"main() before Observing System\n");	
	strcpy((char*)oa.leaves[0].name,"Observing System");
	strcpy((char*)observing_system.name,"Observing_System");
	strcpy((char*)observing_system.osc[0].ename,"Observing_System_Component");
	strcpy((char*)observing_system.osc[0].name->name,"name");
	strcpy((char*)observing_system.osc[0].name->value,"messenger");
	strcpy((char*)observing_system.osc[0].type->name,"type");
	strcpy((char*)observing_system.osc[0].type->value,"Host");
	strcpy((char*)observing_system.osc[1].ename,"Observing_System_Component");
	strcpy((char*)observing_system.osc[1].name->name,"name");
	strcpy((char*)observing_system.osc[1].name->value,"mercury dual imaging system narrow angle camera");
	strcpy((char*)observing_system.osc[1].type->name,"type");
	strcpy((char*)observing_system.osc[1].type->value,"Instrument");
    if(verbose)fprintf(stderr,"main() before Target_Identification\n");	
	strcpy((char*)oa.target[0].ename,"Target_Identification");
	strcpy((char*)oa.target[0].name.name,"name");
	strcpy((char*)oa.target[0].name.value,"mercury");
	strcpy((char*)oa.target[0].type.name,"type");
	strcpy((char*)oa.target[0].type.value,"Planet");
	strcpy((char*)mission_area.name,"Mission_Area");
	strcpy((char*)mission_area.leaves[0]->name,"mess:MESSENGER");
    if(verbose)fprintf(stderr,"main() before Investigation Area\n");	
	strcpy((char*)iaa.ename,"Investigation_Area");
	strcpy((char*)iaa.name.name,"name");
	strcpy((char*)iaa.name.value,"messenger");
	strcpy((char*)iaa.type.name,"type");
	strcpy((char*)iaa.type.value,"Mission");
    if(verbose)fprintf(stderr,"main() before Internal_Reference\n");	
	strcpy((char*)iref.name,"Internal_Reference");
	strcpy((char*)iref.lid_reference.name,"lid_reference");
	strcpy((char*)iref.lid_reference.value,"urn:nasa:pds:investigation.messenger");
	strcpy((char*)iref.reference_type.name,"reference_type");
	strcpy((char*)iref.reference_type.value,"data_to_investigation");
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
	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[0].ename, BAD_CAST NULL); /*<Observing_System_Component>*/
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[0].name->name, BAD_CAST observing_system.osc[0].name->value);
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[0].type->name, BAD_CAST observing_system.osc[0].type->value);
	p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[1].ename, BAD_CAST NULL);/*<Observing_System_Component>*/
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[1].name->name, BAD_CAST observing_system.osc[1].name->value);
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[1].type->name, BAD_CAST observing_system.osc[1].type->value);
	p1=p1->parent;p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <Target_Identification> 3\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST oa.target[0].ename, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].name.name, BAD_CAST oa.target[0].name.value);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].type.name, BAD_CAST oa.target[0].type.value);
	p1=p1->parent;p1=p1->parent;
    if(verbose)fprintf(stderr,"main() opening <File_Area_Observational> node element step 3.5\n");
	p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->fao->name, BAD_CAST NULL); /* File_Area_Observational node element */
	p1 = xmlNewChild(p1, NULL, BAD_CAST file.name, BAD_CAST NULL); /* opening <File> element */
	xmlNewChild(p1, NULL, BAD_CAST file.leaves[0].name, BAD_CAST file.leaves[0].value); /* <file_name> element */
	p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.name, BAD_CAST NULL); /* opening <Array_2D_Image> node */
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[0].name, BAD_CAST BAD_CAST array2d.leaves[0].value); /* offset */
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
	free(modification_history.leaves[0].leaves);
	free(modification_history.leaves);
	free(prodfnam);
	for(i=0;i<2;i++){
		free(observing_system.osc[i].name);
		free(observing_system.osc[i].type);
	}
	free(mission_area.leaves->leaves);
	free(mission_area.leaves);
	free(file.leaves);
	for(i=0;i<6;i++)
		free(array2d.leaves[i].leaves);
	// prodfname freed 
	free(oa.target);
	free(po.attributes); // po.attributes freed 
	fprintf(stderr,"main() ending program\n");
  return 0;
}
