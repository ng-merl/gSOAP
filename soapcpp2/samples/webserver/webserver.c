/*	webserver.c

	Example stand-alone gSOAP Web server using gSOAP HTTP GET plugin
	This is a small but fully functional (embedded) Web server that can
	serve static and dynamic pages and provide SOAP/XML responses.

	Copyright (C) 2000-2003 Robert A. van Engelen, Genivia inc.
	All Rights Reserved.

	The Web server handles HTTP GET requests to serve pages and HTTP POST
	reguests to handle SOAP/XML messages. This example only implements
	a simple calculator XML Web service for demonstration purposes.

	This application requires Zlib and Pthreads (you can replace Pthreads
	with another thread library, but you need to study the OpenSSL Pthread
	implementation when you plan to use OpenSSL).

	On Unix/Linux, please enable SIGPIPE handling, see main function below

	Compile without OpenSSL:
	soapcpp2 -c webserver.h
	Customize your COOKIE_DOMAIN in this file
	gcc -DWITH_COOKIES -DWITH_ZLIB -o webserver webserver.c httpget.c stdsoap2.c soapC.c soapClient.c soapServer.c -lpthread -lz

	Compile with OpenSSL:
	soapcpp2 -c webserver.h
	Customize your COOKIE_DOMAIN in this file
	gcc -DWITH_OPENSSL -DWITH_COOKIES -DWITH_ZLIB -o webserver webserver.c httpget.c stdsoap2.c soapC.c soapClient.c soapServer.c -lpthread -lz -lssl -lcrypto

	Use (HTTP GET):
	Compile the web server as explained above
	Start the web server on an even numbered port (e.g. 8080):
	> webserver 8080 &
	Start a web browser and open a (localhost) location:
	http://127.0.0.1:8080
	then type userid 'admin' and passwd 'guest'
	http://127.0.0.1:8080/calc.html
	then enter an expression
	http://127.0.0.1:8080/test.html
	https://127.0.0.1:8081/webserver.wsdl

	Use (HTTPS GET):
	Create the SSL certificate
	Compile the web server with OpenSSL as explained above
	Start the web server on an odd numbered port (e.g. 8081)
	> webserver 8081 &
	Actually, you can start two servers, one on 8080 and a secure one on
	8081
	Start a web browser and open a (localhost) location:
	https://127.0.0.1:8081
	then type userid 'admin' and passwd 'guest'
	https://127.0.0.1:8081/calc.html
	then enter an expression
	https://127.0.0.1:8081/test.html
	https://127.0.0.1:8081/webserver.wsdl

	Use (HTTP POST):
	Serves SOAP/XML calculation requests

*/

#include "soapH.h"
#include "webserver.nsmap"
#include "httpget.h"
#include <pthread.h>

#define BACKLOG (100)
#define SERVER_TIMEOUT (24*60*60) /* die after 24 hrs waiting */
#define IO_TIMEOUT (60) /* deny serving slow clients */
#define COOKIE_DOMAIN "localhost:8080" /* must be your current domain for cookies to work */

#define AUTH_USERID "admin"
#define AUTH_PASSWD "guest"

static time_t start = 0;
static int secure = 0;	/* =0: no SSL, =1: SSL */

void *process_request(void*);	/* multi-threaded request handler */

int http_get_handler(struct soap*);	/* HTTP get handler */
int check_authentication(struct soap*);	/* HTTP authentication check */
int copy_file(struct soap*, const char*, const char*);	/* copy file as HTTP response */
int calc(struct soap*);
int info(struct soap*);
int html_hbar(struct soap*, const char*, size_t, unsigned long);

void CRYPTO_thread_setup();
void CRYPTO_thread_cleanup();

/******************************************************************************\
 *
 *	Main
 *
\******************************************************************************/

