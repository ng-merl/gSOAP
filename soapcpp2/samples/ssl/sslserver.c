#include "soapH.h"
#include "ssl.nsmap"
#include <pthread.h>
#include <signal.h>	/* use sighandlers to catch interupts */

void *process_request(void *);

void sigpipe_handle(int);

int main()
{ int m, s; /* master and slave sockets */
  pthread_t tid;
  struct soap soap, *tsoap;
  signal(SIGPIPE, sigpipe_handle);
  soap_init(&soap);
  soap.keyfile = "server.pem";	/* see SSL docs on how to obtain this file */
  soap.password = "password";	/* your password */
  soap.cafile = "cacert.pem";	/* see SSL docs on how to obtain this file */
  soap.dhfile = "dh512.pem";	/* see SSL docs on how to obtain this file */
  soap.randfile = NULL;		/* if randfile!=NULL: use a file with random data to seed randomness */ 
  soap.accept_timeout = 600;	/* server times out after 10 minutes of inactivity */
  soap.recv_timeout = 30;	/* if read stalls, then timeout after 30 seconds */
  m = soap_bind(&soap, NULL, 18081, 100);
  if (m < 0)
  { soap_print_fault(&soap, stderr);
    exit(-1);
  }
  fprintf(stderr, "Socket connection successful: master socket = %d\n", m);
  for ( ; ; )
  { s = soap_accept(&soap);
    fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);
    if (s < 0)
    { soap_print_fault(&soap, stderr);
      break;
    } 
    if (soap_ssl_accept(&soap))
    { soap_print_fault(&soap, stderr);
      break;
    }
    tsoap = soap_copy(&soap);
    if (!tsoap)
      break;
    pthread_create(&tid, NULL, &process_request, (void*)tsoap);
  }
  soap_end(&soap);
  soap_done(&soap);
  return 0;
} 

void *process_request(void *soap)
{ pthread_detach(pthread_self());
  soap_serve((struct soap*)soap);
  soap_end((struct soap*)soap);
  free(soap);
  return NULL;
}

void sigpipe_handle(int x) { }

int ns__add(struct soap *soap, double a, double b, double *result)
{ *result = a + b;
  return SOAP_OK;
} 
