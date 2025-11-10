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
- archivepds.c ---> create a xml label for the given products files with the metadata given in the configuration file
###
`./archivepds <conf.file> <nproducts> <prod1> <prod2> ... <prodn> > outputlabel.xml` 
###
## archivepds
execution examples:
###
`./archivepds arpds.conf 1 mercury1_500x320.raw > mylabel.xml`
###
`./archivepds messglob.conf 1 MSGR_DEM_USG_SC_J_V02.IMG | tee out.xml`
