/*	stringtest.cpp

	Example use of std::string

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.
*/

#include "soapH.h"
#include <iostream>

char endpoint[] = "http://www.cs.fsu.edu/~engelen/interop2.cgi";

using namespace std;

int main()
{ struct soap soap;
  soap_init(&soap);
  std::string a("hello world!"), b("OK");
  struct ns__echoStringResponse r;
  cout << "Testing single string echo:" << endl;
  if (soap_call_ns__echoString(&soap, endpoint, "", a, r))
    soap_print_fault(&soap, stderr);
  else
    cout << r._return << endl;
  ArrayOfstring *A = soap_new_ArrayOfstring(&soap, -1); // alloc non-array (-1)
  ArrayOfstring *B = soap_new_ArrayOfstring(&soap, -1); // alloc non-array (-1)
  A->resize(3);
  A->__ptr[0] = a;
  A->__ptr[1] = b;
  A->__ptr[2] = b;
  cout << "Testing array of string echo:" << endl;
  if (soap_call_ns__echoStringArray(&soap, endpoint, "", A, B))
    soap_print_fault(&soap, stderr);
  else
    cout << B->__ptr[0] << endl << B->__ptr[1] << endl;
  soap_destroy(&soap);
  soap_end(&soap);
  soap_done(&soap);
  cout << "Done" << endl;
  return 0;
}

ArrayOfstring::ArrayOfstring()
{ }

ArrayOfstring::~ArrayOfstring()
{ } // gSOAP will deallocate

void ArrayOfstring::resize(int len)
{ __ptr = soap_new_xsd__string(soap, len);
  __size = len;
}

xsd__string& ArrayOfstring::operator[](int index) const
{ if (!__ptr || index < 0 || index >= __size)
   cerr <<  "Array index out of bounds" << endl;
  return __ptr[index];
}

struct Namespace namespaces[] =
{ {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/"},
  {"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/"},
  {"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance"},
  {"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema"},
  {"ns", "http://soapinterop.org/"},
  {NULL, NULL}
};
