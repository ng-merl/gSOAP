//gsoap ns service name: webserver
//gsoap ns service namespace: http://www.cs.fsu.edu/~engelen/webserver.wsdl
//gsoap ns schema namespace: urn:demowebserver

int ns__add(int, int, int*);		// HTTP POST request - response
int ns__sub(int, int, int*);		// HTTP POST request - response
int ns__mul(int, int, int*);		// HTTP POST request - response
int ns__div(int, int, int*);		// HTTP POST request - response

int ns__addResponse_(int, void);	// to produce SOAP/XML upon HTTP GET response
int ns__subResponse_(int, void);	// to produce SOAP/XML upon HTTP GET response
int ns__mulResponse_(int, void);	// to produce SOAP/XML upon HTTP GET response
int ns__divResponse_(int, void);	// to produce SOAP/XML upon HTTP GET response
