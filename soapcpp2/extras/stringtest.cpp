/*	stringtest.cpp

	Example use of std::string

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.
*/

#include "soapH.h"
#include <iostream.h>

int main()
{ struct soap soap;
  soap_init(&soap);
  std::string a("hello world!");
  std::string b;
  if (soap_call_ns__echoString(&soap, "http://www.cs.fsu.edu/~engelen/interop2.cgi", "", a, b))
    soap_print_fault(&soap, stderr);
  else
    cout << b << endl;
  return 0;
}

struct Namespace namespaces[] =
{ {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/"},
  {"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/"},
  {"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance"},
  {"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema"},
  {"ns", "http://soapinterop.org/"},
  {NULL, NULL}
};
