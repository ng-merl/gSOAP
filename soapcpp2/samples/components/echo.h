/*	echo.h

	Example echo definitions

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	See echo.cpp for more details

*/

#import "header.h"		/* user-defined headers */

#import "fault.h"		/* user-defined faults */

//gsoap ns service name: Echo
//gsoap ns service namespace: http://websrv.cs.fsu.edu/~engelen/Echo.wsdl
//gsoap ns service location: http://websrv.cs.fsu.edu/~engelen/echo.cgi

//gsoap ns schema namespace: urn:echo

//gsoap ns service method-documentation: echo copies a string
//gsoap ns service method-header-part: echo h__credentials
//gsoap ns service method-action: echo "echo"
int ns__echo(char *in, char *&out);
