/*	handler.c

	Multi-threaded stand-alone event handler service

	Events are based on asynchronous one-way SOAP messaging using HTTP
	keep-alive for persistent connections

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Compile:
	soapcpp2 -c event.h
	cc -o handler handler.c stdsoap2.c soapC.c soapService.c

	Run:
	handler 18000 &

	Server will time out after 24hr of inactivity

	This code enables keep-alive support which can cause "broken pipes"
	when the client prematurely closes the connection while indicating
	it wants the connection to stay alive. Broken pipes (SIGPIPE) can be
	fixed using MSG_NOSIGNAL, SO_NOSIGPIPE, or with a signal handler,
	but this is not very portable (see code below)

*/

#include "soapH.h"
#include "Event.nsmap"
#include <pthread.h>

#define BACKLOG (100)
#define TIMEOUT (24*60*60) /* timeout after 24hrs of inactivity */

void *process_request(void*);

int main(int argc, char **argv)
{ struct soap soap, *tsoap;
  pthread_t tid;
  int port;
  int m, s, i;
  if (argc < 2)
  { fprintf(stderr, "Usage: handler <port>\n");
    exit(1);
  }
  port = atoi(argv[1]);
  soap_init2(&soap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);	/* keep I/O alive */
  soap.accept_timeout = TIMEOUT;
  soap.bind_flags |= SO_REUSEADDR;	/* don't use this in unsecured environments */
  /* soap.socket_flags = MSG_NOSIGNAL; */	/* use this to disable SIGPIPE */
  /* soap.bind_flags |= SO_NOSIGPIPE; */	/* or use this to disable SIGPIPE */
  m = soap_bind(&soap, NULL, port, BACKLOG);
  if (m < 0)
  { soap_print_fault(&soap, stderr);
    exit(1);
  }
  fprintf(stderr, "Socket connection successful %d\n", m);
  for (i = 1; ; i++)
  { s = soap_accept(&soap);
    if (s < 0)
    { if (soap.errnum)
        soap_print_fault(&soap, stderr);
      else
        fprintf(stderr, "%s timed out\n", argv[0]);	/* should really wait for threads to terminate, but 24hr timeout should be enough ... */
      break;
    }
    fprintf(stderr, "Thread %d accepts socket %d connection from IP %d.%d.%d.%d\n", i, s, (int)(soap.ip>>24)&0xFF, (int)(soap.ip>>16)&0xFF, (int)(soap.ip>>8)&0xFF, (int)soap.ip&0xFF);
    tsoap = soap_copy(&soap);
    pthread_create(&tid, NULL, (void*(*)(void*))process_request, (void*)tsoap);
  }
  return 0;
}

void *process_request(void *soap)
{ struct soap *tsoap = (struct soap*)soap;
  pthread_detach(pthread_self());
  soap_serve(tsoap);
  soap_destroy(tsoap);
  soap_end(tsoap);
  soap_done(tsoap);
  free(tsoap);
  return NULL;
}

int ns__handle(struct soap *soap, enum ns__event event)
{ switch (event)
  { case EVENT_A: fprintf(stderr, "Event: A\n"); break;
    case EVENT_B: fprintf(stderr, "Event: B\n"); break;
    case EVENT_C: fprintf(stderr, "Event: C\n"); break;
  }
  return SOAP_OK;
}
