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

struct DESCR{ /* product_id descriptor field in details */
 	char obs_id[MAXFNAML]; /* observation id  */
 	char obs_type[MAXFNAML]; /* observation type */
 	char meas_type[MAXFNAML]; /* measurement type */
    char start_date[24]; /* UTC time of the first (and last) measurement in the data product */
    char stop_date[24]; /* UTC time of the first last measurement in the data product */
    char start_orb[6]; /* orbit number of the first measurement in the data product, padded with zeroes) */
    char stop_orb[6]; /* orbit number of the last measurement in the data product, padded with zeroes) */
};
struct PROD_ID{
 	char instr_acr[4]; /* instrument acronym */
 	char proclev_acr[4]; /* processing level acronym */
 	char type[6]; /* for primary products: hk | sc
						  for supplementary products calib | thumb | qla | geo | doc | misc */
	char subinstr_id[4]; /* or sub-unit id e.g. (srn_)eln o (sim_)stc ... */
	char descriptor[64]; /*
	dditional information e.g.
  - observation id
  - observation type
  - measurement type
  - start / stop date (<YYYYMMDDThhmmss> or
    <YYYYMMDDThhmmss>_<YYYYMMDDThhmmss>; UTC time of the first (and last) measurement in the data product
  - orbit number (<NNNNN> or <NNNNN>_<MMMMM>; orbit number of the first (and
    last) measurement in the data product, padded with zeroes)
	*/
};

struct LID{
	char bundle_id[MAXFNAML]; /* bundle identifier */
	char collect_id[MAXFNAML]; /* collection identifier */
	char prod_id[MAXFNAML]; /* product identifier */
	char vid[MAXFNAML]; /* version ID */
};

struct ATTRIBUTE {
	char name[MAXFNAML];
	char value[MAXFNAML];
};
struct ELEMENT {
	struct ELEMENT *parent;
	struct ATTRIBUTE *attributes;
	int nattr; /* number of attributes */
	char name[MAXFNAML];
	char value[MAXFNAML];
//	struct OBSERVING_SYSTEM_COMPONENT osc[MAXLEAV];
	struct ELEMENT *leaves;
	int nleaves; /* number of leaves */
	struct ELEMENT *prev;
	struct ELEMENT *succ;
};
struct INTERNAL_REFERENCE{
	char name[MAXFNAML];
	char value[MAXFNAML];
	struct ELEMENT lid_reference;
	struct ELEMENT reference_type;
};
struct OBSERVING_SYSTEM_COMPONENT{
	char ename[MAXFNAML];
	char value[MAXFNAML];
	char descr[MAXFNAML];
	struct INTERNAL_REFERENCE ir;
	struct ELEMENT *name;
	struct ELEMENT *type;
};
struct OBSERVING_SYSTEM {
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
	struct INTERNAL_REFERENCE *iref;
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
