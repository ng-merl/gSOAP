/*	dimeclient.cpp

	Example DIME client for simple image server. This DIME client
	demonstrates the new gSOAP DIME streaming feature.

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Run from command line. The first optional argument is the image file
	name. The second optional argument is the service endpoint URL. The
	third optional argument is the file name to save the image file to.
*/

#include "soapH.h"
#include "dime.nsmap"

const char *endpoint = "http://websrv.cs.fsu.edu/~engelen/dimesrv.cgi";

// streaming DIME callbacks
static void *dime_write_open(struct soap*, const char*, const char*, const char*);
static void dime_write_close(struct soap*, void*);
static int dime_write(struct soap*, void*, const char*, size_t);

int main(int argc, char **argv)
{ struct soap soap;
  xsd__base64Binary image;
  char *name;
  const char *url;
  soap_init(&soap);
  if (argc > 3)
  { // use DIME streaming callbacks to save the file on the fly
    soap.user = (void*)argv[3]; // soap.user is always available for user data
    soap.fdimewriteopen = dime_write_open;
    soap.fdimewriteclose = dime_write_close;
    soap.fdimewrite = dime_write;
  }
  if (argc < 2)
  { name = "back.jpg";	// no command line argument: use default image and endpoint
    url = endpoint;
  }
  else
  { name = argv[1];	// this is the name of the image
    if (argc < 3)
      url = endpoint;
    else
      url = argv[2];	// second argument is optional endpoint URL
  }
  soap.connect_timeout = 10;
  // soap.connect_flags = SO_NOSIGPIPE;
  // soap.socket_flags = MSG_NOSIGNAL;
  if (soap_call_ns__getImage(&soap, url, "", name, image))
    soap_print_fault(&soap, stderr);
  else if (argc > 3)
    printf("Got image type=%s through streaming DIME\n", image.type?image.type:""); // image.__ptr = file handle and image.__size == 0 when streaming was used
  else
    printf("Got image size=%d type=%s\n", image.__size, image.type?image.type:"");
  soap_destroy(&soap);
  soap_end(&soap);
  return SOAP_OK;
}

static void *dime_write_open(struct soap *soap, const char *id, const char *type, const char *options)
{ FILE *handle = NULL;
  // we can return NULL without setting soap->error if we don't want to use the streaming callback for this DIME attachment
  handle = fopen((char*)soap->user, "wb");
  if (handle)
    printf("Streaming image id=%s type=%s into file %s\n", id, type, (char*)soap->user);
  else
  { soap->error = SOAP_EOF; // could not open file for writing
    soap->errnum = errno; // get reason
  }
  return (void*)handle;
}

static void dime_write_close(struct soap *soap, void *handle)
{ fclose((FILE*)handle);
}

static int dime_write(struct soap *soap, void *handle, const char *buf, size_t len)
{ size_t nwritten;
  while (len)
  { nwritten = fwrite(buf, 1, len, (FILE*)handle);
    if (!nwritten)
    { soap->errnum = errno; // get reason
      return SOAP_EOF;
    }
    len -= nwritten;
    buf += nwritten;
  }
  return SOAP_OK;
}

