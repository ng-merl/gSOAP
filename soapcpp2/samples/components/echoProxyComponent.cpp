/*	echoProxyComponent.cpp

	Example echo client proxy component implementation

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Compile:
	soapcpp2 -pecho echo.h
	c++ -c echoProxyComponent.cpp

	Link with baseC.o (see base.h)

*/

#include "echoProxyComponent.h"	/* sets SOAP_FMAC1 = static */
#include "echoC.cpp"		/* local static (de)serialization code */
#undef SOAP_FMAC1
#define SOAP_FMAC1
#include "echoClient.cpp"	/* export the stub code */
#undef SOAP_NMAC
#define SOAP_NMAC static
#include "Echo.nsmap"		/* local static namespaces */

void echo_init(struct soap *soap)
{ soap_init(soap);
  soap->namespaces = namespaces; /* must do this explicitly (because compiled WITH_NONAMESPACES) */
}

EchoProxy::EchoProxy() : Echo()
{ soap->namespaces = namespaces; /* must do this explicitly (because compiled WITH_NONAMESPACES) */
  /* Example credentials for authentication */
  soap->header = (struct SOAP_ENV__Header*)soap_malloc(soap, sizeof(struct SOAP_ENV__Header));
  soap->header->h__credentials = (struct h__authentication*)soap_malloc(soap, sizeof(struct h__authentication));
  soap->header->h__credentials->userid = "foo";
  soap->header->h__credentials->passwd = "bar";
}

void EchoProxy::report() const
{ if (soap->error)
  { soap_print_fault(soap, stderr);
    soap_print_fault_location(soap, stderr);
  }
}
