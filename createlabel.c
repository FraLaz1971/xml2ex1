#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


int main(int argc, char **argv)
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;/* node pointers */
    xmlDtdPtr dtd = NULL;       /* DTD pointer */
    char buff[256];
    int i, j;

    LIBXML_TEST_VERSION;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "Product_Observational");
    xmlDocSetRootElement(doc, root_node);
    xmlNewChild(root_node, NULL, BAD_CAST "Identification_Area", BAD_CAST NULL);
    xmlNodePtr p1=root_node->children;
    xmlNewChild(p1, NULL, BAD_CAST "logical_identifier", BAD_CAST "TBD");
    xmlNewChild(p1, NULL, BAD_CAST "version_id", BAD_CAST "TBD");
    xmlNewChild(p1, NULL, BAD_CAST "title", BAD_CAST "TBD");
    xmlNewChild(p1, NULL, BAD_CAST "information_model_version", BAD_CAST "TBD");
    xmlNewChild(p1, NULL, BAD_CAST "product_class", BAD_CAST "TBD");
    p1=xmlNewChild(p1, NULL, BAD_CAST "Modification_History", BAD_CAST NULL);
    p1=xmlNewChild(p1, NULL, BAD_CAST "Modification_Detail", BAD_CAST NULL);
    xmlNewChild(p1, NULL, BAD_CAST "Modification_Date", BAD_CAST "no");
    xmlNewChild(p1, NULL, BAD_CAST "version_id", BAD_CAST "no");
    xmlNewChild(p1, NULL, BAD_CAST "description", BAD_CAST "no");
    p1=p1->parent;p1=p1->parent;;p1=p1->parent;
    xmlNewChild(p1, NULL, BAD_CAST "Observation_Area", BAD_CAST "no");
    /*
 *      * Dumping document to stdio or file
 *           */
    xmlSaveFormatFileEnc(argc > 1 ? argv[1] : "-", doc, "UTF-8", 1);

    xmlFreeDoc(doc);

    xmlCleanupParser();

  return 0;
}
