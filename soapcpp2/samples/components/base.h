/*	base.h

	Defines base gSOAP environment shared by client and service modules

	Imports SOAP Fault and SOAP Header data structures to be shared by
	client and service modules

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Compile:
	soapcpp2 -i -pbase base.h
	c++ -c baseC.cpp

*/

#define WITH_NONAMESPACES	/* omit init with global namespace table */

#include "stdsoap2.cpp"		/* include entire run-time lib source code */

#import "header.h"		/* user-defined headers */

#import "fault.h"		/* user-defined faults */
