#include "soapH.h"
#include "ssl.nsmap"
#include <pthread.h>	/* See CRYPTO_thread_setup/cleanup routines below */
#include <signal.h>	/* use sighandlers to catch interupts */

/* See OpenSSL /threads/th-lock.c file to support multi-threaded services with
   SSL on various systems (win32, Sun Solaris, Irix)
*/

void *process_request(void*);
void sigpipe_handle(int);
void CRYPTO_thread_setup();
void CRYPTO_thread_cleanup();

int main()
{ int m, s; /* master and slave sockets */
  pthread_t tid;
  struct soap soap, *tsoap;
  /* Need SIGPIPE handler on Unix/Linux systems to catch broken pipes: */
  signal(SIGPIPE, sigpipe_handle);
  CRYPTO_thread_setup();
  soap_init(&soap);
  if (soap_ssl_server_context(&soap,
    SOAP_SSL_DEFAULT,
    "server.pem",	/* keyfile: see SSL docs on how to obtain this file */
    "password",		/* password to read the key file */
    "cacert.pem",	/* cacert */
    NULL,		/* capath */
    "dh512.pem",	/* dhfile, if NULL use rsa */
    NULL		/* if randfile!=NULL: use a file with random data to seed randomness */ 
  ))
  { soap_print_fault(&soap, stderr);
    exit(1);
  }
  soap.accept_timeout = 60;	/* server times out after 10 minutes of inactivity */
  soap.recv_timeout = 30;	/* if read stalls, then timeout after 60 seconds */
  m = soap_bind(&soap, NULL, 18081, 100);
  if (m < 0)
  { soap_print_fault(&soap, stderr);
    exit(1);
  }
  fprintf(stderr, "Socket connection successful: master socket = %d\n", m);
  for (;;)
  { s = soap_accept(&soap);
    fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);
    if (s < 0)
    { if (soap.errnum)
        soap_print_fault(&soap, stderr);
      else
        fprintf(stderr, "Server timed out\n");
      break;
    }
    fprintf(stderr, "Socket %d connection from IP %d.%d.%d.%d\n", s, (int)(soap.ip>>24)&0xFF, (int)(soap.ip>>16)&0xFF, (int)(soap.ip>>8)&0xFF, (int)soap.ip&0xFF);
    tsoap = soap_copy(&soap);
    if (!tsoap)
    { soap_closesock(&soap);
      continue;
    }
    if (soap_ssl_accept(tsoap))
    { soap_print_fault(tsoap, stderr);
      fprintf(stderr, "SSL request failed, continue with next call...\n");
      soap_done(tsoap);
      free(tsoap);
      continue;
    }
    pthread_create(&tid, NULL, &process_request, (void*)tsoap);
  }
  CRYPTO_thread_cleanup();
  soap_done(&soap); /* MUST call after CRYPTO_thread_cleanup */
  return 0;
} 

void *process_request(void *soap)
{ pthread_detach(pthread_self());
  soap_serve((struct soap*)soap);
  soap_destroy((struct soap*)soap); /* for C++ */
  soap_end((struct soap*)soap);
  soap_done((struct soap*)soap);
  free(soap);
  return NULL;
}

/* OpenSSL Pthread code
   The code below uses Pthreads (Linux and other Unices).  Please take a look
   at the OpenSSL /threads/th-lock.c file on how to implement locks on other
   systems
*/

static pthread_mutex_t *lock_cs;
static long *lock_count;
void locking_function(int, int, const char*, int);
unsigned long id_function();

void CRYPTO_thread_setup()
{ int i;
  lock_cs = (pthread_mutex_t*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
  lock_count = (long*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
  for (i = 0; i < CRYPTO_num_locks(); i++)
  { lock_count[i]=0;
    pthread_mutex_init(&(lock_cs[i]), NULL);
  }
  CRYPTO_set_id_callback(id_function);
  CRYPTO_set_locking_callback(locking_function);
}

void CRYPTO_thread_cleanup()
{ int i;
  CRYPTO_set_locking_callback(NULL);
  for (i = 0; i < CRYPTO_num_locks(); i++)
    pthread_mutex_destroy(&(lock_cs[i]));
  OPENSSL_free(lock_cs);
  OPENSSL_free(lock_count);
}

/* OpenSSL requires two callbacks and a mutex for MT applications (see
   crypto/threads/mttest.c for examples) 
*/

void locking_function(int mode, int type, const char *file, int line)
{ if (mode & CRYPTO_LOCK)
  { pthread_mutex_lock(&(lock_cs[type]));
    lock_count[type]++;
  }
  else
    pthread_mutex_unlock(&(lock_cs[type]));
}

unsigned long id_function()
{ return (unsigned long)pthread_self();
}

/* end of Pthread code */

/* Catch Unix SIGPIPE */

void sigpipe_handle(int x) { }

/* Our service method */

int ns__add(struct soap *soap, double a, double b, double *result)
{ *result = a + b;
  return SOAP_OK;
} 
