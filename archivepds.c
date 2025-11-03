#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/tree.h>
#include "pds.h"

int main(int argc, char **argv)
{	
	int status=0;
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, pi=NULL;/* node pointers */
	xmlNodePtr p1 = NULL; /* node pointers */
    xmlDtdPtr dtd = NULL;       /* DTD pointer */
    xmlAttr *attribute = NULL;
    xmlAttrPtr attr = NULL;
    xmlNs *namespace_decl = NULL;
    xmlTextWriterPtr writer;
    xmlChar *attr_name;
    xmlChar * attr_value;
    char buff[256];
    int i, j, res;
	struct PDS pds; struct PRODUCT_OBSERVATIONAL po; struct IDENTIFICATION_AREA ia;
	struct OBSERVATION_AREA oa; struct FILE_AREA_OBSERVATIONAL fao;
	struct ELEMENT logical_identifier;
	struct ELEMENT version_id;
	struct ELEMENT title;
	struct ELEMENT information_model_version;
	struct ELEMENT product_class;
	struct ELEMENT modification_history;
	struct ELEMENT observing_system;
	struct TIME_COORDINATES timecoord;
	FILE *lp; /* pds4 label file handler */
	char **prodfnam;
	if(argc<3){
		fprintf(stderr,"usage:%s <nproducts> <prodname1> <prodname2> ... <prodnamen>\n",argv[0]);
		fprintf(stderr,"e.g: %s 1 image.raw \n",argv[0]);
		return 1;
	}
	pds.numproducts = atoi(argv[1]); /* number of products pointed by the label */
	/* setup data structures and links */
	timecoord.tstart=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	timecoord.tstop=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	timecoord.tstart->name=(char*)malloc(MAXFNAML);
	timecoord.tstart->value=(char*)malloc(32);
	timecoord.tstop->name=(char*)malloc(MAXFNAML);
	timecoord.tstop->value=(char*)malloc(32);
	prodfnam=(char **)malloc(pds.numproducts*sizeof(char *));
	pds.products=(FILE**)malloc(pds.numproducts*sizeof(FILE *));
	for(i=0;i<pds.numproducts;i++){
		prodfnam[i]=(char *)malloc(MAXFNAML);
		pds.products[i] = (FILE*)malloc(sizeof(FILE));
		if(argv[i+2]!=NULL){
			strcpy(prodfnam[i],argv[i+2]);
		} else {
			perror("ERROR: filename lacking, exiting");
			exit(1);
		}
	}
	po.value=NULL;
	pds.pfnames=prodfnam;
	pds.xmlintest=(char *)malloc(MAXFNAML);strcpy(pds.xmlintest,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>");
	pds.xml_model=(char *)malloc(MAXFNAML);strcpy(pds.xml_model,"href=\"https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1K00.sch\" \n schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	po.name=(char *)malloc(MAXFNAML); strcpy(po.name,"Product_Observational");
	po.value=NULL;
	ia.name=(char *)malloc(MAXFNAML); strcpy(ia.name,"Identification_Area");
	ia.value=NULL;
	oa.name=(char *)malloc(MAXFNAML); strcpy(oa.name,"Observation_Area");
	oa.value=NULL;
	fao.name=(char *)malloc(MAXFNAML); strcpy(fao.name,"File_Area_Observational");
	fao.value=NULL;
	po.ia=&ia; po.oa=&oa; po.fao=&fao; pds.po=&po;
	lp=stdout;
	logical_identifier.name=(char *)malloc(MAXFNAML);
	logical_identifier.value=(char *)malloc(MAXFNAML);
	version_id.name=(char *)malloc(MAXFNAML);
	version_id.value=(char *)malloc(MAXFNAML);
	title.name=(char *)malloc(MAXFNAML);
	title.value=(char *)malloc(MAXFNAML);
	information_model_version.name=(char *)malloc(MAXFNAML);
	information_model_version.value=(char *)malloc(MAXFNAML);
	product_class.name=(char *)malloc(MAXFNAML);
	product_class.value=(char *)malloc(MAXFNAML);
	modification_history.name=(char *)malloc(MAXFNAML);
	modification_history.value=NULL;
	strcpy(logical_identifier.name,"logical_identifier");
	strcpy(logical_identifier.value,"urn:nasa:pds:data:mess-h-mdis-5-dem-elevation-v1.0:msgr_dem_usg_sc_j_v02");
	strcpy(version_id.name,"version_id");
	strcpy(version_id.value,"1.0");
	strcpy(title.name,"title");
	strcpy(title.value,"mess-h-mdis-5-dem-elevation-v1.0 msgr_dem_usg_sc_j_v02");
	strcpy(information_model_version.name,"information_model_version");
	strcpy(information_model_version.value,"1.20.0.0");
	strcpy(product_class.name,"product_class");
	strcpy(product_class.value,"Product_Observational");
	strcpy(modification_history.name,"Modification_History");
	po.ia->leaves=(struct ELEMENT **)malloc(6*sizeof(struct ELEMENT*));
	for(i=0;i<6;i++)
		po.ia->leaves[i]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	po.ia->leaves[0] = &logical_identifier;
	po.ia->leaves[1] = &version_id;
	po.ia->leaves[2] = &title;
	po.ia->leaves[3] = &information_model_version;
	po.ia->leaves[4] = &product_class;
	po.ia->leaves[5] = &modification_history;
	strcpy(timecoord.tstart->name,"start_date_time");
	strcpy(timecoord.tstart->value,"2011-03-18T00:00:00Z");
	strcpy(timecoord.tstop->name,"stop_date_time");
	strcpy(timecoord.tstop->value,"2014-11-01T00:00:00Z");
	oa.times=&timecoord;
	oa.leaves=(struct ELEMENT **)malloc(1*sizeof(struct ELEMENT*));
	observing_system.name=(char*)malloc(MAXFNAML);strcpy(observing_system.name,"Observing_System");
	observing_system.value=NULL;
	for(i=0;i<1;i++)
		oa.leaves[i]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	oa.leaves[0]=&observing_system;
	observing_system.osc=(struct OBSERVING_SYSTEM_COMPONENT**)malloc(2*sizeof(struct OBSERVING_SYSTEM_COMPONENT*));
	for(i=0;i<2;i++){
		observing_system.osc[i]=(struct OBSERVING_SYSTEM_COMPONENT*)malloc(sizeof(struct OBSERVING_SYSTEM_COMPONENT));
		observing_system.osc[i]->ename=(char *)malloc(MAXFNAML);
		observing_system.osc[i]->value=(char *)malloc(MAXFNAML);
		observing_system.osc[i]->name=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
		observing_system.osc[i]->type=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
		observing_system.osc[i]->name->name=(char *)malloc(MAXFNAML);
		observing_system.osc[i]->name->value=(char *)malloc(MAXFNAML);
		observing_system.osc[i]->type->name=(char *)malloc(MAXFNAML);
		observing_system.osc[i]->type->value=(char *)malloc(MAXFNAML);
	}
	strcpy(observing_system.osc[0]->name->name,"name");
	strcpy(observing_system.osc[0]->name->value,"messenger");
	strcpy(observing_system.osc[0]->type->name,"type");
	strcpy(observing_system.osc[0]->type->value,"Spacecraft");
	strcpy(observing_system.osc[1]->name->name,"name");
	strcpy(observing_system.osc[1]->name->value,"mercury dual imaging system narrow angle camera");
	strcpy(observing_system.osc[1]->type->name,"type");
	strcpy(observing_system.osc[1]->type->value,"Instrument");
	oa.target=(struct TARGET_IDENTIFICATION*)malloc(sizeof(struct TARGET_IDENTIFICATION));
/*	oa.target[0].name->name=(char *)malloc(MAXFNAML);
	oa.target[0].name->value=(char *)malloc(MAXFNAML);
	oa.target[0].type->name=(char *)malloc(MAXFNAML);
	oa.target[0].type->value=(char *)malloc(MAXFNAML);
	strcpy(oa.target[0].name->name,"name");
	strcpy(oa.target[0].name->value,"mercury");
	strcpy(oa.target[0].type->name,"type");
	strcpy(oa.target[0].type->value,"Planet");*/
	if (init_pds(lp,&pds, prodfnam)) perror("error in initing pds product archiving"); 
    LIBXML_TEST_VERSION;

	doc = xmlNewDoc(BAD_CAST "1.0");

    root_node = xmlNewNode(NULL, BAD_CAST po.name);    
    xmlDocSetRootElement(doc, root_node);
    attribute = root_node->properties;
    	 // Create the xml-model processing instruction
    pi = xmlNewPI(BAD_CAST "xml-model", BAD_CAST pds.xml_model);
    
    // Add it before the root element
    xmlAddPrevSibling(root_node, pi);

	printf("main() Before xmlNewChild(root_node, ...)");
    xmlNewChild(root_node, NULL, BAD_CAST pds.po->ia->name, BAD_CAST NULL); /* Identification_Area node element */
    p1=root_node->children;
    xmlNewChild(p1, NULL, BAD_CAST logical_identifier.name, BAD_CAST logical_identifier.value);
    xmlNewChild(p1, NULL, BAD_CAST version_id.name, BAD_CAST version_id.value);
    xmlNewChild(p1, NULL, BAD_CAST title.name, BAD_CAST title.value);
    xmlNewChild(p1, NULL, BAD_CAST information_model_version.name, BAD_CAST information_model_version.value);
    xmlNewChild(p1, NULL, BAD_CAST product_class.name, BAD_CAST product_class.value);
    p1=xmlNewChild(p1, NULL, BAD_CAST modification_history.name, BAD_CAST NULL);
    //p1=xmlNewChild(p1, NULL, BAD_CAST "Modification_Detail", BAD_CAST NULL);
    //xmlNewChild(p1, NULL, BAD_CAST "Modification_Date", BAD_CAST "2025-10-24");
    //xmlNewChild(p1, NULL, BAD_CAST "version_id", BAD_CAST "1.0");
    //xmlNewChild(p1, NULL, BAD_CAST "description", BAD_CAST "PDS4 product label generated by SETM libraries");
    //p1=p1->parent;p1=p1->parent;p1=p1->parent;
    p1=root_node; /* temporary, to change*/
    p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->oa->name, BAD_CAST NULL); /* Observation_Area node element */
	p1 = xmlNewChild(p1, NULL, BAD_CAST "Time_Coordinates", BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST "start_date_time", BAD_CAST "2025-10-24T09:00:00Z");
	xmlNewChild(p1, NULL, BAD_CAST "stop_date_time", BAD_CAST "2025-10-24T12:00:00Z");
	p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST "Investigation_Area", BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST "name", BAD_CAST "messenger");
	xmlNewChild(p1, NULL, BAD_CAST "type", BAD_CAST "Mission");
	p1=xmlNewChild(p1, NULL, BAD_CAST "Internal_Reference", BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST "lid_reference", BAD_CAST "urn:nasa:pds:investigation.messenger");
	xmlNewChild(p1, NULL, BAD_CAST "reference_type", BAD_CAST "data_to_investigation");
	p1=p1->parent;p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST "Observing_System", BAD_CAST NULL);
	p1 = xmlNewChild(p1, NULL, BAD_CAST "Observing_System_Component", BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST "name", BAD_CAST "mercury dual imaging system narrow angle camera");
	xmlNewChild(p1, NULL, BAD_CAST "type", BAD_CAST "Instrument");
	p1=p1->parent;p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST "Target_Identification", BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST "name", BAD_CAST "mercury");
	xmlNewChild(p1, NULL, BAD_CAST "planet", BAD_CAST "Planet");
	p1=p1->parent;p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->fao->name, BAD_CAST NULL); /* File_Area_Observational node element */
	//p1 = xmlNewChild(p1, NULL, BAD_CAST "File", BAD_CAST NULL);
	//xmlNewChild(p1, NULL, BAD_CAST "file_name", BAD_CAST "MSGR_DEM_USG_SC_J_V02.IMG");
	//p1=p1->parent;
	//p1 = xmlNewChild(p1, NULL, BAD_CAST "Array_2D_Image", BAD_CAST NULL);
	//p1 = xmlNewChild(p1, NULL, BAD_CAST "offset", BAD_CAST "0");
	//attr = xmlSetProp(p1, "unit","byte");
	//p1=p1->parent;
	//xmlNewChild(p1, NULL, BAD_CAST "axes", BAD_CAST "2");
	//xmlNewChild(p1, NULL, BAD_CAST "axis_indes_order", BAD_CAST "Last Index Fastest");
	//p1 = xmlNewChild(p1, NULL, BAD_CAST "Element_Array", BAD_CAST NULL);
	//xmlNewChild(p1, NULL, BAD_CAST "data_type", BAD_CAST "SignedLSB2");
	//p1=p1->parent;
	//p1 = xmlNewChild(p1, NULL, BAD_CAST "Axis_Array", BAD_CAST NULL);
	//xmlNewChild(p1, NULL, BAD_CAST "axis_name", BAD_CAST "Line");
	//xmlNewChild(p1, NULL, BAD_CAST "elements", BAD_CAST "11520");
	//xmlNewChild(p1, NULL, BAD_CAST "sequence_number", BAD_CAST "1");
	//p1=p1->parent;
	//p1 = xmlNewChild(p1, NULL, BAD_CAST "Axis_Array", BAD_CAST NULL);
	//xmlNewChild(p1, NULL, BAD_CAST "axis_name", BAD_CAST "Sample");
	//xmlNewChild(p1, NULL, BAD_CAST "elements", BAD_CAST "23040");
	//xmlNewChild(p1, NULL, BAD_CAST "sequence_number", BAD_CAST "2");
	//p1=p1->parent;p1=p1->parent;p1=p1->parent;p1=p1->parent;

    /*
 *      * Dumping document to stdio or file
 *           
    xmlSaveFormatFileEnc(argc > 1 ? argv[1] : "-", doc, "UTF-8", 1);
*/
	xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
    xmlFreeDoc(doc);

    xmlCleanupParser();
