/*	dimesrv.cpp

	Example simple image server using DIME

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Runs as CGI (not multi-threaded) or multi-threaded stand-alone
	Web service

	NOTE: THE SERVER WILL ONLY SEND FILES THAT ARE IN THE CURRENT DIR FOR
	SECURITY REASONS. HOWEVER, THE AUTHOR IS NOT RESPONSIBLE FOR ANY DAMAGES
	THAT MAY RESULT FROM THE USE OF THIS PROGRAM.
*/

// For stand-alone Web service functionality, run from the command line with
// port number as command line argument, e.g.
// > dimesrv 18085 &
// Use port 80 to install and run as HTTP Web server accessible over the Web
// Change the 'endpoint' in 'dimeclt.cpp' to
// endpoint="http://machine:18085"
// where 'machine' is the host name of the machine on which the service runs.
// The service is multi-threaded. Multi-threading is not required, but can
// improve QoS. Remove the pthread code to obtain a non-multi-threaded service.

#include "soapH.h"
#include "dime.nsmap"
#include <pthread.h>	// use Pthreads
#include <sys/stat.h>	// use fstat() for streaming DIME

#define BACKLOG (100)		// Max. request backlog
#define MAX_FILE_SIZE (10000)	// Max. file size

static void *process_request(void*);

// streaming DIME callbacks
static void *dime_read_open(struct soap*, void*, const char*, const char*, const char*);
static void dime_read_close(struct soap*, void*);
static size_t dime_read(struct soap*, void*, char*, size_t);

int main(int argc, char **argv)
{ struct soap soap;
  soap_init(&soap);
  if (argc < 2)		// no args: this is a CGI application
  { soap_serve(&soap);	// serve request
    soap_destroy(&soap);// cleanup class instances
    soap_end(&soap);	// cleanup
    soap_done(&soap);	// cleanup
  }
  else
  { struct soap *tsoap;
    pthread_t tid;
    int port;
    int m, s, i;
    port = atoi(argv[1]);	// port is first command line argument
    m = soap_bind(&soap, NULL, port, BACKLOG);
    if (m < 0)
    { soap_print_fault(&soap, stderr);
      exit(1);
    }
    fprintf(stderr, "Socket connection successful %d\n", m);
    // soap.accept_timeout = 24*60*60;	// die after 24 hrs waiting
    for (i = 1; ; i++)
    { s = soap_accept(&soap);
      if (s < 0)
      { if (soap.errnum)
          soap_print_fault(&soap, stderr);
	else
          fprintf(stderr, "Server timed out\n");
	break;
      }
      fprintf(stderr, "Thread %d accepts socket %d connection from IP %d.%d.%d.%d\n", i, s, (int)(soap.ip>>24)&0xFF, (int)(soap.ip>>16)&0xFF, (int)(soap.ip>>8)&0xFF, (int)soap.ip&0xFF);
      tsoap = soap_copy(&soap);
      pthread_create(&tid, NULL, (void*(*)(void*))process_request, (void*)tsoap);
    }
  }
  soap_done(&soap);
  return 0;
}

void *process_request(void *soap)
{ pthread_detach(pthread_self());
  soap_serve((struct soap*)soap);	// serve pending request
  soap_destroy((struct soap*)soap);	// remove class instances
  soap_end((struct soap*)soap);		// clean up
  soap_done((struct soap*)soap);	// detach
  free(soap);
  return NULL;
}

int ns__getImage(struct soap *soap, char *name, xsd__base64Binary &image)
{ if (name)
  { FILE *fd = NULL;
    struct stat sb;
    int i, c;
    // do some checks on the file name to verify it is local:
    if (!strchr(name, '/') && !strchr(name, '\\') && !strchr(name, ':'))
      fd = fopen(name, "rb");
    if (!fd)
      return soap_receiver_fault(soap, "Cannot open file", NULL);
    if (!fstat(fileno(fd), &sb) && sb.st_size > 0)
    { // since we can get the length of the file, we can stream it
      soap->fdimereadopen = dime_read_open;
      soap->fdimereadclose = dime_read_close;
      soap->fdimeread = dime_read;
      image.__ptr = (unsigned char*)fd; // must set to non-NULL (this is our fd handle which we need in the callbacks)
      image.__size = sb.st_size; // must set size
    }
    else // don't know the size, so buffer it
    { image.__ptr = (unsigned char*)soap_malloc(soap, MAX_FILE_SIZE);
      for (i = 0; i < MAX_FILE_SIZE; i++)
      { if ((c = fgetc(fd)) == EOF)
          break;
        image.__ptr[i] = c;
      }
      fclose(fd);
      image.__size = i;
    }
    image.type = "image/jpeg";
    image.options = soap_dime_option(soap, 0, "My picture");
  }
  else
    return soap_receiver_fault(soap, "Name required", NULL);
  return SOAP_OK;
}

static void *dime_read_open(struct soap *soap, void *handle, const char *id, const char *type, const char *options)
{ // we should return NULL without setting soap->error if we don't want to use the streaming callback for this DIME attachment. The handle contains the non-NULL __ptr field value which should have been set in the application.
  // the value of the handle can be changed and will be passed on to the fdimeread and fdimereadclose callbacks. The value will not affect the __ptr field.
  return handle;
}

static void dime_read_close(struct soap *soap, void *handle)
{ fclose((FILE*)handle);
}

static size_t dime_read(struct soap *soap, void *handle, char *buf, size_t len)
{ return fread(buf, 1, len, (FILE*)handle);
}

