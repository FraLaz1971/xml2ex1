#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
/*
 * typedef enum {
    XML_ELEMENT_NODE=		1,
    XML_ATTRIBUTE_NODE=		2,
    XML_TEXT_NODE=		    3,
    XML_CDATA_SECTION_NODE=	4,
    XML_ENTITY_REF_NODE=	5,
    XML_ENTITY_NODE=		6,
    XML_PI_NODE=		    7,
    XML_COMMENT_NODE=		8,
    XML_DOCUMENT_NODE=		9,
    XML_DOCUMENT_TYPE_NODE=	10,
    XML_DOCUMENT_FRAG_NODE=	11,
    XML_NOTATION_NODE=		12,
    XML_HTML_DOCUMENT_NODE=	13,
    XML_DTD_NODE=		    14,
    XML_ELEMENT_DECL=		15,
    XML_ATTRIBUTE_DECL=		16,
    XML_ENTITY_DECL=		17,
    XML_NAMESPACE_DECL=		18,
    XML_XINCLUDE_START=		19,
    XML_XINCLUDE_END=		20
#ifdef LIBXML_DOCB_ENABLED
   ,XML_DOCB_DOCUMENT_NODE=	21
#endif
} xmlElementType;

 * */
void process_node_with_all_attributes(xmlNode *node) {
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

void process_all_nodes(xmlNode *a_node) {
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		 
         printf("node type: %d content: %s ",cur_node->type,cur_node->content);
        if (cur_node->type == XML_ELEMENT_NODE) {
            process_node_with_all_attributes(cur_node);
        } else if (cur_node->type == XML_TEXT_NODE) {
			printf("text ");
		}
        // Recursively process children
        process_all_nodes(cur_node->children);
    }
}

int main(int argc, char** argv) {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    if(argc<2){
      fprintf(stderr,"usage:%s <file.xml>\n",argv[0]);
      return 1;
    }
    // The name of your XML file
    const char *filename = argv[1];

    // Initialize libxml2
    LIBXML_TEST_VERSION

    // Parse the file
    doc = xmlParseFile(filename);
    if (doc == NULL) {
        fprintf(stderr, "Error: unable to parse XML file %s\n", filename);
        return 1;
    }
	// xmlParserCtxtPtr xp=xmlNewParserCtxt();
    // Get the root element
    root_element = xmlDocGetRootElement(doc);
    if (root_element == NULL) {
        fprintf(stderr, "Error: empty document\n");
        xmlFreeDoc(doc);
        return 1;
    }

    // Process all nodes, including the root
    process_all_nodes(root_element);

    // Free the document and cleanup
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return 0;
}