//freall:    
	for(i=0;i<pds.numproducts;i++){
		free(prodfnam[i]);
		free(pds.products[i]);
	}
	free(po.name);
	free(ia.name);
	free(oa.name);
	free(oa.target);
	free(fao.name);
	free(logical_identifier.value);
	free(observing_system.name);
	free(timecoord.tstart);
	free(timecoord.tstop);
	free(logical_identifier.name);
	free(version_id.name);
	free(version_id.value);
	free(title.name);
	free(title.value);
	free(information_model_version.name);
	free(information_model_version.value);
	free(product_class.name);
	free(product_class.value);
	free(modification_history.name);
	free(timecoord.tstart->name);
	free(timecoord.tstart->value);
	free(timecoord.tstop->name);
	free(timecoord.tstop->value);
	free(timecoord.tstart);
	free(timecoord.tstop);
	free(oa.leaves[0]);
	free(oa.leaves);
	for(i=0;i<6;i++)
		free(po.ia->leaves[i]);
	free(po.ia->leaves);
	for(i=0;i<2;i++){
		free(observing_system.osc[i]->ename);
		free(observing_system.osc[i]->value);
		free(observing_system.osc[i]->name);
		free(observing_system.osc[i]->type);
		free(observing_system.osc[i]);
	}
	free(observing_system.osc);
  return status;
}
