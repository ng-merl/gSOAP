/*	echoServiceComponent.cpp

	Example echo client proxy component implementation

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Compile:
	soapcpp2 -pecho echo.h
	c++ -c echoServiceComponent.cpp

	Link with baseC.o (see base.h)

*/

#include "echoServiceComponent.h"	/* sets SOAP_FMAC1 = static */
#include "echoC.cpp"		/* local static (de)serialization code */
#include "echoServer.cpp"	/* local static skeleton code */
#undef SOAP_NMAC
#define SOAP_NMAC static
#include "Echo.nsmap"		/* local static namespaces */

/* must provide remote service methods as C functions (see echo.h): */
int ns__echo(struct soap *soap, char *in, char *&out)
{ EchoService *echo = (EchoService*)soap;	/* when soap_serve(this) is called, we pass an EchoService object which we get here */
  if (!echo->header
   || !echo->header->h__credentials
   || strcmp(echo->header->h__credentials->userid, "foo")
   || strcmp(echo->header->h__credentials->passwd, "bar")
     )
    return soap_sender_fault(echo, "Not authorized", NULL);
  out = in;
  echo->header = NULL;	/* do not return header back to client (old header is GCed with soap_end()) */
  return SOAP_OK;
}

EchoService::EchoService()
{ soap_init(this);
  namespaces = ::namespaces; /* must do this explicitly (because compiled WITH_NONAMESPACES) */
}

EchoService::~EchoService()
{ soap_done(this);	/* detach cleanly */
}

int EchoService::serve()
{ return soap_serve(this);
}

void EchoService::report()
{ if (error)
  { soap_print_fault(this, stderr);
    soap_print_fault_location(this, stderr);
  }
}
