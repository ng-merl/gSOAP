//////////////////////////////////////////////////////////////////////
//
// Contributed to gSOAP
//
// GSoapWinInet.cpp: implementation of the CGSoapWinInet class.
// ------------------------------------------------------
// Author:  Jack Kustanowitz (jackk@atomica.com)
// Date:    26 May, 2003
// Redistribution: Feel free to use, improve, and share.  I would appreciate
//					notification of any bugs found/fixed, or improvements made.
//					This code has not been extensively tested, so use at your own risk.  
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GSoapWinInet.h"

#include "afxinet.h"
#include "stdsoap2.h"

// static members
HINTERNET		CGSoapWinInet::m_hInternet;
HINTERNET		CGSoapWinInet::m_hConnect;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSoapWinInet::CGSoapWinInet()
{
	m_hInternet = NULL;
	m_hConnect = NULL;
}

CGSoapWinInet::~CGSoapWinInet()
{
	InternetCloseHandle(m_hConnect);
	m_hConnect = NULL;
	InternetCloseHandle(m_hInternet);
	m_hInternet = NULL;
}

// replaces gsoap's fopen function
int CGSoapWinInet::wi_open(struct soap *soap, const char *endpoint, const char *host, int port) 
{
	/*	From the GSOAP documentation:

		Called from a client proxy to open a connection to a Web Service located at endpoint. 
		Input parameters host and port are micro-parsed from endpoint. 
		Should return a valid file descriptor, or -1 and soap->error set to an error code. 
		Built-in gSOAP function: tcp_connect 
	*/

	// open a connection if we haven't done it yet
	if (!m_hInternet)
	{
		m_hInternet = InternetOpen(_T("GuruNet"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL );
		if (!m_hInternet)
		{
			// this ain't gonna work, get out now
			TRACE(_T("wi_open: Error in InternetOpen"));
			TRACE(_T("wi_open exits"));
			soap->error = GetLastError();
			return -1;
		}
		// set up the callback function so we get notifications
		InternetSetStatusCallback(m_hInternet, InternetStatusCallback);
	}

	// connect to the target url, if we haven't connected yet or if it was dropped
	if (!m_hConnect)
	{
		// Use the thread ID as the context.  Doesn't matter, should just be some form of ID
		m_hConnect = InternetConnect(m_hInternet, host, port, _T(""), _T(""), INTERNET_SERVICE_HTTP, NULL, AfxGetThread()->m_nThreadID);
		if (!m_hConnect)
		{
			// this ain't gonna work, get out now
			TRACE(_T("wi_open: Error in InternetConnect"));
			TRACE(_T("wi_open exits"));
			soap->error = GetLastError();
			return -1;
		}
	}

	// parse out the "object" (i.e. everything after the servername in the url),
	// and chuck the rest
	CString temp, obj;
	DWORD dwTemp;
	INTERNET_PORT portTemp;
	AfxParseURL(endpoint, dwTemp, temp, obj, portTemp);

	// open the Request -- be sure to set KeepAlive so that NTLM works (handshake
	// requires KeepAlives because it's in more than one stage)
	HINTERNET hReq = HttpOpenRequest(m_hConnect, "POST", obj, "HTTP/1.1", NULL, NULL, INTERNET_FLAG_KEEP_CONNECTION, AfxGetThread()->m_nThreadID);
	if (!hReq)
	{
		// this ain't gonna work, get out now
		InternetCloseHandle(m_hConnect);
		TRACE(_T("wi_open: Error in HttpOpenRequest"));
		TRACE(_T("wi_open exits"));
		soap->error = GetLastError();
		return -1;
	}

	// save the handle to the request to the soap object, and return it.
	soap->sendfd = (int) hReq;

	return (int) hReq;
}

// replaces gsoap's fsend function
int CGSoapWinInet::wi_send(struct soap *soap, const char *s, size_t n) 
{ 
	/*	From the GSOAP documentation:

		Called for all send operations to emit contents of s of length n. 
		Should return SOAP_OK, or a gSOAP error code. 
		Built-in gSOAP function: fsend 	
	*/

	// assume everything's gonna work out fine
	int nRet = SOAP_OK;

	// set up the buffer to send as the POST request
	LPCTSTR szSuffix = _T("\r\n");

	int postLength = n+strlen(szSuffix)+1;
	char* szPost = new char[postLength];
	memset(szPost, NULL, postLength);

	strncpy(szPost, s, n);
	strcat(szPost, szSuffix);

	// now szPost has the full text needed to be POSTed by this request

	BOOL bRetry = TRUE;
	while (bRetry)
	{
		// by default, try only once
		bRetry = FALSE;

		// open the request
		BOOL bRet = HttpSendRequest((void*) soap->sendfd, NULL, NULL, (void*) szPost, strlen(szPost));
		DWORD dwErr = 0;
		if (!bRet)
		{
			dwErr = GetLastError();
			TRACE(_T("Error in HttpSendRequest: GetLastError() returned %d"), dwErr);
			nRet = SOAP_HTTP_ERROR;
			break;
		}
			
		// get the status code from the response
		DWORD dwCode = 0;
		DWORD dwSize = sizeof(DWORD);
		bRet = HttpQueryInfo ((void*) soap->sendfd, HTTP_QUERY_STATUS_CODE |
				  HTTP_QUERY_FLAG_NUMBER, &dwCode, &dwSize, NULL);

		if (!bRet)
		{
			TRACE(_T("wi_send: Error in HttpSendRequest: GetLastError returned %d.\n"), GetLastError());
			nRet = SOAP_HTTP_ERROR;
			break;
		}

		TRACE(_T("wi_send: HTTP_STATUS_CODE=%d"), dwCode);

		if (dwCode == HTTP_STATUS_PROXY_AUTH_REQ)
		{
			// yup, got an error -- show the dialog & ask for a username/password,
			// saving the values to the hReq object so that we can use
			// InternetQueryOption to get them in a few lines
			DWORD dwRet = InternetErrorDlg (GetDesktopWindow(),
				  (void*) soap->sendfd, ERROR_INTERNET_INCORRECT_PASSWORD,
				  FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
				  FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
				  FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
				  NULL);

			if (dwRet == ERROR_INTERNET_FORCE_RETRY)
				bRetry = TRUE;
		}
	}

	delete [] szPost;

	return nRet; 
} 

// we will read this many bytes at a time using InternetReadFile (chosen arbitrarily)
#define BUFSIZE		1024

// replaces gsoap's frecv function
size_t CGSoapWinInet::wi_recv(struct soap *soap, char *s, size_t n) 
{ 
	/*	From the GSOAP documentation:

		Called for all receive operations to fill buffer s of maximum length n. 
		Should return the number of bytes read or 0 in case of an error, e.g. EOF. 
		Built-in gSOAP function: frecv 
	*/

	// clear out the buffer that we were provided with
	memset(s, NULL, n);

	// set up a temproray buffer, which will never be bigger than the buffer we were given
	char* szRaw = new char[n];
	::memset(szRaw, NULL, n);

	DWORD dwTotalRead = 0;		// running total of how much we've read so far
	DWORD dwRead = 0;			// how much have we read this time through?

	do
	{
		BOOL bRet = FALSE;

		// if we're about to read more than we have room for, DON'T.
		if ((dwTotalRead + BUFSIZE) > n)
			break;

		// read from the file, BUFSIZE at a time
		bRet = InternetReadFile((void*) soap->sendfd, szRaw+dwTotalRead, BUFSIZE, &dwRead);
		if (bRet)
			dwTotalRead += dwRead;
		else
		{
			DWORD dwErr = GetLastError();
			TRACE(_T("wi_send: Error in InternetReadFile: GetLastError returned %d.\n"), dwErr);
		}
	} while (dwRead != 0);		// if it's 0, we're done or there was an error -- in any case break out.

	// copy from our buffer into the buffer that was provided us.  Use memcpy
	// and not strcpy because strcpy will die if gzip is enabled since there might be NULLs.
	memcpy(s, szRaw, dwTotalRead);

	// clean up
	delete [] szRaw;

	// return the number of bytes read
	return dwTotalRead;
} 

// replaces gsoap's fposthdr function
int CGSoapWinInet::wi_posthdr(struct soap *soap, const char *key, const char *val)  
{
	/*	From the GSOAP documentation:

		Called by http_post and http_response (through the callbacks). 
		Emits HTTP key: val header entries. Should return SOAP_OK, or a gSOAP error code. 
		Built-in gSOAP function: http_post_header.  
	*/

	// set up the header
	CString header = CString(key) + CString(_T(": ")) + CString(val);
	HttpAddRequestHeaders((void*) soap->sendfd, header, header.GetLength(), HTTP_ADDREQ_FLAG_ADD_IF_NEW );

	return SOAP_OK; 
}

// replaces gsoap's fclose function
int CGSoapWinInet::wi_close(struct soap *soap) 
{
	/*	From the GSOAP documentation:

		Called by client proxy multiple times, to close a socket connection before a 
		new socket connection is established and at the end of communications 
		when the SOAP_IO_KEEPALIVE flag is not set and soap.keep_alive != 0 
		(indicating that the other party supports keep alive). 
		Should return SOAP_OK, or a gSOAP error code. 
		Built-in gSOAP function: tcp_disconnect 
	*/

	InternetCloseHandle((HINTERNET) soap->sendfd);
	soap->sendfd = NULL;

	// don't close the connection on each query
//	InternetCloseHandle(m_hConnect);
//	m_hConnect = NULL;

	return SOAP_OK;
}

void CGSoapWinInet::InternetStatusCallback (
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
)
{
	// this is mostly for tracing, although it is important to set the m_hConnect
	// member variable to NULL when the connection closes.
	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_CLOSING_CONNECTION:
		TRACE(_T("Thread %d: INTERNET_STATUS_CLOSING_CONNECTION\n"), dwContext);
		break;
	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		TRACE(_T("Thread %d: INTERNET_STATUS_CONNECTED_TO_SERVER\n"), dwContext);
		break;
	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		TRACE(_T("Thread %d: INTERNET_STATUS_CONNECTING_TO_SERVER\n"), dwContext);
		break;
	case INTERNET_STATUS_CONNECTION_CLOSED:
		// the server closed the connection, so update our member variable
		m_hConnect = NULL;
		TRACE(_T("Thread %d: INTERNET_STATUS_CONNECTION_CLOSED\n"), dwContext);
		break;
	case INTERNET_STATUS_CTL_RESPONSE_RECEIVED:
		TRACE(_T("Thread %d: INTERNET_STATUS_CTL_RESPONSE_RECEIVED\n"), dwContext);
		break;
	case INTERNET_STATUS_DETECTING_PROXY:
		TRACE(_T("Thread %d: INTERNET_STATUS_DETECTING_PROXY\n"), dwContext);
		break;
	case INTERNET_STATUS_HANDLE_CLOSING:
		TRACE(_T("Thread %d: INTERNET_STATUS_HANDLE_CLOSING\n"), dwContext);
		break;
	case INTERNET_STATUS_HANDLE_CREATED:
		TRACE(_T("Thread %d: INTERNET_STATUS_HANDLE_CREATED\n"), dwContext);
		break;
	case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
		TRACE(_T("Thread %d: INTERNET_STATUS_INTERMEDIATE_RESPONSE\n"), dwContext);
		break;
	case INTERNET_STATUS_NAME_RESOLVED:
		TRACE(_T("Thread %d: INTERNET_STATUS_NAME_RESOLVED\n"), dwContext);
		break;
	case INTERNET_STATUS_PREFETCH:
		TRACE(_T("Thread %d: INTERNET_STATUS_PREFETCH\n"), dwContext);
		break;
	case INTERNET_STATUS_RECEIVING_RESPONSE:
		TRACE(_T("Thread %d: INTERNET_STATUS_RECEIVING_RESPONSE\n"), dwContext);
		break;
	case INTERNET_STATUS_REDIRECT:
		TRACE(_T("Thread %d: INTERNET_STATUS_REDIRECT\n"), dwContext);
		break;
	case INTERNET_STATUS_REQUEST_COMPLETE:
		TRACE(_T("Thread %d: INTERNET_STATUS_REQUEST_COMPLETE\n"), dwContext);
		break;
	case INTERNET_STATUS_REQUEST_SENT:
		TRACE(_T("Thread %d: INTERNET_STATUS_REQUEST_SENT\n"), dwContext);
		break;
	case INTERNET_STATUS_RESOLVING_NAME:
		TRACE(_T("Thread %d: INTERNET_STATUS_RESOLVING_NAME\n"), dwContext);
		break;
	case INTERNET_STATUS_RESPONSE_RECEIVED:
		TRACE(_T("Thread %d: INTERNET_STATUS_RESPONSE_RECEIVED\n"), dwContext);
		break;
	case INTERNET_STATUS_SENDING_REQUEST:
		TRACE(_T("Thread %d: INTERNET_STATUS_SENDING_REQUEST\n"), dwContext);
		break;
	case INTERNET_STATUS_STATE_CHANGE:
		TRACE(_T("Thread %d: INTERNET_STATUS_STATE_CHANGE\n"), dwContext);
		break;
	case INTERNET_STATE_CONNECTED:
		TRACE(_T("Thread %d: INTERNET_STATE_CONNECTED\n"), dwContext);
		break;
	case INTERNET_STATE_DISCONNECTED:
		TRACE(_T("Thread %d: INTERNET_STATE_DISCONNECTED\n"), dwContext);
		break;
	case INTERNET_STATE_DISCONNECTED_BY_USER:
		TRACE(_T("Thread %d: INTERNET_STATE_DISCONNECTED_BY_USER\n"), dwContext);
		break;
	case INTERNET_STATE_IDLE:
		TRACE(_T("Thread %d: INTERNET_STATE_IDLE\n"), dwContext);
		break;
	case INTERNET_STATE_BUSY:
		TRACE(_T("Thread %d: INTERNET_STATE_BUSY\n"), dwContext);
		break;
	case INTERNET_STATUS_USER_INPUT_REQUIRED:
		TRACE(_T("Thread %d: INTERNET_STATUS_USER_INPUT_REQUIRED\n"), dwContext);
		break;
	case INTERNET_STATUS_COOKIE_SENT:
		TRACE(_T("Thread %d: INTERNET_STATUS_COOKIE_SENT\n"), dwContext);
		break;
	}

}
