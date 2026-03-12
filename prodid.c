#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pds.h"
/*
 * <instrument_acronym>_[<processing_level_acronym>]_[<type>]_[<subunit>]_<descriptor>
[ ] = optional fields

struct PROD_ID{
 	char instr_acr[MAXFNAML]; // instrument acronym
 	char proclev_acr[MAXFNAML]; // processing level acronym
 	char type[MAXFNAML]; // for primary products: hk | sc
						 // for supplementary products calib | thumb | qla | geo | doc | misc
	char subinstr_id[MAXFNAML]; // or sub-unit id e.g. srn_eln o sim_stc ...
	char descriptor[MAXFNAML];
	additional information e.g.
  - observation id
  - observation type
  - measurement type
  - start / stop date (<YYYYMMDDThhmmss> or
    <YYYYMMDDThhmmss>_<YYYYMMDDThhmmss>; UTC time of the first (and last)
    measurement in the data product)
  - orbit number (<NNNNN> or <NNNNN>_<MMMMM>; orbit number of the first (and
    last) measurement in the data product, padded with zeroes)
};
----------------------------
Processing    Processing
Level ID      Level Acronym
----------------------------
Telemetry  |  tm
----------------------------
Raw        |  raw
----------------------------
Partially  |  par
Processed  |
----------------------------
Calibrated |  cal
----------------------------
Derived    |  der
----------------------------

----------------------------
MPO Instruments
---------------------------------------------------------------------------------------------------------------------------------------------------------------
Instrument |  Instrument | Instrument | Instrument Name                            | Instrument Type               | Sub-Instrument | Sub-Units
Host ID    | ID          | Acronym    |                                            |                               | IDs            | Ids
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | bela        | bel        | BepiColombo Laser Altimeter                | Altimeter                     |                |
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | berm        | ber        | BepiColombo Radiation Monitor              |                               |                |
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | isa         | isa        | Italian Spring Accelerometer               | Accelerometer                 |                |
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | mpo-mag     | mag        | MPO Mercury Magnetometer                   | Magnetometer                  |                | ib
           |             |            |                                            |                               |                | ob
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | mertis      | mer        | Mercury Thermal Infrared Spectrometer      | Radiometer                    | tir            |
           |             |                                                         | Spectrometer                  | tis            |
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | mgns        | mgn        | Mercury Gamma Ray and Neutron              | Neutron Detector              |                |
           |             |            | Spectrometer                               |                               |                |
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | mixs        | mix        | Mercury Imaging X-Ray Spectrometer         | Spectrometer                  |                |
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | more        | mre        | Mercury Orbiter Radio Science Exoeriment   | Radio Science                 |                |
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | phebus      | phe        | Probing of Hermean Exosphere by            | Ultraviolet Spectrometer      |                |
           |             |            | Ultraviolet Spectroscopy                   |                               |                |
---------------------------------------------------------------------------------------------------------------------------------------------------------------
mpo        | serena      | srn        | Search fpr Exosphere Refilling and Emitted | Neutral Particle Detector     | elena          |
           |             |            | Neutral Abundances                         | Mass Spectrometer             | mipa           |
           |             |            |                                            | Plasma Analyzer (Ion monitor) | picam          |
           |             |            |                                            | Mass Spectrometer             | strofio        |
---------------------------------------------------------------------------------------------------------------------------------------------------------------





Sub-Instrument
IDs
Sub-Unit
IDs
tir
tis
mixs-t
mixs-c
elena
mipa
picam
strofio
ib
ob
fpaC
fpaT
dpu
fuv
euv



*/

int print(FILE *ofp, struct PROD_ID *pdid);
int read(FILE *ifp, struct PROD_ID *pdid);

