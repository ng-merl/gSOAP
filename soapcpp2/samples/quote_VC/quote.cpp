#include "soapH.h"	// include generated proxy and SOAP support

int main(int argc, char **argv)
{ struct soap soap;	// allocate gSOAP runtime environment
  char *sym;
  float q;
  if (argc <= 1)
    sym = "AOL";
  else
    sym = argv[1];
  soap_init(&soap);	// must initialize
  if (soap_call_ns__getQuote(&soap, "http://services.xmethods.net/soap", "", sym, &q) == 0)
    printf("\nCompany - %s    Quote - %f\n", sym, q);
  else
  { soap_print_fault(&soap, stderr);
    soap_print_fault_location(&soap, stderr);
  }
  return 0;
}

// The namespace mapping table is required and associates namespace prefixes with namespace names:
struct Namespace namespaces[] =
{//{"ns-prefix", "ns-name" [, "ns-name-input-pattern"]}
   {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/"}, // MUST be first
   {"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/"}, // MUST be second
   {"xsi", "http://www.w3.org/1999/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance"}, // MUST be third
   {"xsd", "http://www.w3.org/1999/XMLSchema", "http://www.w3.org/*/XMLSchema"},
   {"ns", "urn:xmethods-delayed-quotes"},	// Method namespace URI
   {NULL, NULL}
};


