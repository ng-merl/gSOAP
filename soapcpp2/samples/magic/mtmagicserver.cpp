//	mtmagicserver.cpp
//	Multi-threaded magic squares server
//	Runs as CGI (not multi-threaded) or multi-threaded standalone
//	Computation of the magic square has been deliberately slowed down to
//	demonstrate work load issues (see SLEEP constant)

// Run from the command line with arguments IP (which must be the
// IP of the current machine you are using) and PORT to run this as a
// multi-threaded stand-alone server on a port. For example:
// > mtmagicserver.cgi 18081
// To let 'magic' client talk to this service, change the URL in code magic.cpp
// into "localhost:18081"

// This example illustrates two alternative server implementations with threads.
// The first implementation recycles gSOAP resources but is bounded to a maximum
// number of threads. Each thread needs to be joined, so runaway processes will
// halt the server at some point.
// The second implementation has no thread limitation. Runaway threads are not
// controlled.

#include "soapH.h"
#include <unistd.h>	// import sleep()
#include <pthread.h>

#define BACKLOG (100)	// Max. request backlog
#define MAX_THR (8)	// Max. threads to serve requests
#define SLEEP	(1)	// make each thread sleep to mimic work load latency

////////////////////////////////////////////////////////////////////////////////
//
//	Multi-Threaded Magic Squares Server
//
////////////////////////////////////////////////////////////////////////////////

void *process_request(void*);

int main(int argc, char **argv)
{ struct soap soap;
  soap_init(&soap);
  if (argc < 2)		// no args: assume this is a CGI application
  { soap_serve(&soap);	// serve request
    soap_destroy(&soap);// cleanup class instances
    soap_end(&soap);	// cleanup
  }
  else
  { pthread_t tid;
    int port;
    int m, s, i;
    port = atoi(argv[1]);
    m = soap_bind(&soap, NULL, port, BACKLOG);
    if (m < 0)
      exit(-1);
    fprintf(stderr, "Socket connection successful %d\n", m);
    for (i = 0; ; i++)
    { s = soap_accept(&soap);
      if (s < 0)
        break;
      fprintf(stderr, "Thread %d accepts socket %d connection from IP %d.%d.%d.%d\n", i, s, (int)(soap.ip>>24)&0xFF, (int)(soap.ip>>16)&0xFF, (int)(soap.ip>>8)&0xFF, (int)soap.ip&0xFF);
      pthread_create(&tid, NULL, (void*(*)(void*))process_request, (void*)soap_copy(&soap));
    }
  }
  soap_done(&soap);
  return 0;
}

void *process_request(void *soap)
{ pthread_detach(pthread_self());
  soap_serve((struct soap*)soap);
  soap_destroy((struct soap*)soap);
  soap_end((struct soap*)soap);
  soap_done((struct soap*)soap);
  free(soap);
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//
//	Magic Square Algorithm
//
////////////////////////////////////////////////////////////////////////////////

int ns1__magic(struct soap *soap, int n, matrix *square)
{ int i, j, k, l, key = 2;
  if (n < 1)
    return soap_receiver_fault(soap, "Negative or zero size", "The input parameter must be positive");
  if (n > 100)
    return soap_receiver_fault(soap, "size > 100", "The input parameter must not be too large");
  square->resize(n, n);
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      (*square)[i][j] = 0;
  i = 0;
  j = (n-1)/2;
  (*square)[i][j] = 1;
  while (key <= n*n)
  { if (i-1 < 0)
      k = n-1;
    else
      k = i-1;
    if (j-1 < 0)
      l = n-1;
    else
      l = j-1;
    if ((*square)[k][l])
      i = (i+1) % n;
    else
    { i = k;
      j = l;
    }
    (*square)[i][j] = key;
    key++;
  }
  sleep(SLEEP);		// mimic work load latency
  return SOAP_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
//	Class vector Methods
//
////////////////////////////////////////////////////////////////////////////////

vector::vector()
{ __ptr = 0;
  __size = 0;
}

vector::vector(int n)
{ __ptr = (int*)soap_malloc(soap, n*sizeof(int));
  __size = n;
}

vector::~vector()
{ soap_unlink(soap, this); // not required, but just to make sure if someone calls delete on this
}

void vector::resize(int n)
{ int *p;
  if (__size == n)
    return;
  p = (int*)soap_malloc(soap, n*sizeof(int));
  if (__ptr)
  { for (int i = 0; i < (n <= __size ? n : __size); i++)
      p[i] = __ptr[i];
    soap_unlink(soap, __ptr);
    free(__ptr);
  }
  __size = n;
  __ptr = p;
}

int& vector::operator[](int i)
{ if (!__ptr || i < 0 || i >= __size)
    fprintf(stderr, "Array index out of bounds\n");
  return __ptr[i];
}

////////////////////////////////////////////////////////////////////////////////
//
//	Class matrix Methods
//
////////////////////////////////////////////////////////////////////////////////

matrix::matrix()
{ __ptr = 0;
  __size = 0;
}

matrix::matrix(int rows, int cols)
{ __ptr = soap_new_vector(soap, rows);
  for (int i = 0; i < cols; i++)
    __ptr[i].resize(cols);
  __size = rows;
}

matrix::~matrix()
{ soap_unlink(soap, this); // not required, but just to make sure if someone calls delete on this
}

void matrix::resize(int rows, int cols)
{ int i;
  vector *p;
  if (__size != rows)
  { if (__ptr)
    { p = soap_new_vector(soap, rows);
      for (i = 0; i < (rows <= __size ? rows : __size); i++)
      { if (this[i].__size != cols)
          (*this)[i].resize(cols);
	(p+i)->__ptr = __ptr[i].__ptr;
	(p+i)->__size = cols;
      }
      for (; i < rows; i++)
        __ptr[i].resize(cols);
    }
    else
    { __ptr = soap_new_vector(soap, rows);
      for (i = 0; i < rows; i++)
        __ptr[i].resize(cols);
      __size = rows;
    }
  }
  else
    for (i = 0; i < __size; i++)
      __ptr[i].resize(cols);
}

vector& matrix::operator[](int i)
{ if (!__ptr || i < 0 || i >= __size)
    fprintf(stderr, "Array index out of bounds\n");
  return __ptr[i];
}

////////////////////////////////////////////////////////////////////////////////
//
//	Namespace Definition Table
//
////////////////////////////////////////////////////////////////////////////////

struct Namespace namespaces[] =
{ { "SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/" }, // must be first
  { "SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/" }, // must be second
  { "xsi", "http://www.w3.org/1999/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance" },
  { "xsd", "http://www.w3.org/1999/XMLSchema",          "http://www.w3.org/*/XMLSchema" },
  { "ns1", "urn:MagicSquare" },		// "ns1" namespace prefix
  { NULL, NULL }
};
