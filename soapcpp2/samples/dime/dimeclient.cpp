//	dimeclt.cpp
//	Example client for simple image server
//	Run from command line with file name argument followed by optional service endpoint URL

#include "soapH.h"
#include "dime.nsmap"

const char *endpoint = "http://websrv.cs.fsu.edu/~engelen/dimesrv.cgi";

int main(int argc, char **argv)
{ struct soap soap;
  xsd__base64Binary image;
  char *name;
  const char *url;
  soap_init(&soap);
  if (argc < 2)
  { name = "back.jpg";	// no command line argument: use default image
    url = endpoint;
  }
  else
  { name = argv[1];	// this is the name of the image
    if (argc < 3)
      url = endpoint;
    else
      url = argv[2];	// endpoint URL
  }
  soap.connect_timeout = 10;
  // soap.connect_flags = SO_NOSIGPIPE;
  // soap.socket_flags = MSG_NOSIGNAL;
  if (soap_call_ns__getImage(&soap, url, "", name, image))
    soap_print_fault(&soap, stderr);
  else
    printf("Got image size=%d type=%s\n", image.__size, image.type?image.type:"");
  soap_destroy(&soap);
  soap_end(&soap);
  return SOAP_OK;
}
