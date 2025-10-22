#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

void process_node_with_all_attributes(xmlNode *node) {
    if (node == NULL || node->type != XML_ELEMENT_NODE) {
        return;
    }

    printf("Processing attributes for node: %s\n", node->name);

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
        if (cur_node->type == XML_ELEMENT_NODE) {
            process_node_with_all_attributes(cur_node);
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
