//////////////////////////////////////////////////////////////////////
//
// Contributed to gSOAP
//
// GSoapWinInet.h: interface for the CGSoapWinInet class.
// ------------------------------------------------------
// Purpose: Allow redirection of gsoap through the WinInet API,
//			to get all the configuration options IE sets (proxies,
//			etc.) for free, such that "if IE works, this works."
// Author:  Jack Kustanowitz (jackk@atomica.com)
// Date:    26 May, 2003
// Use: To include this in your project, simply reassign some of the gsoap 
//		function pointers as follows (where m_psoap is a pointer to the main soap object):
//
//			m_psoap->fopen = CGSoapWinInet::wi_open;
//			m_psoap->fsend = CGSoapWinInet::wi_send;
//			m_psoap->frecv = CGSoapWinInet::wi_recv;
//			m_psoap->fposthdr = CGSoapWinInet::wi_posthdr;
//			m_psoap->fclose = CGSoapWinInet::wi_close;
// 
//		and instantiate the CGSoapWinInet class somewhere it can stay alive 
//		(this is to manage the handle returned by InternetOpen).
// Redistribution: Feel free to use, improve, and share.  I would appreciate
//					notification of any bugs found/fixed, or improvements made.  
//					This code has not been extensively tested, so use at your own risk.  
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GSOAPWININET_H__5363B97B_B538_492F_A392_229D6256D252__INCLUDED_)
#define AFX_GSOAPWININET_H__5363B97B_B538_492F_A392_229D6256D252__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wininet.h"

class CGSoapWinInet  
{
public:
	CGSoapWinInet();
	virtual ~CGSoapWinInet();

	static int wi_send(struct soap *soap, const char *s, size_t n) ;
	static size_t wi_recv(struct soap *soap, char *s, size_t n) ;
	static int wi_open(struct soap *soap, const char *endpoint, const char *host, int port);
	static int wi_close(struct soap *soap) ;
	static int wi_posthdr(struct soap *soap, const char *key, const char *val) ;

protected:
	static HINTERNET		m_hInternet;
	static HINTERNET		m_hConnect;
	static void CALLBACK InternetStatusCallback (    HINTERNET hInternet,
								DWORD_PTR dwContext,
								DWORD dwInternetStatus,
								LPVOID lpvStatusInformation,
								DWORD dwStatusInformationLength);

};

#endif // !defined(AFX_GSOAPWININET_H__5363B97B_B538_492F_A392_229D6256D252__INCLUDED_)
