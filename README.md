# xml2ex1
###
Example programs using the libxml2 c library. Contains also programs using the libpng library.
###
- createxml.c ---> simple program that dump a xml file on the stdout, useful to be modified for real applications.
- pngprog001.c ---> reads a png image and creates another png image
- readxml.c ---> reads an xml file and prints nodes/elements/attributes/namespaces
- readxml2.c ---> same as readxml.c but validate also
- temp1.c
- temp2.c ---> create binary array files
- testWriter.c
- xmltm2ascii.c
- createlabel.c ---> create a xml PDS4 label for the MESSENGER global TDM
- readraw.c
- readraw16.c
- readxml.c
- testWriter.c
- xmltm2ascii.c ---> saves xml file telemetry packets data in ascii hex dump (each row is a packet)
- archivepds.c ---> creates a xml label for the given products files with the metadata given in the configuration file. #include "pds.h".
It takes in input a configuration file with all the data archiving specifications, the number of products, the products list and gives the result on the standard output.
For example the messhlob.conf file contains data to archive messenger DEM images:
        BITPIX    16
        SIGN  signed
        ENDIAN   LSB
        WIDTH  23040
        HEIGHT 11520
        NPI 2
        PI_00 href=\"https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1A10.sch\">schematypens=\"http://purl.oclc.org/dsdl/schematron\"
        PI_01 href=\"https://pds.nasa.gov/pds4/mission/mess/v1/PDS4_MESS_1B00_1020.sch\">schematypens=\"http://purl.oclc.org/dsdl/schematron\"
        NPOATTR 4
        PO_ATTR_00_NAME xmlns
        PO_ATTR_00_VALUE http://pds.nasa.gov/pds4/pds/v1
        PO_ATTR_01_NAME xmlns:mess
        PO_ATTR_01_VALUE http://pds.nasa.gov/pds4/mission/mess/v1
        PO_ATTR_02_NAME xmlns:xsi
        PO_ATTR_02_VALUE http://www.w3.org/2001/XMLSchema-instance
        PO_ATTR_03_NAME xsi:schemaLocation
        PO_ATTR_03_VALUE http://pds.nasa.gov/pds4/pds/v1>https://pds.nasa.gov/pds4/pds/v1/PDS4_PDS_1A10.xsd>http://pds.nasa.gov/pds4/mission/mess/v1>https://pds.nasa.gov/pds4/mission/mess/v1/PDS4_MESS_1B00_1020.xsd
        PROD_LID urn:nasa:pds:mess_mdis_raw:data_edr:mdis2015091_dat
        PROD_VID 1.0
        TITLE Test>to>develop>BC>SIMBIO-SYS>STC>TDM>PDS4>labels
        IM_VERS 1.10.1.0
        PROD_CLASS Product_Observational
        MOD_DATE 2025-10-20
        VERSID 1.0
        DESCR PDS4>product>label>generated>by>SETM>libraries
        TSTART 2011-03-18T00:00:00Z
        TSTOP 2014-11-01T00:00:00Z
        PURP Science
        PROC_LEV Raw
        RES_DESCR Summary>Of>Results
        LAMBDA Visible
        DOMAIN Surface
        DISCIPL Imaging
        INVEST_AREA messenger
        INVEST_TYPE Mission
        INV_LID urn:nasa:pds:context:investigation:mission.messenger
        REFTYPE data_to_investigation
        NOSC_COMP 2
        COMPNAME_00 messenger
        COMPTYPE_00 Spacecraft
        COMPNAME_01 mercury>dual>imaging>system>narrow>angle>camera
        COMPTYPE_01 Instrument
        TARGET_NAME mercury
        TARGET_TYPE Planet
        MISSION mess:MESSENGER
        PHASE Mercury>Orbit>Year>5
        CLOCK_START N/A
        CLOCK_STOP N/A
        PRODID mdisedr
        SWNAME pipe-mdis2edr
        SWVERS 1.1

###
`./archivepds <conf.file> <nproducts> <prod1> <prod2> ... <prodn> > outputlabel.xml`
###
## archivepds
execution examples:
###
`./archivepds arpds.conf 1 mercury1_500x320.raw > mylabel.xml`
###
`./archivepds messglob.conf 1 MSGR_DEM_USG_SC_J_V02.IMG | tee out.xml`
###
- png2ascii.c
- read_pds4_lbl.c
- readraw16.c
- sval.c
- write_gs_png.c
- archivepds2.c
- createxml.c
- png_gs_2ascii.c
- read_png.c
- readraw8.c
- write_png.c
- archivepds_cas.c
- parse.c
- png_rgb_to_gs.c
- readconf.c
- readrawf.c
- xmlreader.c
- ascii2raw.c
- parse2.c
- readlabel.c
- test001.c
- ascii2raw_i.c
- pds.c
- read_gs_png.c
- readraw.c
- zpipe.c
- XmlReaderTypes.h
- pds.h --> contains the data structures of the pds(4) label
- prodid.c --> creates the product id
		product_id = <instrument_acronym>_[<processing_level_acronym>]_[<type>]_[<subunit>]_<descriptor>
		[ ] = optional fields