int main(int argc, char **argv)
{ struct soap soap, *tsoap;
  pthread_t tid;
  int port = 80;
  int m, s, i;
  if (argc >= 2)
    port = atoi(argv[1]);
  start = time(NULL);
  fprintf(stderr, "Starting Web server on port %d\n", port);
  soap_init2(&soap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);
  /* HTTP cookies (to enable: compile all sources with -DWITH_COOKIES) */
  soap.cookie_domain = COOKIE_DOMAIN; /* must be the current host name */
  soap.cookie_path = "/"; /* the path which is used to filter/set cookies with this destination */
  /* SSL init (to enable: compile all sources with -DWITH_OPENSSL) */
#ifdef WITH_OPENSSL
  CRYPTO_thread_setup();
  /* if the port is an odd number, the Web server uses HTTPS only */
  if (port % 2)
    secure = 1;
  if (secure)
  { soap.keyfile = "server.pem";/* see SSL docs on how to obtain this file */
    soap.password = "password";	/* your password */
    soap.cafile = "cacert.pem";	/* see SSL docs on how to obtain this file */
    soap.dhfile = "dh512.pem";	/* see SSL docs on how to obtain this file */
    soap.randfile = NULL;	/* if randfile!=NULL: use a file with random data to seed randomness */ 
  }
#endif
  /* Register HTTP GET plugin */
  soap_register_plugin_arg(&soap, http_get, (void*)http_get_handler);
  /* Unix SIGPIPE, this is OS dependent */
  soap.accept_flags = SO_NOSIGPIPE; 	/* some systems like this */
  /* soap.socket_flags = MSG_NOSIGNAL; */	/* others need this */
  m = soap_bind(&soap, NULL, port, BACKLOG);
  if (m < 0)
  { soap_print_fault(&soap, stderr);
    exit(1);
  }
  fprintf(stderr, "Port bind successful: master socket = %d\n", m);
  soap.accept_timeout = SERVER_TIMEOUT;
  soap.send_timeout = IO_TIMEOUT;
  soap.recv_timeout = IO_TIMEOUT;
  for (i = 1; ; i++)
  { s = soap_accept(&soap);
    if (s < 0)
    { if (soap.errnum)
      { soap_print_fault(&soap, stderr);
        exit(1);
      }
      fprintf(stderr, "gSOAP Web server timed out\n");
      break;
    }
    fprintf(stderr, "Thread %d accepts socket %d connection from IP %d.%d.%d.%d\n", i, s, (int)(soap.ip>>24)&0xFF, (int)(soap.ip>>16)&0xFF, (int)(soap.ip>>8)&0xFF, (int)soap.ip&0xFF);
    tsoap = soap_copy(&soap);
#ifdef WITH_OPENSSL
    if (secure && soap_ssl_accept(tsoap))
    { soap_print_fault(tsoap, stderr);
      fprintf(stderr, "SSL request failed, continue with next call...\n");
      soap_end(tsoap);
      soap_done(tsoap);
      free(tsoap);
      continue;
    }
#endif
    pthread_create(&tid, NULL, (void*(*)(void*))process_request, (void*)tsoap);
  }
#ifdef WITH_OPENSSL
  CRYPTO_thread_cleanup();
#endif
  soap_done(&soap);
  return 0;
}

/******************************************************************************\
 *
 *	Process dispatcher
 *
\******************************************************************************/

void *process_request(void *soap)
{ pthread_detach(pthread_self());
  soap_serve((struct soap*)soap);
  /* soap_destroy((struct soap*)soap); */ /* cleanup class instances (but this is a C app) */
  soap_end((struct soap*)soap);
  soap_done((struct soap*)soap);
  free(soap);
  return NULL;
}

/******************************************************************************\
 *
 *	SOAP/XML handling
 *
\******************************************************************************/

int ns__add(struct soap *soap, int a, int b, int *c)
{ *c = a + b;
  return SOAP_OK;
}

int ns__sub(struct soap *soap, int a, int b, int *c)
{ *c = a - b;
  return SOAP_OK;
}

int ns__mul(struct soap *soap, int a, int b, int *c)
{ *c = a * b;
  return SOAP_OK;
}

int ns__div(struct soap *soap, int a, int b, int *c)
{ *c = a / b;
  return SOAP_OK;
}

