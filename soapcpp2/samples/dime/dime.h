//gsoap ns service name: dime
//gsoap ns service namespace: http://websrv.cs.fsu.edu/~engelen/dime.wsdl
//gsoap ns service location: http://websrv.cs.fsu.edu/~engelen
//gsoap ns service executable: dimesrv.cgi
//gsoap ns schema  namespace: urn:dime
class xsd__base64Binary
{ unsigned char *__ptr;
  int __size;
  char *id;
  char *type;
  char *options;
};
int ns__getImage(char *name, xsd__base64Binary &image);
