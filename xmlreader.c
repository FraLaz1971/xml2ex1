#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
/* 
 * Enum xmlReaderTypes {
    XML_READER_TYPE_NONE = 0
    XML_READER_TYPE_ELEMENT = 1
    XML_READER_TYPE_ATTRIBUTE = 2
    XML_READER_TYPE_TEXT = 3
    XML_READER_TYPE_CDATA = 4
    XML_READER_TYPE_ENTITY_REFERENCE = 5
    XML_READER_TYPE_ENTITY = 6
    XML_READER_TYPE_PROCESSING_INSTRUCTION = 7
    XML_READER_TYPE_COMMENT = 8
    XML_READER_TYPE_DOCUMENT = 9
    XML_READER_TYPE_DOCUMENT_TYPE = 10
    XML_READER_TYPE_DOCUMENT_FRAGMENT = 11
    XML_READER_TYPE_NOTATION = 12
    XML_READER_TYPE_WHITESPACE = 13
    XML_READER_TYPE_SIGNIFICANT_WHITESPACE = 14
    XML_READER_TYPE_END_ELEMENT = 15
    XML_READER_TYPE_END_ENTITY = 16
    XML_READER_TYPE_XML_DECLARATION = 17
}
 * */
void process_attributes(xmlNode *node);
int main(int argc, char **argv) {
	int i;
	if (argc<2){
	  fprintf(stderr,"usage:%s <file.xml>\n",argv[0]);
	  return 1;
	}
	char *fname = argv[1];
    // Create a reader for a file with named given as input
    xmlTextReaderPtr reader = xmlReaderForFile(fname, NULL, 0);

    if (reader != NULL) {
        // Loop through the document, node by node
        int ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            // Get the node type and name
            int nodeType = xmlTextReaderNodeType(reader);
            const xmlChar *name = xmlTextReaderName(reader);
			printf("%d %d %s %d %d\n", 
			xmlTextReaderDepth(reader),
			xmlTextReaderNodeType(reader),
			name,
			xmlTextReaderIsEmptyElement(reader),
			xmlTextReaderHasValue(reader));
			const xmlChar *val;
			const xmlChar *localName;
			const xmlChar *namespaceURI;
			if(xmlTextReaderHasAttributes(reader)){
				for(i=1;xmlTextReaderGetAttributeNo(reader,i)!=0;i++){
					printf("%s ",xmlTextReaderGetAttributeNo(reader,i));				
				}
				puts("");
			xmlNodePtr node = xmlTextReaderCurrentNode(reader);
			xmlAttr* attribute = node->properties;
				process_attributes(node);
			}
			if(xmlTextReaderHasValue(reader)){
                const xmlChar *value = xmlTextReaderValue(reader);
                printf("Value: %s\n", value);
			}
            // Process based on node type
            if (nodeType == XML_READER_TYPE_ELEMENT) {
                printf("Element: %s\n", name);
            } else if (nodeType == XML_READER_TYPE_TEXT) {
                const xmlChar *value = xmlTextReaderValue(reader);
                printf("Text: %s\n", value);
            } else if (nodeType == XML_READER_TYPE_SIGNIFICANT_WHITESPACE) {
                printf("significant white space\n");
            }

            // Move to the next node
            ret = xmlTextReaderRead(reader);
        }

        // Check for parsing errors
        if (ret != 0) {
            fprintf(stderr, "Failed to parse XML\n");
        }

        // Clean up
        xmlFreeTextReader(reader);
    } else {
        fprintf(stderr, "Failed to open XML file\n");
    }

    return 0;
}

void process_attributes(xmlNode *node) {
    if (node == NULL || node->type != XML_ELEMENT_NODE) {
        return;
    }

    printf("node: %s\n", node->name);

    // Iterate over regular attributes (non-namespace)
    xmlAttr *attribute;
    for (attribute = node->properties; attribute != NULL; attribute = attribute->next) {
        xmlChar *attr_name = attribute->name;
        xmlChar *attr_value = xmlNodeListGetString(node->doc, attribute->children, 1);
        if (attr_name && attr_value) {
            printf("  Attribute: %s = %s\n", attr_name, attr_value);
            xmlFree(attr_value);
        }
    }

    // Iterate over namespace declarations
    xmlNs *namespace_decl;
    for (namespace_decl = node->nsDef; namespace_decl != NULL; namespace_decl = namespace_decl->next) {
        if (namespace_decl->prefix) {
            printf("  Namespace: xmlns:%s = %s\n", namespace_decl->prefix, namespace_decl->href);
        } else {
            // Default namespace
            printf("  Namespace: xmlns = %s\n", namespace_decl->href);
        }
    }
}
