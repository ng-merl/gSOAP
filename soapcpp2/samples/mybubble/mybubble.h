// See http://www.xmethods.com "mybubble" Company Profile service
//gsoap ns service name: mybubble
//gsoap ns service location: http://www.mybubble.com:8080/soap/servlet/rpcrouter
//gsoap ns service executable:
//gsoap ns schema namespace: urn:MyBubble-SoapServices
typedef char *xsd__string;
int ns__getServiceResponsePublic(xsd__string serviceName, xsd__string inputText, xsd__string *_return);
