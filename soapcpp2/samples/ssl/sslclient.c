#include "soapH.h"
#include "ssl.nsmap"

const char server[] = "https://localhost:18081";

int main()
{ struct soap soap;
  double a, b, result;
  a = 10.0;
  b = 20.0;
  soap_init(&soap);
  soap.require_server_auth = 0;
  soap.password = "password"; /* may be omitted when server does not require client authentication */
  soap.keyfile = "client.pem"; /* may be omitted when server does not require client authentication */
  soap.cafile = "cacert.pem"; /* may be omitted when server does not require client authentication */
  if (soap_call_ns__add(&soap, server, "", a, b, &result) == SOAP_OK)
    fprintf(stdout,"Result = %f\n", result);
  else
    soap_print_fault(&soap, stderr);
  soap_end(&soap);
  soap_done(&soap);
  return 0;
}
