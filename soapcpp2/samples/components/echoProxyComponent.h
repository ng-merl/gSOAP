/*	echoProxyComponent.h

	Example echo client proxy component interface

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Compile:
	soapcpp2 -pecho echo.h
	c++ -c echoProxyComponent.cpp

	Link with baseC.o (see base.h)

*/

#define WITH_NOGLOBAL
#define WITH_NONAMESPACES	/* omit init with global namespace table */
#include "stdsoap2.h"
#include "echoStub.h"
#undef SOAP_FMAC1
#define SOAP_FMAC1 static
#include "echoEchoProxy.h"	/* do not include when using pure C */

void echo_init(struct soap*);

class EchoProxy: public Echo	/* remove when using pure C */
{ public:
  EchoProxy();
  void report() const;
  /* You can add other stuff here */
};
