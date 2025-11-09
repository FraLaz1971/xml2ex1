#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/schematron.h>
#include <libxml/xmlerror.h>
void myErrorHandler(void *userData, xmlErrorPtr error);
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <XML file> <Schematron file or URL>\n", argv[0]);
        return 1;
    }

    const char *xmlFile = argv[1];
    const char *schFile = argv[2];

    xmlDocPtr doc = NULL;
    xmlSchematronParserCtxtPtr parserCtxt = NULL;
    xmlSchematronPtr schema = NULL;
    xmlSchematronValidCtxtPtr validCtxt = NULL;
    int ret = -1;
    xmlSetStructuredErrorFunc(NULL, (xmlStructuredErrorFunc)myErrorHandler);
    /* Initialize the libxml2 library */
    xmlInitParser();
    LIBXML_TEST_VERSION

    /* Load the XML document */
    doc = xmlReadFile(xmlFile, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse XML document '%s'\n", xmlFile);
        goto cleanup;
    }

    /* Create the Schematron parser context (URL or local path) */
    parserCtxt = xmlSchematronNewParserCtxt(schFile);
    if (parserCtxt == NULL) {
        fprintf(stderr, "Failed to create Schematron parser context\n");
        goto cleanup;
    }

    /* Parse the Schematron schema */
    schema = xmlSchematronParse(parserCtxt);
    if (schema == NULL) {
        fprintf(stderr, "Failed to parse Schematron schema '%s'\n", schFile);
        goto cleanup;
    }

    /* Create a Schematron validation context */
    validCtxt = xmlSchematronNewValidCtxt(schema, 0);
    if (validCtxt == NULL) {
        fprintf(stderr, "Failed to create Schematron validation context\n");
        goto cleanup;
    }

    /* Optional: set up error and warning handlers */
    xmlSchematronSetValidStructuredErrors(validCtxt,
        (xmlStructuredErrorFunc)fprintf, stderr);

    /* Perform validation */
    ret = xmlSchematronValidateDoc(validCtxt, doc);

    printf("Document '%s' is valid according to Schematron '%s'? res=%d\n", xmlFile, schFile, ret);
    if (ret == 0) {
        printf("Document '%s' is valid according to Schematron '%s'\n", xmlFile, schFile);
    } else if (ret > 0) {
        printf("Document '%s' is NOT valid (Schematron validation failed)\n", xmlFile);
    } else {
        printf("Validation failed due to an internal error\n");
    }

cleanup:
    if (validCtxt) xmlSchematronFreeValidCtxt(validCtxt);
    if (schema) xmlSchematronFree(schema);
    if (parserCtxt) xmlSchematronFreeParserCtxt(parserCtxt);
    if (doc) xmlFreeDoc(doc);
    xmlCleanupParser();

    return (ret > 0) ? 1 : 0;
}

void myErrorHandler(void *userData, xmlErrorPtr error) {
    fprintf(stderr, "XML error: %s\n", error->message);
}

