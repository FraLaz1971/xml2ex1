#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/tree.h>
#define MAXFNAML 256 /* maximum file name allowed (conventional) may include path */
struct ATTRIBUTE {
	char * name;
	char * value;
};

struct ELEMENT {
	struct ELEMENT *parent;
	struct ATTRIBUTE *attributes;
	int nattr; /* number of attributes */
	char * name;
	char *value;
	struct OBSERVING_SYSTEM_COMPONENT **osc;
	struct ELEMENT **leaves;
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};
struct TIME_COORDINATES {
	char * name;
	char *value;
	struct ELEMENT *tstart;
	struct ELEMENT *tstop;
};
struct INTERNAL_REFERENCE{
	char * name;
	char *value;
	struct ELEMENT *lid_reference;
	struct ELEMENT *reference_type;
};
struct INVESTIGATION_AREA {
	char * ename;
	char *value;
	struct ELEMENT *name;
	struct ELEMENT *type;
	struct INTERNAL_REFERENCE *iref;
};
struct IDENTIFICATION_AREA {
	struct PRODUCT_OBSERVATIONAL *parent;
	struct ATTRIBUTE *attributes;
	int nattr; /* number of attributes */
	char * name;
	char *value;
	struct ELEMENT **leaves;
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};
struct TARGET_IDENTIFICATION{
	char * ename;
	char *value;
	struct ELEMENT *name;
	struct ELEMENT *type;
};
struct OBSERVING_SYSTEM_COMPONENT{
	char * ename;
	char *value;
	struct ELEMENT *name;
	struct ELEMENT *type;
};
struct OBSERVATION_AREA {
	struct PRODUCT_OBSERVATIONAL *parent;
	struct ATTRIBUTE *attributes;
	int nattr; /* number of attributes */
	char * name;
	char *value;
	struct TIME_COORDINATES *times;
	struct INVESTIGATION_AREA *investa;
	struct TARGET_IDENTIFICATION *target;
	struct ELEMENT **leaves; /* leaves[0]=observing system*/
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};

struct FILE_AREA_OBSERVATIONAL {
	struct PRODUCT_OBSERVATIONAL *parent;
	struct ATTRIBUTE *attributes;
	int nattr; /* number of attributes */
	char * name;
	char *value;
	struct ELEMENT **leaves;
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};

struct PRODUCT_OBSERVATIONAL {
	struct ATTRIBUTE *attributes;
	struct IDENTIFICATION_AREA *ia;
	struct OBSERVATION_AREA *oa;
	struct FILE_AREA_OBSERVATIONAL *fao;
	struct ELEMENT ** oleaves; /* other elements, in case */
	char * name;
	char *value;
};

struct PDS {
	int version;
	char *xmlintest;
	char *xml_model;
	char *ifname;
	char *ofname;
	char **pfnames; /* names of the product files referred by the label*/
	FILE **products; /* product files pointed by the pds label */
	int numproducts;
	struct PRODUCT_OBSERVATIONAL *po;
};

int init_pds(FILE *xmlp,struct PDS *pds,char **prodfnam);
int read_pds(FILE *ixmlp,struct PDS *pds);
int write_pds(FILE *oxmlp,struct PDS *pds);
int close_pds(FILE *xmlp,struct PDS *pds);
