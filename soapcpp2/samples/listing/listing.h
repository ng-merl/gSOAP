//gsoap ns schema namespace: urn:xmethodsServicesManager
//gsoap ns3 schema namespace: http://www.xmethods.net/XMethodsListingsService.xsd
struct ns3__SOAPService
{ int ID;
  char *name;
  char *owner;
  char *description;
  char *homepageURL;
  char *endpoint;
  char *SOAPAction;
  char *methodNamespaceURI;
  char *serviceStatus;
  char *methodName;
  char *dateCreated;
  char *downloadURL;
  char *wsdlURL;
  char *instructions;
  char *contactEmail;
  char *serverImplementation;
};
struct ArrayOfSOAPService {struct ns3__SOAPService *__ptr; int __size;};
int ns__getAllSOAPServices(void *_, struct ArrayOfSOAPService &return_);
