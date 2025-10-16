/**
 * section: xmlReader
 * synopsis: Parse an XML telemetry file with an xmlReader and dump the ascii hex dump of the TM packets
 * purpose: Demonstrate the use of xmlReaderForFile() to parse an XML file
 *          and use the information about the nodes found in the process.
 *          (Note that the XMLReader functions require libxml2 version later
 *          than 2.6.)
 * usage: ./xmltm2ascii <filename>
 * test: ./xmltm2ascii test2.xml > reader1.asc
 * author: Francesco Lazzarotto
 * mailto:francesco.lazzarotto@inaf.it
 */

#include <stdio.h>
#include <string.h>
#include <libxml/xmlreader.h>

#ifdef LIBXML_READER_ENABLED

/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */

static void processNode(xmlTextReaderPtr reader) {
    const xmlChar *name, *value;
    name = xmlTextReaderConstName(reader);
    if (name == NULL)
	name = BAD_CAST "--";
	/*for the node type see https://www.gnu.org/software/dotgnu/pnetlib-doc/System/Xml/XmlNodeType.html 
	 * type==3 is text */
    if ((xmlTextReaderDepth(reader)==5)&&(xmlTextReaderNodeType(reader)==3)&&
    xmlTextReaderHasValue(reader)){
	    value = xmlTextReaderConstValue(reader);	    
	    if (value == NULL)
			printf("\n");
	    else {
	      printf("%s\n", value);
	    }
	}
}

/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse and print information about an XML file.
 */
static void streamFile(const char *filename) {
    xmlTextReaderPtr reader;
    int ret;
    reader = xmlReaderForFile(filename, NULL, 0);
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            processNode(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            fprintf(stderr, "xmltm2ascii::streamFile() %s : failed to parse\n", filename);
        }
    } else {
        fprintf(stderr, "xmltm2ascii::streamFile() Unable to open %s\n", filename);
    }
}

int main(int argc, char **argv) {
    if (argc != 2){
		fprintf(stderr,"usage:%s <infile.xml>\n",argv[0]);
        return(1);
    }
    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    streamFile(argv[1]);

    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();
    /*
     * this is to debug memory for regression tests     
   */ 
    xmlMemoryDump();
return(0);
}

#else
int main(void) {
    fprintf(stderr, "XInclude support not compiled in\n");
    return(0);
}
#endif