/******************************************************************************\
 *
 *	Server dummy methods to avoid link errors
 *
\******************************************************************************/

int ns__addResponse_(struct soap *soap, int a)
{ return SOAP_NO_METHOD; /* we don't use this: we use soap_send_ns__addResponse instead */
}

int ns__subResponse_(struct soap *soap, int a)
{ return SOAP_NO_METHOD; /* we don't use this: we use soap_send_ns__subResponse instead */
}

int ns__mulResponse_(struct soap *soap, int a)
{ return SOAP_NO_METHOD; /* we don't use this: we use soap_send_ns__mulResponse instead */
}

int ns__divResponse_(struct soap *soap, int a)
{ return SOAP_NO_METHOD; /* we don't use this: we use soap_send_ns__divResponse instead */
}

/******************************************************************************\
 *
 *	HTTP GET handler for plugin
 *
\******************************************************************************/

int http_get_handler(struct soap *soap)
{ /* HTTP response choices: */
  /* soap_omode(soap, SOAP_IO_STORE); */ /* you have to buffer entire content when returning HTML pages to determine content length */
  soap_set_omode(soap, SOAP_IO_CHUNK); /* ... or use chunked HTTP content (faster) */
  if (soap->zlib_out == SOAP_ZLIB_GZIP) /* client accepts gzip */
    soap_set_omode(soap, SOAP_ENC_ZLIB); /* so we can compress content (gzip) */
  soap->z_level = 9; /* best compression */
  /* Use soap->path (from request URL) to determine request: */
  fprintf(stderr, "Request: %s\n", soap->endpoint);
  /* Note: soap->path starts with '/' */
  if (strchr(soap->path + 1, '/') || strchr(soap->path + 1, '\\'))	/* we don't like snooping in dirs */
    return 403; /* HTTP forbidden */
  if (!soap_tag_cmp(soap->path, "*.html"))
    return copy_file(soap, soap->path + 1, "text/html");
  if (!soap_tag_cmp(soap->path, "*.xml")
   || !soap_tag_cmp(soap->path, "*.xsd")
   || !soap_tag_cmp(soap->path, "*.wsdl"))
    return copy_file(soap, soap->path + 1, "text/xml");
  if (!soap_tag_cmp(soap->path, "*.jpg"))
    return copy_file(soap, soap->path + 1, "image/jpeg");
  if (!soap_tag_cmp(soap->path, "*.gif"))
    return copy_file(soap, soap->path + 1, "image/gif");
  if (!soap_tag_cmp(soap->path, "*.ico"))
    return copy_file(soap, soap->path + 1, "image/ico");
  if (!strncmp(soap->path, "/calc?", 6))
    return calc(soap);
  /* Check requestor's authentication: */
  if (check_authentication(soap))
    return 401;
  /* Return Web server status */
  if (!strcmp(soap->path, "/"))
    return info(soap);
  return 404; /* HTTP not found */
}

int check_authentication(struct soap *soap)
{ if (!soap->userid
   || !soap->passwd
   || strcmp(soap->userid, AUTH_USERID)
   || strcmp(soap->passwd, AUTH_PASSWD))
    return 401; /* HTTP not authorized error */
  return SOAP_OK;
}

/******************************************************************************\
 *
 *	Copy static page
 *
\******************************************************************************/

int copy_file(struct soap *soap, const char *name, const char *type)
{ FILE *fd;
  size_t r;
  fd = fopen(name, "r"); /* open file to copy */
  if (!fd)
    return SOAP_EOF; /* return HTTP error? */
  soap->http_content = type;
  if (soap_begin_send(soap)
   || soap_response(soap, SOAP_FILE)) /* OK HTTP response header */
    return soap->error;
  for (;;)
  { r = fread(soap->tmpbuf, 1, sizeof(soap->tmpbuf), fd);
    if (!r)
      break;
    if (soap_send_raw(soap, soap->tmpbuf, r))
    { soap_end_send(soap);
      fclose(fd);
      return soap->error;
    }
  }
  fclose(fd);
  if (soap_end_send(soap))
    return soap->error;
  return SOAP_OK;
}

