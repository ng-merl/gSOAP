/*	echoServiceComponent.h

	Example echo service component interface

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Compile:
	soapcpp2 -pecho echo.h
	c++ -c echoServiceComponent.cpp

	Link with baseC.o (see base.h)

*/

#define WITH_NOGLOBAL
#define WITH_NONAMESPACES	/* omit init with global namespace table */
#include "stdsoap2.h"
#undef SOAP_FMAC1
#define SOAP_FMAC1 static
#include "echoStub.h"

/* echo service component is statically derived from struct soap: do NOT use virtual methods */
struct EchoService : public soap
{ public:
  EchoService();
  ~EchoService();
  int serve();
  void report();
  /* You can add other stuff here */
};
