#include "soapH.h"
#include "ssl.nsmap"
#include <pthread.h>	/* See CRYPTO_thread_setup/cleanup routines below */
#include <signal.h>	/* use sighandlers to catch interupts */

void *process_request(void*);

static pthread_mutex_t *lock_cs;
static long *lock_count;
void locking_function(int, int, const char*, int);
unsigned long id_function();

void sigpipe_handle(int);
void CRYPTO_thread_setup();
void CRYPTO_thread_cleanup();

int main()
{ int m, s; /* master and slave sockets */
  pthread_t tid;
  struct soap soap, *tsoap;
  /* Need SIGPIPE handler on Unix/Linux systems: */
  signal(SIGPIPE, sigpipe_handle);
  CRYPTO_thread_setup();
  soap_init(&soap);
  soap.keyfile = "server.pem";	/* see SSL docs on how to obtain this file */
  soap.password = "password";	/* password to read the key file */
  soap.cafile = "cacert.pem";	/* see SSL docs on how to obtain this file */
  soap.dhfile = "dh512.pem";	/* see SSL docs on how to obtain this file */
  soap.randfile = NULL;		/* if randfile!=NULL: use a file with random data to seed randomness */ 
  soap.accept_timeout = 600;	/* server times out after 10 minutes of inactivity */
  soap.recv_timeout = 30;	/* if read stalls, then timeout after 30 seconds */
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
    { soap_print_fault(&soap, stderr);
      break;
    } 
    if (soap_ssl_accept(&soap))
    { soap_print_fault(&soap, stderr);
      fprintf(stderr, "Continue with next call...\n");
      continue;
    }
    tsoap = soap_copy(&soap);
    if (!tsoap)
      break;
    pthread_create(&tid, NULL, &process_request, (void*)tsoap);
  }
  soap_done(&soap);
  CRYPTO_thread_cleanup();
  return 0;
} 

void *process_request(void *soap)
{ pthread_detach(pthread_self());
  soap_serve((struct soap*)soap);
  soap_end((struct soap*)soap);
  soap_done((struct soap*)soap);
  free(soap);
  return NULL;
}

/* OpenSSL Pthread code
   The code below uses Pthreads (Linux and other Unices).  Please use the
   OpenSSL /threads/th-lock.c file to implement locks on other systems
*/
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

void sigpipe_handle(int x) { }

int ns__add(struct soap *soap, double a, double b, double *result)
{ *result = a + b;
  return SOAP_OK;
} 
