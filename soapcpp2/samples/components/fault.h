/*	fault.h

	Defines SOAP Fault data structure shared by client and service modules

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

*/

//gsoap f schema namespace: http://websrv.cs.fsu.edu/~engelen/f.xsd

struct SOAP_ENV__Fault
{ // SOAP 1.1 Fault definitions:
  char *faultcode;
  char *faultstring;
  char *faultactor;
  char *detail;			// detail field type is user-definable
  // SOAP 1.2 Fault definitions:
  struct SOAP_ENV__Code *SOAP_ENV__Code;
  char *SOAP_ENV__Reason;
  char *SOAP_ENV__Detail;	// SOAP_ENV__Detail field type is user-definable
};
