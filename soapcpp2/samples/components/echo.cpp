/*	echo.cpp

	Demonstrates echo proxy and service components combined into one
	application (echo.cpp)

	These example components are based on the same echo.h header file spec,
	but components are generally not required to be defined by one header
	file only. The SOAP Header and SOAP Fault definitions must be identical
	however (use #import to import them from a file).

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Needs:
	echo.h:		header file spec for client proxy and service components
	header.h:	SOAP Header spec
	fault.h:	SOAP Fault spec
	echo.cpp:	application that uses the proxy and service components
	echoProxyComponent.h:		header file of proxy component
	echoProxyComponent.cpp:		proxy component implementation
	echoServiceComponent.h:		header file of service component
	echoServiceComponent.cpp:	service component implementation

	Compile:
	soapcpp2 -i -pbase base.h
	c++ -c baseC.cpp
	soapcpp2 -pecho echo.h
	c++ -c echoProxyComponent.cpp
	c++ -c echoServiceComponent.cpp
	c++ echo.cpp baseC.o echoProxyComponent.o echoServiceComponent.o

	Run:
	echo "<string>"
	This will use the client proxy component to send the string to a
	server

	Install as CGI:
	as CGI application this program will use the service component

*/

#include <iostream.h>

#include "echoProxyComponent.h"
#include "echoServiceComponent.h"

int main(int argc, char **argv)
{ if (argc < 2)
  { EchoService e;
    e.serve();	/* CGI-style server using stdin/out, just like calling soap_serve(soap_new()) */
  }
  else
  { EchoProxy e;
    char *s;
    if (e.echo(argv[1], s))
      e.report();
    else
      cout << "Echo: " << s << endl;
    /* C-style code:
    struct soap soap;
    echo_init(&soap);
    if (soap_call_ns__echo(&soap, "http://websrv.cs.fsu.edu/~engelen/echo.cgi", "echo", argv, s))
      soap_print_fault(&soap, stderr);
    else
      printf("Echo: %s\n", s);
    */
  }
  return 0;
}
