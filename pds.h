#ifndef PDS_H
#define PDS_H
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/tree.h>
#define MAXFNAML 1024 /* maximum file name[MAXFNAML] allowed (conventional) may include path */
#define MAXLEAV 16

enum endian{LSB,MSB};

struct ATTRIBUTE {
	char name[MAXFNAML];
	char value[MAXFNAML];
};

struct OBSERVING_SYSTEM_COMPONENT{
	char ename[MAXFNAML];
	char value[MAXFNAML];
	struct ELEMENT *name;
	struct ELEMENT *type;
};

struct ELEMENT {
	struct ELEMENT *parent;
	struct ATTRIBUTE *attributes;
	int nattr; /* number of attributes */
	char name[MAXFNAML];
	char value[MAXFNAML];
	struct OBSERVING_SYSTEM_COMPONENT osc[MAXLEAV];
	struct ELEMENT *leaves;
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};

struct MODIFICATION_DETAIL{
	struct ELEMENT mdate; /* modification date */
	struct ELEMENT versid; /* version id */
	struct ELEMENT descr; /* description of the modification */
};
struct TIME_COORDINATES {
	char name[MAXFNAML];
	char value[MAXFNAML];
	struct ELEMENT tstart;
	struct ELEMENT tstop;
};
struct INTERNAL_REFERENCE{
	char name[MAXFNAML];
	char value[MAXFNAML];
	struct ELEMENT lid_reference;
	struct ELEMENT reference_type;
};
struct INVESTIGATION_AREA {
	char ename[MAXFNAML];
	char value[MAXFNAML];
	struct ELEMENT name;
	struct ELEMENT type;
	struct INTERNAL_REFERENCE *iref;
};
struct IDENTIFICATION_AREA {
	struct PRODUCT_OBSERVATIONAL *parent;
	struct ATTRIBUTE *attributes;
	int nattr; /* number of attributes */
	char name[MAXFNAML];
	char value[MAXFNAML];
	struct ELEMENT leaves[MAXLEAV];
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};
struct TARGET_IDENTIFICATION{
	char ename[MAXFNAML];
	char value[MAXFNAML];
	struct ELEMENT name;
	struct ELEMENT type;
};
struct OBSERVATION_AREA {
	struct PRODUCT_OBSERVATIONAL *parent;
	struct ATTRIBUTE *attributes;
	int nattr; /* number of attributes */
	char name[MAXFNAML];
	char value[MAXFNAML];
	struct TIME_COORDINATES *times;
	struct INVESTIGATION_AREA *investa;
	struct TARGET_IDENTIFICATION *target;
	struct ELEMENT leaves[MAXLEAV]; /* leaves[0]=observing system*/
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};

struct FILE_AREA_OBSERVATIONAL {
	struct PRODUCT_OBSERVATIONAL *parent;
	struct ATTRIBUTE *attributes;
	int nattr; /* number of attributes */
	char name[MAXFNAML];
	char value[MAXFNAML];
	struct ELEMENT leaves[MAXLEAV];
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};
struct ARRAY_2D_IMAGE{
	char name[MAXFNAML];
	char value[MAXFNAML];
	struct ELEMENT leaves[MAXLEAV];
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};
struct PRODUCT_OBSERVATIONAL {
	struct ATTRIBUTE *attributes;
	struct IDENTIFICATION_AREA *ia;
	struct OBSERVATION_AREA *oa;
	struct FILE_AREA_OBSERVATIONAL *fao;
	struct ELEMENT oleaves[MAXLEAV]; /* other elements, in case */
	char name[MAXFNAML];
	char value[MAXFNAML];
};

struct PDS {
	int version;
	char xmlintest[MAXFNAML];
	char xml_model[10][MAXFNAML];
	char ifname[MAXFNAML];
	char ofname[MAXFNAML];
	char **pfnames; /* names of the product files referred by the label*/
	FILE **products; /* product files pointed by the pds label */
	int numproducts;
	struct PRODUCT_OBSERVATIONAL *po;
};

int init_pds(FILE *xmlp,struct PDS *pds,char **prodfnam,char **argv);
int read_pds(FILE *ixmlp,struct PDS *pds);
int write_pds(FILE *oxmlp,struct PDS *pds);
int close_pds(FILE *xmlp,struct PDS *pds);
#endif  /* PDS_H */
