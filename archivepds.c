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
    int i, j, res, numproducts;
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
	struct INTERNAL_REFERENCE iref;
	struct INVESTIGATION_AREA iaa;
	struct ARRAY_2D_IMAGE array2d;

	FILE *lp; /* pds4 label file handler */
	char **prodfnam;
	if(argc<3){
		fprintf(stderr,"usage:%s <nproducts> <prodname1> <prodname2> ... <prodnamen>\n",argv[0]);
		fprintf(stderr,"e.g: %s 1 image.raw \n",argv[0]);
		return 1;
	}
	numproducts = atoi(argv[1]); /* number of products pointed by the label */
	/* setup data structures and links */
	po.ia=&ia; po.oa=&oa; po.fao=&fao; pds.po=&po;
	prodfnam=(char **)malloc(numproducts*sizeof(char *));
	for(i=0;i<numproducts;i++)
		prodfnam[i]=(char *)malloc(MAXFNAML);
	po.attributes=(struct ATTRIBUTE *)malloc(sizeof(struct ATTRIBUTE));
	po.ia->leaves=(struct ELEMENT **)malloc(6*sizeof(struct ELEMENT*));
	for(i=0;i<6;i++)
		po.ia->leaves[i]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	oa.leaves=(struct ELEMENT **)malloc(sizeof(struct ELEMENT *));
	for(i=0;i<1;i++)
		oa.leaves[i]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	modification_history.leaves=(struct ELEMENT **)malloc(sizeof(struct ELEMENT*));
	modification_history.leaves[0]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	modification_history.leaves[0]->leaves=(struct ELEMENT **)malloc(sizeof(struct ELEMENT*));
	for(i=0;i<3;i++)
		modification_history.leaves[0]->leaves[i]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	strcpy((char *)oa.leaves[0]->name,"Observing System");
	pds.products=(FILE**)malloc(numproducts*sizeof(FILE *)); 
	oa.target=(struct TARGET_IDENTIFICATION*)malloc(sizeof(struct TARGET_IDENTIFICATION));
	observing_system.osc=(struct OBSERVING_SYSTEM_COMPONENT*)malloc(2*sizeof(struct OBSERVING_SYSTEM_COMPONENT));
	pds.po->fao->leaves=(struct ELEMENT **)malloc(sizeof(struct ELEMENT*));
	pds.po->fao->leaves[0]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	pds.po->fao->leaves[0]->leaves=(struct ELEMENT **)malloc(2*sizeof(struct ELEMENT*));
	pds.po->fao->leaves[0]->leaves[0]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	pds.po->fao->leaves[0]->leaves[1]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	array2d.leaves=(struct ELEMENT **)malloc(6*sizeof(struct ELEMENT*));
	for(i=0;i<6;i++)
		array2d.leaves[i]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	array2d.leaves[0]->attributes=(struct ATTRIBUTE *)malloc(sizeof(struct ATTRIBUTE));	
	array2d.leaves[3]->leaves=(struct ELEMENT **)malloc(4*sizeof(struct ELEMENT*));
	array2d.leaves[3]->leaves[0]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	array2d.leaves[4]->leaves=(struct ELEMENT **)malloc(3*sizeof(struct ELEMENT*));
	for(i=0;i<3;i++)
		array2d.leaves[4]->leaves[i]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	array2d.leaves[5]->leaves=(struct ELEMENT **)malloc(3*sizeof(struct ELEMENT*));
	for(i=0;i<3;i++)
		array2d.leaves[5]->leaves[i]=(struct ELEMENT *)malloc(sizeof(struct ELEMENT));
	fprintf(stderr,"main() numproducts = %d\n",numproducts);
	for(i=0;i<numproducts;i++){
		pds.products[i] = (FILE*)malloc(2048);
		if(argv[i+2]!=NULL){
			strcpy((char*)prodfnam[i],argv[i+2]);
		} else {
			perror("ERROR: filename lacking, exiting");
			exit(1);
		}
	}
	strcpy((char*)pds.xmlintest,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>");
	strcpy((char*)pds.xml_model,"href=\"https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1K00.sch\" \n schematypens=\"http://purl.oclc.org/dsdl/schematron\"");
	strcpy((char*)po.name,"Product_Observational");
	strcpy((char*)ia.name,"Identification_Area");
	strcpy((char*)oa.name,"Observation_Area");
	strcpy((char*)fao.name,"File_Area_Observational");
	strcpy((char*)logical_identifier.name,"logical_identifier");
	strcpy((char*)logical_identifier.value,"urn:nasa:pds:data:mess-h-mdis-5-dem-elevation-v1.0:msgr_dem_usg_sc_j_v02");
	strcpy((char*)version_id.name,"version_id");
	strcpy((char*)version_id.value,"1.0");
	strcpy((char*)title.name,"title");
	strcpy((char*)title.value,"mess-h-mdis-5-dem-elevation-v1.0 msgr_dem_usg_sc_j_v02");
	strcpy((char*)information_model_version.name,"information_model_version");
	strcpy((char*)information_model_version.value,"1.20.0.0");
	strcpy((char*)product_class.name,"product_class");
	strcpy((char*)product_class.value,"Product_Observational");
	strcpy((char*)modification_history.name,"Modification_History");
	strcpy((char*)modification_history.leaves[0]->name,"Modification_Detail");
	strcpy((char*)modification_history.leaves[0]->leaves[0]->name,"modification_date");
	strcpy((char*)modification_history.leaves[0]->leaves[0]->value,"2025-10-20");
	strcpy((char*)modification_history.leaves[0]->leaves[1]->name,"version_id");
	strcpy((char*)modification_history.leaves[0]->leaves[1]->value,"1.0");
	strcpy((char*)modification_history.leaves[0]->leaves[2]->name,"description");
	strcpy((char*)modification_history.leaves[0]->leaves[2]->value,"PDS4 product label generated by SETM libraries");
	po.ia->leaves[0] = &logical_identifier;
	po.ia->leaves[1] = &version_id;
	po.ia->leaves[2] = &title;
	po.ia->leaves[3] = &information_model_version;
	po.ia->leaves[4] = &product_class;
	po.ia->leaves[5] = &modification_history;
	strcpy((char*)timecoord.name,"Time_Coordinates");
	strcpy((char*)timecoord.tstart.name,"start_date_time");
	strcpy((char*)timecoord.tstart.value,"2011-03-18T00:00:00Z");
	strcpy((char*)timecoord.tstop.name,"stop_date_time");
	strcpy((char*)timecoord.tstop.value,"2014-11-01T00:00:00Z");
	strcpy((char*)observing_system.name,"Observing_System");
	strcpy((char*)observing_system.osc[0].ename,"Observing_System_Component");
	strcpy((char*)observing_system.osc[0].name.name,"name");
	strcpy((char*)observing_system.osc[0].name.value,"messenger");
	strcpy((char*)observing_system.osc[0].type.name,"type");
	strcpy((char*)observing_system.osc[0].type.value,"Spacecraft");
	strcpy((char*)observing_system.osc[1].ename,"Observing_System_Component");
	strcpy((char*)observing_system.osc[1].name.name,"name");
	strcpy((char*)observing_system.osc[1].name.value,"mercury dual imaging system narrow angle camera");
	strcpy((char*)observing_system.osc[1].type.name,"type");
	strcpy((char*)observing_system.osc[1].type.value,"Instrument");
	strcpy((char*)oa.target[0].ename,"Target_Identification");
	strcpy((char*)oa.target[0].name.name,"name");
	strcpy((char*)oa.target[0].name.value,"mercury");
	strcpy((char*)oa.target[0].type.name,"type");
	strcpy((char*)oa.target[0].type.value,"Planet");
	strcpy((char*)iaa.ename,"Investigation Area");
	strcpy((char*)iaa.name.name,"name");
	strcpy((char*)iaa.name.value,"messenger");
	strcpy((char*)iaa.type.name,"type");
	strcpy((char*)iaa.type.value,"Spacecraft");
	strcpy((char*)iref.name,"Internal_Reference");
	strcpy((char*)iref.lid_reference.name,"lid_reference");
	strcpy((char*)iref.lid_reference.value,"urn:nasa:pds:investigation.messenger");
	strcpy((char*)iref.reference_type.name,"reference_type");
	strcpy((char*)iref.reference_type.value,"data_to_investigation");
	strcpy((char*)pds.po->fao->leaves[0]->name,"File");
	strcpy((char*)pds.po->fao->leaves[0]->leaves[0]->name,"file_name");
	strcpy((char*)pds.po->fao->leaves[0]->leaves[0]->value,prodfnam[0]);
	strcpy(array2d.name,"Array_2D_Image");
	strcpy(array2d.leaves[0]->name,"offset"); 
	strcpy(array2d.leaves[0]->attributes->name,"unit");
	strcpy(array2d.leaves[0]->attributes->value,"byte");
	strcpy(array2d.leaves[0]->value,"0");
	strcpy(array2d.leaves[1]->name,"axes");
	strcpy(array2d.leaves[1]->value,"2");
	strcpy(array2d.leaves[2]->name,"axis_index_order");
	strcpy(array2d.leaves[2]->value,"Last Index Fastest");
	strcpy(array2d.leaves[3]->name,"Element_Array");
	strcpy(array2d.leaves[3]->leaves[0]->name,"data_type");
	strcpy(array2d.leaves[3]->leaves[0]->value,"SignedLSB2");
	strcpy(array2d.leaves[4]->name,"Axis_Array");
	strcpy(array2d.leaves[4]->leaves[0]->name,"axis_name");
	strcpy(array2d.leaves[4]->leaves[0]->value,"Line");
	strcpy(array2d.leaves[4]->leaves[1]->name,"elements");
	strcpy(array2d.leaves[4]->leaves[1]->value,"11520");
	strcpy(array2d.leaves[4]->leaves[2]->name,"sequence_number");
	strcpy(array2d.leaves[4]->leaves[2]->value,"1");
	strcpy(array2d.leaves[5]->name,"Axis_Array");
	strcpy(array2d.leaves[5]->leaves[0]->name,"axis_name");
	strcpy(array2d.leaves[5]->leaves[0]->value,"Sample");
	strcpy(array2d.leaves[5]->leaves[1]->name,"elements");
	strcpy(array2d.leaves[5]->leaves[1]->value,"23040");
	strcpy(array2d.leaves[5]->leaves[2]->name,"sequence_number");
	strcpy(array2d.leaves[5]->leaves[2]->value,"2");
	lp=stdout; 	pds.pfnames=prodfnam;
	oa.times=&timecoord;
	oa.leaves[0]=&observing_system;
	if (init_pds(lp,&pds, prodfnam,argv)) perror("error in initing pds product archiving"); 
    LIBXML_TEST_VERSION;

	doc = xmlNewDoc(BAD_CAST "1.0");

    root_node = xmlNewNode(NULL, BAD_CAST po.name);    
    xmlDocSetRootElement(doc, root_node);
    attribute = root_node->properties;
    	 // Create the xml-model processing instruction
    pi = xmlNewPI(BAD_CAST "xml-model", BAD_CAST pds.xml_model);
    
    // Add it before the root element
    xmlAddPrevSibling(root_node, pi);

    xmlNewChild(root_node, NULL, BAD_CAST pds.po->ia->name, BAD_CAST NULL); /* Identification_Area node element */
    p1=root_node->children;
    xmlNewChild(p1, NULL, BAD_CAST logical_identifier.name, BAD_CAST logical_identifier.value);
    xmlNewChild(p1, NULL, BAD_CAST version_id.name, BAD_CAST version_id.value);
    xmlNewChild(p1, NULL, BAD_CAST title.name, BAD_CAST title.value);
    xmlNewChild(p1, NULL, BAD_CAST information_model_version.name, BAD_CAST information_model_version.value);
    xmlNewChild(p1, NULL, BAD_CAST product_class.name, BAD_CAST product_class.value);
    p1=xmlNewChild(p1, NULL, BAD_CAST modification_history.name, BAD_CAST NULL);
    p1=xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0]->name, BAD_CAST NULL);
    xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0]->leaves[0]->name, BAD_CAST modification_history.leaves[0]->leaves[0]->value);
    xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0]->leaves[1]->name, BAD_CAST modification_history.leaves[0]->leaves[1]->value);
    xmlNewChild(p1, NULL, BAD_CAST modification_history.leaves[0]->leaves[2]->name, BAD_CAST modification_history.leaves[0]->leaves[2]->value);
    p1=p1->parent;p1=p1->parent;p1=p1->parent;
    p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->oa->name, BAD_CAST NULL); /* Observation_Area node element */
	p1 = xmlNewChild(p1, NULL, BAD_CAST timecoord.name, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST timecoord.tstart.name, BAD_CAST timecoord.tstart.value);
	xmlNewChild(p1, NULL, BAD_CAST timecoord.tstop.name, BAD_CAST timecoord.tstop.value);
	p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST iaa.ename, BAD_CAST NULL); /* Investigation_Area node element */
	xmlNewChild(p1, NULL, BAD_CAST iaa.name.name, BAD_CAST iaa.name.value);
	xmlNewChild(p1, NULL, BAD_CAST iaa.type.name, BAD_CAST iaa.type.value);
	p1=xmlNewChild(p1, NULL, BAD_CAST iref.name, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST iref.lid_reference.name, BAD_CAST iref.lid_reference.value);
	xmlNewChild(p1, NULL, BAD_CAST iref.reference_type.name, BAD_CAST iref.reference_type.value);
	p1=p1->parent;p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.name, BAD_CAST NULL);
	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[0].ename, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[0].name.name, BAD_CAST observing_system.osc[0].name.value);
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[0].type.name, BAD_CAST observing_system.osc[0].type.value);
	p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[1].ename, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[1].name.name, BAD_CAST observing_system.osc[1].name.value);
	xmlNewChild(p1, NULL, BAD_CAST observing_system.osc[1].type.name, BAD_CAST observing_system.osc[1].type.value);
	p1=p1->parent;p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST oa.target[0].ename, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].name.name, BAD_CAST oa.target[0].name.value);
	xmlNewChild(p1, NULL, BAD_CAST oa.target[0].type.name, BAD_CAST oa.target[0].type.value);
	p1=p1->parent;p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->fao->name, BAD_CAST NULL); /* File_Area_Observational node element */
	p1 = xmlNewChild(p1, NULL, BAD_CAST pds.po->fao->leaves[0]->name, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST pds.po->fao->leaves[0]->leaves[0]->name, BAD_CAST pds.po->fao->leaves[0]->leaves[0]->value);
	p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.name, BAD_CAST NULL);
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[0]->name, BAD_CAST BAD_CAST array2d.leaves[0]->value);
	attr = xmlSetProp(p1, array2d.leaves[0]->attributes[0].name,array2d.leaves[0]->attributes[0].value);
	p1=p1->parent;
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[1]->name, BAD_CAST array2d.leaves[1]->value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[2]->name, BAD_CAST array2d.leaves[2]->value);
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[3]->name, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[3]->leaves[0]->name, BAD_CAST array2d.leaves[3]->leaves[0]->value);
	p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4]->name, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4]->leaves[0]->name, BAD_CAST array2d.leaves[4]->leaves[0]->value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4]->leaves[1]->name, BAD_CAST array2d.leaves[4]->leaves[1]->value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[4]->leaves[2]->name, BAD_CAST array2d.leaves[4]->leaves[2]->value);
	p1=p1->parent;
	p1 = xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[5]->name, BAD_CAST NULL);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[5]->leaves[0]->name, BAD_CAST array2d.leaves[5]->leaves[0]->value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[5]->leaves[1]->name, BAD_CAST array2d.leaves[5]->leaves[1]->value);
	xmlNewChild(p1, NULL, BAD_CAST array2d.leaves[5]->leaves[2]->name, BAD_CAST array2d.leaves[5]->leaves[2]->value);
	p1=p1->parent;p1=p1->parent;p1=p1->parent;p1=p1->parent;

    /*
 *      * Dumping document to stdio or file
 *           
    xmlSaveFormatFileEnc(argc > 1 ? argv[1] : "-", doc, "UTF-8", 1);
*/
    xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
    xmlFreeDoc(doc);

    xmlCleanupParser();
	for(i=0;i<numproducts;i++){
		free(prodfnam[i]);
		free(pds.products[i]);
		free(pds.po->fao->leaves[0]->leaves[0]);		
	}
/*	free(pds.po->fao->leaves);
	free(pds.po->fao->leaves[0]); */
	free(prodfnam);
	/* prodfname freed */
	free(oa.target);
/*	for(i=0;i<6;i++)
		free(po.ia->leaves[i]);*/
	free(po.ia->leaves);
/*	free(oa.leaves[0]);
	*/
/*	free(oa.target);*/
	free(modification_history.leaves);
	free(array2d.leaves);
	free(oa.leaves);
	free(po.attributes); /* po.attributes freed */
	free(observing_system.osc);
  return status;
}