/******************************************************************************\
 *
 *	Dynamic HTTP GET form-based calculator
 *
\******************************************************************************/

int calc(struct soap *soap)
{ int o = 0, a = 0, b = 0;
  char *s = query(soap); /* get arguments from query string */
  while (s)
  { char *key = query_key(soap, &s); /* decode next query string key */
    char *val = query_val(soap, &s); /* decode next query string value */
    if (key && val)
    { if (!strcmp(key, "o"))
        o = val[0];
      else if (!strcmp(key, "a"))
        a = strtol(val, NULL, 10);
      else if (!strcmp(key, "b"))
        b = strtol(val, NULL, 10);
    }
  }
  if (soap_begin_send(soap)
   || soap_response(soap, SOAP_OK))
    return soap->error;
  switch (o)
  { case 'a':
      if (soap_send_ns__addResponse_(soap, NULL, NULL, a + b))
        return soap->error;
      break;
    case 's':
      if (soap_send_ns__subResponse_(soap, NULL, NULL, a - b))
        return soap->error;
      break;
    case 'm':
      if (soap_send_ns__mulResponse_(soap, NULL, NULL, a * b))
        return soap->error;
      break;
    case 'd':
      if (soap_send_ns__divResponse_(soap, NULL, NULL, a / b))
        return soap->error;
      break;
    default:
      soap_sender_fault(soap, "Unknown operation", NULL);
      soap_send_fault(soap);
  }
  return SOAP_OK;
}

/******************************************************************************\
 *
 *	Dynamic Web server info page
 *
\******************************************************************************/

