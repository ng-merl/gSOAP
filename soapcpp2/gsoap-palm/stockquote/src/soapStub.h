/* soapStub.h
   Generated by gSOAP 2.7.0e from quote.h
   Copyright (C) 2001-2004 Genivia, Inc. All Rights Reserved.
   This software is released under one of the following three licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
   See README.txt for further details.
*/
#ifndef soapStub_H
#define soapStub_H
#include "stdsoap2.h"
#ifdef __cplusplus
extern "C" {
#endif

SOAP_BEGIN_NAMESPACE(soap)

/* Enumerations */

/* Classes and Structs */

/* ns:getQuoteResponse: */
struct ns__getQuoteResponse
{
	float *Result;	/* RPC return element */	/* optional element */
};

/* ns:getQuote: */
struct ns__getQuote
{
	char *symbol;	/* optional element */
};

/* SOAP Header: */
struct SOAP_ENV__Header
{
	void *dummy;	/* transient */
};

/* SOAP Fault Code: */
struct SOAP_ENV__Code
{
	char *SOAP_ENV__Value;	/* optional element */
	char *SOAP_ENV__Node;	/* optional element */
	char *SOAP_ENV__Role;	/* optional element */
};

/* SOAP-ENV:Detail: */
struct SOAP_ENV__Detail
{
	int __type;	/* any type of element <fault> (defined below) */
	void *fault;	/* transient */
	char *__any;
};

/* SOAP Fault: */
struct SOAP_ENV__Fault
{
	char *faultcode;	/* optional element */
	char *faultstring;	/* optional element */
	char *faultactor;	/* optional element */
	struct SOAP_ENV__Detail *detail;	/* optional element */
	struct SOAP_ENV__Code *SOAP_ENV__Code;	/* optional element */
	char *SOAP_ENV__Reason;	/* optional element */
	struct SOAP_ENV__Detail *SOAP_ENV__Detail;	/* optional element */
};

/* Types With Custom (De)serializers: */

/* Typedefs */
typedef char *_XML;
typedef char *_QName;

/* Extern */

/* Remote Methods */

SOAP_FMAC5 int SOAP_FMAC6 ns__getQuote(struct soap*, char *, float *);

/* Stubs */

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__getQuote(struct soap*, const char*, const char*, char *, float *);

/* Skeletons */

SOAP_FMAC5 int SOAP_FMAC6 soap_serve(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_request(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns__getQuote(struct soap*);

SOAP_END_NAMESPACE(soap)

#ifdef __cplusplus
}
#endif

#endif

/* end of soapStub.h */
