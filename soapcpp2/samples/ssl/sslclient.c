#include "soapH.h"
#include "ssl.nsmap"

const char server[] = "https://localhost:18081";

int main()
{ struct soap soap;
  double a, b, result;
  a = 10.0;
  b = 20.0;
  soap_init(&soap);
  if (soap_ssl_client_context(&soap,
    SOAP_SSL_DEFAULT,
    "client.pem",	/* keyfile: see SSL docs on how to obtain this file */
    "password",		/* password to read the key file */
    NULL, 		/* cacert: optional for client. To verify client by server (not implemented yet) */
    NULL,		/* capath */
    NULL		/* if randfile!=NULL: use a file with random data to seed randomness */ 
  ))
  { soap_print_fault(&soap, stderr);
    exit(1);
  }
  soap.require_server_auth = 0;
  if (soap_call_ns__add(&soap, server, "", a, b, &result) == SOAP_OK)
    fprintf(stdout, "Result: %f + %f = %f\n", a, b, result);
  else
    soap_print_fault(&soap, stderr);
  soap_destroy(&soap); /* C++ */
  soap_end(&soap);
  soap_done(&soap);
  return 0;
}