int info(struct soap *soap)
{ struct http_get_data *data;
  const char *t1, *t2, *t3, *t4, *t5, *t6;
  char *s;
  int r;
#ifdef WITH_COOKIES
  t1 = "<td align='center' bgcolor=green>YES</td>";
  if (soap_cookie_value(soap, "visit", NULL, NULL))
    t2 = "<td align='center' bgcolor=green>PASS</td>";
  else
    t2 = "<td align='center' bgcolor=yellow>WAIT</td>";
#else
  t1 = "<td align='center' bgcolor=red>NO</td>";
  t2 = "<td align='center' bgcolor=blue>N/A</td>";
#endif
  if (secure)
  { t3 = "<td align='center' bgcolor=green>YES</td>";
    if (soap->imode & SOAP_ENC_SSL)
      t4 = "<td align='center' bgcolor=green>PASS</td>";
    else
      t4 = "<td align='center' bgcolor=red><blink>FAIL</blink></td>";
  }
  else
  { t3 = "<td align='center' bgcolor=red>NO</td>";
    t4 = "<td align='center' bgcolor=blue>N/A</td>";
  }
#ifdef WITH_ZLIB
  t5 = "<td align='center' bgcolor=green>YES</td>";
  if (soap->omode & SOAP_ENC_ZLIB)
    t6 = "<td align='center' bgcolor=green>PASS</td>";
  else
    t6 = "<td align='center' bgcolor=yellow>WAIT</td>";
#else
  t5 = "<td align='center' bgcolor=red>NO</td>";
  t6 = "<td align='center' bgcolor=blue>N/A</td>";
#endif
  soap_set_cookie(soap, "visit", "true", NULL, NULL);
  soap_set_cookie_expire(soap, "visit", 600, NULL, NULL);
  if (soap_begin_send(soap)
   || soap_response(soap, SOAP_HTML))
    return soap->error;
  asprintf(&s, "\
<html>\
<head>\
<meta name='Author' content='Robert A. van Engelen'>\
<meta name='Generator' content='gSOAP'>\
<meta http-equiv='Refresh' content='60'>\
<style type='text/css' media='screen'><!-- td { color: white; font-size: 10px; line-height: 10px; font-family: Arial, Helve tica, Geneva, Swiss, SunSans-Regular } --></style>\
<title>gSOAP Web Server Administration</title>\
</head>\
<body bgcolor='FFFFFF'>\
<h1>gSOAP Web Server Administration Page</h1>\
<p>Server endpoint: %s\
<p>Client agent IP: %d.%d.%d.%d\
<p>Time elapsed since start: %ld seconds\
", soap->endpoint, (int)(soap->ip>>24)&0xFF, (int)(soap->ip>>16)&0xFF, (int)(soap->ip>>8)&0xFF, (int)soap->ip&0xFF, time(NULL)-start);
  r = soap_send(soap, s);
  free(s);
  if (r)
    return r;
  asprintf(&s, "\
<h2>Function Tests</h2>\
<table border='0' cellspacing='0' cellpadding='0' bgcolor='#666666' nosave>\
<tr height='10'><td height='10'></td><td height='10'><i>Function</i></td><td align='center' height='10'><i>Result</i></td><td height='10' background='obls.gif'></td></tr>\
<tr><td></td><td>HTTP operational</td><td align='center' bgcolor=green>YES</td><td width='10' background='ls.gif'></td></tr>\
<tr><td></td><td>HTTP cookies enabled</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td></td><td>HTTP cookies test</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td></td><td>HTTPS (OpenSSL) enabled</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td></td><td>HTTPS (OpenSSL) test</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td></td><td>HTTP compression enabled</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td></td><td>HTTP compression test</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr height='10'><td width='10' height='10' background=otrs.gif></td><td height='10' background='ts.gif'></td><td height='10' background='ts.gif'></td><td width='10' height='10' background='otls.gif'></td></tr>\
</table>", t1, t2, t3, t4, t5, t6);
  r = soap_send(soap, s);
  free(s);
  if (r)
    return r;
  data = (struct http_get_data*)soap_lookup_plugin(soap, http_get_id);
  if (data)
  { soap_send(soap, "<h2>Usage Statistics</h2>");
    html_hbar(soap, "HTTP GET", data->stat_get, 0x0000FF);
    html_hbar(soap, "HTTP POST", data->stat_post, 0x00FF00);
    html_hbar(soap, "HTTP FAIL", data->stat_fail, 0xFF0000);
  }
  soap_send(soap, "\
<p>This page will automatically reload every minute to refresh the statistics.\
<br><br><br>Powered by gSOAP\
</body>\
</HTML>");
  return soap_end_send(soap);
}

int html_hbar(struct soap *soap, const char *title, size_t len, unsigned long color)
{ const char *scale;
  char *s;
  int r;
  if (len > 1000000)
  { len /= 1000000;
    scale = "&#183;10<sup>6</sup>";
  }
  else if (len > 1000)
  { len /= 1000;
    scale = "&#183;10<sup>3</sup>";
  }
  else
    scale = "";
  asprintf(&s, "\
<table border='0' cellspacing='0' cellpadding='0' height='30'>\
<tr height='10'>\
<td width='10' height='10' background='bl.gif'></td>\
<td rowspan='2' bgcolor='#666666' height='20'>%s %d%s</td>\
<td bgcolor='#%.6x' width='%hu' height='10'></td>\
<td bgcolor='#%.6x' width='%hu' height='10'></td>\
<td width='10' height='10' background='obls.gif'></td>\
</tr>\
<tr height='10'>\
<td width='10' height='10' background='bl.gif'></td>\
<td colspan='2' height='10' background='ruler.gif'></td>\
<td width='10' height='10' background='ls.gif'></td>\
</tr>\
<tr height='10'>\
<td width='10' height='10' background='otrs.gif'></td>\
<td colspan='3' height='10' background='ts.gif'></td>\
<td width='10' height='10' background='otls.gif'></td>\
</tr>\
</table>", title, len, scale, color, len, color, len);
  r = soap_send(soap, s);
  free(s);
  return r;
}

/******************************************************************************\
 *
 *	OpenSSL
 *
\******************************************************************************/

#ifdef WITH_OPENSSL

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

#endif
