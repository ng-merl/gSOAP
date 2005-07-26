/* soapClient.c
   Generated by gSOAP 2.7.4 from quote.h
   Copyright (C) 2000-2005, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/
#include "soapH.h"
#ifdef __cplusplus
extern "C" {
#endif

SOAP_SOURCE_STAMP("@(#) soapClient.c ver 2.7.4 2005-07-26 14:31:00 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__getQuote(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *symbol, float *Result)
{	struct ns__getQuote soap_tmp_ns__getQuote;
	struct ns__getQuoteResponse *soap_tmp_ns__getQuoteResponse;
	soap->encodingStyle = NULL;
	soap_tmp_ns__getQuote.symbol = symbol;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_ns__getQuote(soap, &soap_tmp_ns__getQuote);
	soap_begin_count(soap);
	if (soap->mode & SOAP_IO_LENGTH)
	{	soap_envelope_begin_out(soap);
		soap_putheader(soap);
		soap_body_begin_out(soap);
		soap_put_ns__getQuote(soap, &soap_tmp_ns__getQuote, "ns:getQuote", "");
		soap_body_end_out(soap);
		soap_envelope_end_out(soap);
	}
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__getQuote(soap, &soap_tmp_ns__getQuote, "ns:getQuote", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_float(soap, Result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_tmp_ns__getQuoteResponse = soap_get_ns__getQuoteResponse(soap, NULL, "ns:getQuoteResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (Result && soap_tmp_ns__getQuoteResponse->Result)
		*Result = *soap_tmp_ns__getQuoteResponse->Result;
	return soap_closesock(soap);
}

#ifdef __cplusplus
}
#endif

/* End of soapClient.c */
