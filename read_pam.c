#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

/* Funzione per estrarre i metadati dai nodi <MDI> */
void print_metadata(xmlNode * a_node) {
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            // Cerchiamo il tag <MDI>
            if (xmlStrEqual(cur_node->name, (const xmlChar *)"MDI")) {
                // Leggiamo l'attributo "key"
                xmlChar *key = xmlGetProp(cur_node, (const xmlChar *)"key");
                // Leggiamo il contenuto testuale
                xmlChar *value = xmlNodeGetContent(cur_node);

                if (key) {
                    printf("%s=%s\n", key, (value && xmlStrlen(value) > 0) ? (char *)value : "n/a");
                }

                xmlFree(key);
                xmlFree(value);
            }
        }
        // Ricorsione sui figli del nodo corrente
        print_metadata(cur_node->children);
    }
}

int main(int argc, char **argv) {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    if (argc != 2) {
        printf("Utilizzo: %s <file_xml_pam>\n", argv[0]);
        return 1;
    }

    // Inizializza la libreria e leggi il file
    LIBXML_TEST_VERSION
    doc = xmlReadFile(argv[1], NULL, 0);

    if (doc == NULL) {
        fprintf(stderr, "Error: impossible to analyze the file %s\n", argv[1]);
        return 1;
    }

    // Ottieni l'elemento radice <PAMDataset>
    root_element = xmlDocGetRootElement(doc);

    if (root_element != NULL) {
        printf("Extracting Metadata from: %s\n", argv[1]);
        printf("----------------------------------------\n");
        print_metadata(root_element);
    }

    // Pulizia
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}