int main(int argc, char **argv){
    struct PROD_ID prodid;
    struct DESCR descr;
    char *confile;
    FILE *cfp;
    int res;

    if (argc<2){
        fprintf(stderr, "usage:%s <file.conf>\n", argv[0]);
        return 1;
    }
    /* read configuration file */
    confile = argv[1];
    cfp = fopen(confile, "r");
    if(cfp==NULL){
     perror("configuration file not present");
     exit(1);
    }
    res=fscanf(cfp, "INSTR_ACR %3s\n",&prodid.instr_acr);
    res=fscanf(cfp, "PROCLEV_ACR %3s\n",&prodid.proclev_acr);
    res=fscanf(cfp, "TYPE %10s\n",&prodid.type);
    res=fscanf(cfp, "SUBINSTR_ID %5s\n",&prodid.subinstr_id);
    res=fscanf(cfp, "DESCRIPTOR %23s\n",&prodid.descriptor);
/*
INSTR_ACR mag
PROCLEV_ACR der
TYPE sc
SUBINSTR_ID ob
DESCRIPTOR a001_e2k_00000_20230803

 */
    fclose(cfp);
    res = print(stdout, &prodid );
    return 0;
}
/* <instrument_acronym>_[<processing_level_acronym>]_[<type>]_[<subunit>]_<descriptor, 
 * example: mag_der_sc_ob_a001_e2k_00000_20230803.xml
 * */
int print(FILE *ofp, struct PROD_ID *pdid){
  char totprodid[128];
  fprintf(stderr, "instrument_acronym = %s\n", pdid->instr_acr);
  fprintf(stderr, "processing_level_acronym = %s\n", pdid->proclev_acr);
  fprintf(stderr, "data type = %s\n", pdid->instr_acr);
  fprintf(stderr, "instrument_subunit = %s\n", pdid->subinstr_id);
  fprintf(stderr, "descriptor = %s\n", pdid->descriptor);
  snprintf(totprodid, 36, "%3s_%s_%s_%s_%s",pdid->instr_acr, pdid->proclev_acr, pdid->subinstr_id,pdid->descriptor);
  fprintf(ofp, "%36s\n",totprodid);
  return 0;
}

int read(FILE *ifp, struct PROD_ID *pdid){

  return 0;
}
/*
 * see BC-SGS-TN-043
 * Archiving Guide (Annex to PSA PDS4 Archiving Guide)
 * by Santa Martinez (ESA)
 *
o <instrument_acronym>, see Table 3 and Table 4
o <processing_level_acronym>: see Table 7
o <type>, defined based on the type of products resulting from each instrument, e.g.
  - For primary products, possible split is: hk | sc
  - For supplementary products, specify type: calib | thumb | qla | geo | doc | misc (list
    controlled by SGS) (qla=quick look analysis)
o <sub-instrument_id>, or <sub-unit_id>, this can be the sub-instrument, sensor,
   detector/ sub-unitidentifier (as applicable), see Table 3 and Table 4
o <descriptor>: additional information e.g.
  - observation id
  - observation type
  - measurement type
  - start / stop date (<YYYYMMDDThhmmss> or
    <YYYYMMDDThhmmss>_<YYYYMMDDThhmmss>; UTC time of the first (and last)
    measurement in the data product)
  - orbit number (<NNNNN> or <NNNNN>_<MMMMM>; orbit number of the first (and
    last) measurement in the data product, padded with zeroes)

BC SIMBIO-SYS data files (HK)
sim_raw_hk_hric_cruise_ico11_2024-04-08.csv
sim_raw_hk_hric_cruise_ico11_2024-04-08.xml
sim_raw_hk_me_cruise_ico11_2024-04-08.csv
sim_raw_hk_me_cruise_ico11_2024-04-08.xml
sim_raw_hk_stc_cruise_ico11_2024-04-08.csv
sim_raw_hk_stc_cruise_ico11_2024-04-08.xml
sim_raw_hk_vihi_cruise_ico11_2024-04-08.csv
sim_raw_hk_vihi_cruise_ico11_2024-04-08.xml
* 
* 
* e.g.:
* instrument_acronym = sim
* processing_level_acronym = raw
* type = hk
* sub-instrument_id = stc
* descriptor = cruise_ico11_2024-04-08
* mission-phase = cruise
* measurement-type = ico11
* date = 2024-04-08
 */
