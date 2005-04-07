/*

webserver.c

Example stand-alone gSOAP Web server based on the gSOAP HTTP GET plugin.
This is a small but fully functional (embedded) Web server for serving
static and dynamic pages and SOAP/XML responses.

--------------------------------------------------------------------------------
gSOAP XML Web services tools
Copyright (C) 2001-2004, Robert van Engelen, Genivia, Inc. All Rights Reserved.
This software is released under one of the following two licenses:
GPL or Genivia's license for commercial use.
--------------------------------------------------------------------------------
GPL license.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org
--------------------------------------------------------------------------------
A commercial use license is available from Genivia, Inc., contact@genivia.com
--------------------------------------------------------------------------------

	The Web server handles HTTP GET requests to serve pages and HTTP POST
	reguests to handle SOAP/XML messages. This example only implements
	a simple calculator XML Web service for demonstration purposes (the
	service responds with SOAP/XML).

	This application requires Zlib and Pthreads (you can replace Pthreads
	with another thread library, but you need to study the OpenSSL thread
	changes in the OpenSSL documentation).

	On Unix/Linux, please enable SIGPIPE handling, see main function below.
	SIGPIPE handling will avoid your server from termination when sockets
	are disconnected by clients before the transaction was completed
	(aka broken pipe).

	Compile without OpenSSL:
	soapcpp2 -c -n -popt opt.h
	soapcpp2 -c webserver.h
	Customize your COOKIE_DOMAIN in this file
	gcc -DWITH_COOKIES -DWITH_ZLIB -o webserver webserver.c options.c httpget.c logging.c stdsoap2.c soapC.c soapClient.c soapServer.c -lpthread -lz

	Compile with OpenSSL:
	soapcpp2 -c -n -popt opt.h
	soapcpp2 -c webserver.h
	Customize your COOKIE_DOMAIN in this file
	gcc -DWITH_OPENSSL -DWITH_COOKIES -DWITH_ZLIB -o webserver webserver.c options.c httpget.c logging.c stdsoap2.c soapC.c soapClient.c soapServer.c -lpthread -lz -lssl -lcrypto

	Use (HTTP GET):
	Compile the web server as explained above
	Start the web server on an even numbered port (e.g. 8080):
	> webserver 8080 &
	Start a web browser and open a (localhost) location:
	http://127.0.0.1:8080
	and type userid 'admin' and passwd 'guest' to gain access
	Open the location:
	http://127.0.0.1:8080/calc.html
	then enter an expression
	Open the locations:
	http://127.0.0.1:8080/test.html
	http://127.0.0.1:8081/webserver.wsdl

	Use (HTTPS GET):
	Create the SSL certificate
	Compile the web server with OpenSSL as explained above
	Start the web server on an odd numbered port (e.g. 8081)
	> webserver 8081 &
	Actually, you can start two servers, one on 8080 and a secure one on
	8081
	Start a web browser and open a (localhost) location:
	https://127.0.0.1:8081
	and type userid 'admin' and passwd 'guest' to gain access
	Open the location:
	https://127.0.0.1:8081/calc.html
	and enter an expression
	Open the locations:
	https://127.0.0.1:8081/test.html
	https://127.0.0.1:8081/webserver.wsdl

	Use (HTTP POST):
	Serves SOAP/XML calculation requests

	Command-line options:
	-z		enables compression
	-c		enables chunking
	-k		enables keep-alive
	-i		enables non-threaded iterative server
	-v		enables verbose mode
	-t<val>		sets I/O timeout value (seconds)
	-s<val>		sets server timeout value (seconds)
	-d<host>	sets cookie domain
	-p<path>	sets cookie path
	-l[none inbound outbound both]
			enables logging

	Requires options.h and options.c for command line option parsing and
	for parsing interactive Web page options settings. The
	default_options[] array defines application options, short-hands,
	selection lists, and default values. See options.h for more details.
*/

#include "soapH.h"
#include "webserver.nsmap"
#include "options.h"
#include "httpget.h"
#include "logging.h"
#include <unistd.h>		/* defines _POSIX_THREADS if pthreads are available */
#ifdef _POSIX_THREADS
# include <pthread.h>
#endif
#include <signal.h>		/* defines SIGPIPE */

#define BACKLOG (100)

#define AUTH_USERID "admin"
#define AUTH_PASSWD "guest"

/******************************************************************************\
 *
 *	Program options
 *
\******************************************************************************/

static const struct option default_options[] =
{ { "z.compress", NULL, },
  { "c.chunking", NULL, },
  { "k.keepalive", NULL, },
  { "i.iterative", NULL, },
  { "v.verbose", NULL, },
  { "t.ioTimeout", "seconds", 6, "120"},
  { "s.serverTimeout", "seconds", 6, "3600"},
  { "d.cookieDomain", "host", 20, "localhost:8080"},
  { "p.cookiePath", "path", 20, "/"},
  { "l.logging", "none inbound outbound both", },
  { "", "port", },		/* rest of command line args */
  { NULL },			/* must be NULL terminated */
};

/* The numbering of these defines must correspond to the option sequence */
#define OPTION_z	0
#define OPTION_c	1
#define OPTION_k	2
#define OPTION_i	3
#define OPTION_v	4
#define OPTION_t	5
#define OPTION_s	6
#define OPTION_d	7
#define OPTION_p	8
#define OPTION_l	9
#define OPTION_port	10

/******************************************************************************\
 *
 *	Static
 *
\******************************************************************************/

static struct option *options = NULL;
static time_t start;
static int secure = 0;		/* =0: no SSL, =1: support SSL */

static const char *minutes[60] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const char *hours[24] = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23"};

/******************************************************************************\
 *
 *	Forward decls
 *
\******************************************************************************/

void *process_request(void*);	/* multi-threaded request handler */
int http_get_handler(struct soap*);	/* HTTP get handler */
int check_authentication(struct soap*);	/* HTTP authentication check */
int copy_file(struct soap*, const char*, const char*);	/* copy file as HTTP response */
int calc(struct soap*);
int calcpost(struct soap*);
int info(struct soap*);
int html_hbar(struct soap*, const char*, size_t, size_t, unsigned long);
int html_hist(struct soap*, const char*, size_t, size_t, size_t, const char**, size_t*, size_t);
void sigpipe_handle(int); /* SIGPIPE handler: Unix/Linux only */

/******************************************************************************\
 *
 *	OpenSSL
 *
\******************************************************************************/

int CRYPTO_thread_setup();
void CRYPTO_thread_cleanup();

/******************************************************************************\
 *
 *	Main
 *
\******************************************************************************/

int main(int argc, char **argv)
{ struct soap soap, *tsoap;
  struct logging_data *logdata;
  pthread_t tid;
  int port = 0;
  int m, s, i;
  options = copy_options(default_options); /* must copy, so option values can be modified */
  if (parse_options(argc, argv, options))
    exit(0);
  if (options[OPTION_port].value)
    port = atol(options[OPTION_port].value);
  if (!port)
    port = 8080;
  start = time(NULL);
  fprintf(stderr, "Starting Web server on port %d\n", port);
  if (port != 8080)
    fprintf(stderr, "[Note: use port 8080 to test server from browser with test.html and calc.html]\n");
  fprintf(stderr, "[Note: you must enable Linux/Unix SIGPIPE handler to avoid broken pipe]\n");
  soap_init2(&soap, SOAP_IO_KEEPALIVE, SOAP_IO_DEFAULT);
  /* HTTP cookies (to enable: compile all sources with -DWITH_COOKIES) */
  soap.cookie_domain = options[OPTION_d].value; /* must be the current host name */
  soap.cookie_path = options[OPTION_p].value; /* the path which is used to filter/set cookies with this destination */
  /* SSL init (to enable: compile all sources with -DWITH_OPENSSL) */
#ifdef WITH_OPENSSL
  if (CRYPTO_thread_setup())
  { fprintf(stderr, "Cannot setup thread mutex\n");
    exit(1);
  }
  /* if the port is an odd number, the Web server uses HTTPS only */
  if (port % 2)
    secure = 1;
  if (secure && soap_ssl_server_context(&soap,
    SOAP_SSL_DEFAULT,
    "server.pem",	/* keyfile: see SSL docs on how to obtain this file */
    "password",		/* password to read the key file */
    NULL, 		/* cacert */
    NULL,		/* capath */
    "dh512.pem",	/* DH file, if NULL use RSA */
    NULL,		/* if randfile!=NULL: use a file with random data to seed randomness */ 
    "webserver"		/* server identification for SSL session cache (must be a unique name) */
  ))
  { soap_print_fault(&soap, stderr);
    exit(1);
  }
#endif
  /* Register HTTP GET plugin */
  if (soap_register_plugin_arg(&soap, http_get, (void*)http_get_handler))
    soap_print_fault(&soap, stderr);
  /* Register logging plugin */
  if (soap_register_plugin(&soap, logging))
    soap_print_fault(&soap, stderr);
  logdata = (struct logging_data*)soap_lookup_plugin(&soap, logging_id); /* need to access plugin's data */
  /* Unix SIGPIPE, this is OS dependent (win does not need this) */
  /* soap.accept_flags = SO_NOSIGPIPE; */ 	/* some systems like this */
  /* soap.socket_flags = MSG_NOSIGNAL; */	/* others need this */
  /* signal(SIGPIPE, sigpipe_handle); */	/* and some older Unix systems may require a sigpipe handler */
  m = soap_bind(&soap, NULL, port, BACKLOG);
  if (!soap_valid_socket(m))
  { soap_print_fault(&soap, stderr);
    exit(1);
  }
  fprintf(stderr, "Port bind successful: master socket = %d\n", m);
  for (i = 1; ; i++)
  { if (options[OPTION_s].value)
      soap.accept_timeout = atol(options[OPTION_s].value);
    s = soap_accept(&soap);
    if (!soap_valid_socket(s))
    { if (soap.errnum)
      { soap_print_fault(&soap, stderr);
        fprintf(stderr, "Retry...\n");
	continue;
      }
      fprintf(stderr, "gSOAP Web server timed out\n");
      break;
    }
    if (options[OPTION_v].selected)
      fprintf(stderr, "Thread %d accepts socket %d connection from IP %d.%d.%d.%d\n", i, s, (int)(soap.ip>>24)&0xFF, (int)(soap.ip>>16)&0xFF, (int)(soap.ip>>8)&0xFF, (int)soap.ip&0xFF);
    tsoap = soap_copy(&soap);
    tsoap->cookie_domain = options[OPTION_d].value;
    tsoap->cookie_path = options[OPTION_p].value;
    soap_set_cookie(tsoap, "visit", "true", NULL, NULL);
    soap_set_cookie_expire(tsoap, "visit", 600, NULL, NULL);
    if (options[OPTION_c].selected)
      soap_set_omode(tsoap, SOAP_IO_CHUNK); /* use chunked HTTP content (fast) */
    if (options[OPTION_k].selected)
      soap_set_omode(tsoap, SOAP_IO_KEEPALIVE);
    if (options[OPTION_t].value)
      tsoap->send_timeout = tsoap->recv_timeout = atol(options[OPTION_t].value);
    logdata->inbound = (options[OPTION_l].selected == 1 || options[OPTION_l].selected == 3);
    logdata->outbound = (options[OPTION_l].selected == 2 || options[OPTION_l].selected == 3);
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
    if (options[OPTION_i].selected)
    { if (soap_serve(tsoap))
      { fprintf(stderr, "Thread %d completed with failure %d\n", i, tsoap->error);
        soap_print_fault(tsoap, stderr);
      }
      soap_end(tsoap);
      soap_done(tsoap);
      if (options[OPTION_v].selected)
        fprintf(stderr, "Thread %d completed\n", i);
      free(tsoap);
    }
    else
    { tsoap->user = (void*)i;
      pthread_create(&tid, NULL, (void*(*)(void*))process_request, (void*)tsoap);
    }
  }
#ifdef WITH_OPENSSL
  CRYPTO_thread_cleanup();
#endif
  free_options(options);
  soap_end(&soap);
  soap_done(&soap);
  return 0;
}

/******************************************************************************\
 *
 *	Process dispatcher
 *
\******************************************************************************/

void *process_request(void *soap)
{ struct soap *tsoap = (struct soap*)soap;
  pthread_detach(pthread_self());
  if (soap_serve(tsoap))
  { fprintf(stderr, "Thread %d completed with failure %d\n", (int)tsoap->user, tsoap->error);
    soap_print_fault(tsoap, stderr);
  }
  else if (options[OPTION_v].selected)
    fprintf(stderr, "Thread %d completed\n", (int)tsoap->user);
  /* soap_destroy((struct soap*)soap); */ /* cleanup class instances (but this is a C app) */
  soap_end(tsoap);
  soap_done(tsoap);
  free(soap);
  return NULL;
}

/******************************************************************************\
 *
 *	SOAP/XML handling: calculator example
 *
\******************************************************************************/

int ns__add(struct soap *soap, double a, double b, double *c)
{ *c = a + b;
  return SOAP_OK;
}

int ns__sub(struct soap *soap, double a, double b, double *c)
{ *c = a - b;
  return SOAP_OK;
}

int ns__mul(struct soap *soap, double a, double b, double *c)
{ *c = a * b;
  return SOAP_OK;
}

int ns__div(struct soap *soap, double a, double b, double *c)
{ *c = a / b;
  return SOAP_OK;
}

/******************************************************************************\
 *
 *	Server dummy methods to avoid link errors
 *
\******************************************************************************/

int ns__addResponse_(struct soap *soap, double a)
{ return SOAP_NO_METHOD; /* we don't use this: we use soap_send_ns__addResponse instead */
}

int ns__subResponse_(struct soap *soap, double a)
{ return SOAP_NO_METHOD; /* we don't use this: we use soap_send_ns__subResponse instead */
}

int ns__mulResponse_(struct soap *soap, double a)
{ return SOAP_NO_METHOD; /* we don't use this: we use soap_send_ns__mulResponse instead */
}

int ns__divResponse_(struct soap *soap, double a)
{ return SOAP_NO_METHOD; /* we don't use this: we use soap_send_ns__divResponse instead */
}

/******************************************************************************\
 *
 *	HTTP GET handler for plugin
 *
\******************************************************************************/

int http_get_handler(struct soap *soap)
{ /* gSOAP 2.5 soap_response() will do this automatically for us, when sending SOAP_HTML or SOAP_FILE:
  if ((soap->omode & SOAP_IO) != SOAP_IO_CHUNK)
    soap_set_omode(soap, SOAP_IO_STORE); */ /* if not chunking we MUST buffer entire content when returning HTML pages to determine content length */
#ifdef WITH_ZLIB
  if (options[OPTION_z].selected && soap->zlib_out == SOAP_ZLIB_GZIP) /* client accepts gzip */
    soap_set_omode(soap, SOAP_ENC_ZLIB); /* so we can compress content (gzip) */
  soap->z_level = 9; /* best compression */
#endif
  /* Use soap->path (from request URL) to determine request: */
  if (options[OPTION_v].selected)
    fprintf(stderr, "HTTP GET Request: %s\n", soap->endpoint);
  /* Note: soap->path always starts with '/' */
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
  if (!soap_tag_cmp(soap->path, "*.png"))
    return copy_file(soap, soap->path + 1, "image/png");
  if (!soap_tag_cmp(soap->path, "*.ico"))
    return copy_file(soap, soap->path + 1, "image/ico");
  if (!strncmp(soap->path, "/calc?", 6))
    return calc(soap);
  if (!strncmp(soap->path, "/genivia", 8))
  { strcpy(soap->endpoint, "http://genivia.com"); /* redirect */
    strcat(soap->endpoint, soap->path + 8);
    return 307; /* Temporary Redirect */
  }
  /* Check requestor's authentication: */
  if (check_authentication(soap))
    return 401; /* HTTP not authorized */
  /* Return Web server status */
  if (soap->path[1] == '\0' || soap->path[1] == '?')
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
  fd = fopen(name, "rb"); /* open file to copy */
  if (!fd)
    return 404; /* return HTTP not found */
  soap->http_content = type;
  if (soap_response(soap, SOAP_FILE)) /* OK HTTP response header */
  { soap_end_send(soap);
    fclose(fd);
    return soap->error;
  }
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
  return soap_end_send(soap);
}

/******************************************************************************\
 *
 *	Example dynamic HTTP GET form-based calculator
 *
\******************************************************************************/

int calc(struct soap *soap)
{ int o = 0, a = 0, b = 0;
  char *s = query(soap); /* get argument string from URL ?query string */
  while (s)
  { char *key = query_key(soap, &s); /* decode next query string key */
    char *val = query_val(soap, &s); /* decode next query string value (if any) */
    if (key && val)
    { if (!strcmp(key, "o"))
        o = val[0];
      else if (!strcmp(key, "a"))
        a = strtol(val, NULL, 10);
      else if (!strcmp(key, "b"))
        b = strtol(val, NULL, 10);
    }
  }
  /* since the HTTP response header must be produced and output before the SOAP/XML message, we have to make sure that chunking is used or the message is stored for transmission */
  if ((soap->omode & SOAP_IO) != SOAP_IO_CHUNK)
    soap_set_omode(soap, SOAP_IO_STORE); /* if not chunking we MUST buffer entire content when returning HTML pages to determine content length */
  soap_response(soap, SOAP_OK);
  switch (o)
  { case 'a':
      return soap_send_ns__addResponse_(soap, "", NULL, a + b);	/* URL="" ensures current socket is used and no HTTP header is produced */
    case 's':
      return soap_send_ns__subResponse_(soap, "", NULL, a - b);	/* URL="" ensures current socket is used and no HTTP header is produced */
      break;
    case 'm':
      return soap_send_ns__mulResponse_(soap, "", NULL, a * b);	/* URL="" ensures current socket is used and no HTTP header is produced */
    case 'd':
      return soap_send_ns__divResponse_(soap, "", NULL, a / b);	/* URL="" ensures current socket is used and no HTTP header is produced */
    default:
      return soap_sender_fault(soap, "Unknown operation", NULL);
  }
  return SOAP_OK;
}

/******************************************************************************\
 *
 *	Example dynamic HTTP POST multipart/form-data form-based calculator
 *
\******************************************************************************/

int f__form(struct soap *soap, struct f__formResponse *response)
{ int o = 0, a = 0, b = 0;
  struct soap_multipart *content;
  for (content = soap->mime.list; content; content = content->next)
  { if (content->id && content->ptr)
    { /* may have to check content->encoding to convert data when necessary! */
      if (!strcmp(content->id, "o"))
        o = content->ptr[0];
      else if (!strcmp(content->id, "a"))
        a = strtol(content->ptr, NULL, 10);
      else if (!strcmp(content->id, "b"))
        b = strtol(content->ptr, NULL, 10);
    }
  }
  switch (o)
  { case 'a':
      response->result = a + b;
      break;
    case 's':
      response->result = a - b;
      break;
    case 'm':
      response->result = a * b;
      break;
    case 'd':
      response->result = a / b;
      break;
    default:
      return soap_sender_fault(soap, "Unknown operation", NULL);
  }
  return SOAP_OK;
}

/******************************************************************************\
 *
 *	Dynamic Web server info page
 *
\******************************************************************************/

int info(struct soap *soap)
{ struct http_get_data *getdata;
  struct logging_data *logdata;
  const char *t0, *t1, *t2, *t3, *t4, *t5, *t6, *t7;
  char buf[2048]; /* buffer large enough to hold HTML content */
  struct soap_plugin *p;
  time_t now = time(NULL), elapsed = now - start;
  query_options(soap, options);
  if (soap->omode & SOAP_IO_KEEPALIVE)
    t0 = "<td align='center' bgcolor='green'>YES</td>";
  else
    t0 = "<td align='center' bgcolor='red'>NO</td>";
#ifdef WITH_COOKIES
  t1 = "<td align='center' bgcolor='green'>YES</td>";
  /* soap_env_cookie_value() returns value of a cookie received (from client) */
  if (soap_env_cookie_value(soap, "visit", NULL, NULL))
    t2 = "<td align='center' bgcolor='green'>PASS</td>";
  else
    t2 = "<td align='center' bgcolor='yellow'>WAIT</td>";
#else
  t1 = "<td align='center' bgcolor='red'>NO</td>";
  t2 = "<td align='center' bgcolor='blue'>N/A</td>";
#endif
  if (secure)
  { t3 = "<td align='center' bgcolor='green'>YES</td>";
    if (soap->imode & SOAP_ENC_SSL)
      t4 = "<td align='center' bgcolor='green'>PASS</td>";
    else
      t4 = "<td align='center' bgcolor='red'><blink>FAIL</blink></td>";
  }
  else
  { t3 = "<td align='center' bgcolor='red'>NO</td>";
    t4 = "<td align='center' bgcolor='blue'>N/A</td>";
  }
#ifdef WITH_ZLIB
  if (options[OPTION_z].selected)
  { t5 = "<td align='center' bgcolor='green'>YES</td>";
    if (soap->omode & SOAP_ENC_ZLIB)
      t6 = "<td align='center' bgcolor='green'>PASS</td>";
    else
      t6 = "<td align='center' bgcolor='yellow'>WAIT</td>";
  }
  else
  { t5 = "<td align='center' bgcolor='red'>NO</td>";
    t6 = "<td align='center' bgcolor='blue'>N/A</td>";
  }
#else
  t5 = "<td align='center' bgcolor='red'>NO</td>";
  t6 = "<td align='center' bgcolor='blue'>N/A</td>";
#endif
  if (options[OPTION_c].selected || (soap->omode & SOAP_IO) == SOAP_IO_CHUNK)
    t7 = "<td align='center' bgcolor='green'>YES</td>";
  else
    t7 = "<td align='center' bgcolor='red'>NO</td>";
  if (soap_response(soap, SOAP_HTML))
    return soap->error;
  sprintf(buf, "\
<html>\
<head>\
<meta name='Author' content='Robert A. van Engelen'>\
<meta name='Generator' content='gSOAP'>\
<meta http-equiv='Refresh' content='60'>\
<style type='text/css' media='screen'><!-- table { background-color: #666 } td { color: white; font-size: 10px; line-height: 10px; font-family: Arial, Helvetica, Geneva, Swiss, SunSans-Regular } --></style>\
<title>gSOAP Web Server Administration</title>\
</head>\
<body bgcolor='#FFFFFF'>\
<h1>gSOAP Web Server Administration</h1>\
<p>Server endpoint=%s client agent IP=%d.%d.%d.%d\
<h2>Registered Plugins</h2>\
", soap->endpoint, (int)(soap->ip>>24)&0xFF, (int)(soap->ip>>16)&0xFF, (int)(soap->ip>>8)&0xFF, (int)soap->ip&0xFF);
  if (soap_send(soap, buf))
    return soap->error;
  for (p = soap->plugins; p; p = p->next)
  { sprintf(buf, "%s<br>", p->id);
    if (soap_send(soap, buf))
      return soap->error;
  }
  if (soap_send(soap, "<h2>Elapsed Time</h2>"))
    return soap->error;
  if (elapsed >= 86400)
    html_hbar(soap, "Days:", 100, elapsed/86400, 0x000000);
  if (elapsed >= 3600)
    html_hbar(soap, "Hours:", 100, elapsed/3600%24, 0x000000);
  html_hbar(soap, "Minutes:", 100, elapsed/60%60, 0x000000);
  soap_send(soap, "<h2>Control Panel</h2>");
  if (html_form_options(soap, options))
    return soap->error;
  sprintf(buf, "\
<h2>Function Tests</h2>\
<table border='0' cellspacing='0' cellpadding='0' bgcolor='#666666' nosave>\
<tr height='10'><td height='10' background='bl.gif'></td><td height='10'><i>Function</i></td><td align='center' height='10'><i>Result</i></td><td height='10' background='obls.gif'></td></tr>\
<tr><td background='bl.gif'></td><td>HTTP operational</td><td align='center' bgcolor='green'>YES</td><td width='10' background='ls.gif'></td></tr>\
<tr><td background='bl.gif'></td><td>HTTP keep alive enabled</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td background='bl.gif'></td><td>HTTP cookies enabled</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td background='bl.gif'></td><td>HTTP cookies test</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td background='bl.gif'></td><td>HTTPS (OpenSSL) enabled</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td background='bl.gif'></td><td>HTTPS (OpenSSL) test</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td background='bl.gif'></td><td>HTTP compression enabled</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td background='bl.gif'></td><td>HTTP compression test</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr><td background='bl.gif'></td><td>HTTP chunking enabled</td>%s<td width='10' background='ls.gif'></td></tr>\
<tr height='10'><td width='10' height='10' background=otrs.gif></td><td height='10' background='ts.gif'></td><td height='10' background='ts.gif'></td><td width='10' height='10' background='otls.gif'></td></tr>\
</table>", t0, t1, t2, t3, t4, t5, t6, t7);
  if (soap_send(soap, buf))
    return soap->error;
  getdata = (struct http_get_data*)soap_lookup_plugin(soap, http_get_id);
  logdata = (struct logging_data*)soap_lookup_plugin(soap, logging_id);
  soap_send(soap, "<h2>Usage Statistics</h2>");
  if (getdata)
  { html_hbar(soap, "HTTP&nbsp;GET", 120, getdata->stat_get, 0x0000FF);
    html_hbar(soap, "HTTP&nbsp;POST", 120, getdata->stat_post, 0x00FF00);
    html_hbar(soap, "HTTP&nbsp;FAIL", 120, getdata->stat_fail, 0xFF0000);
  }
  if (logdata)
  { html_hbar(soap, "SENT(kB)", 120, logdata->stat_sent/1024, 0x00FFFF);
    html_hbar(soap, "RECV(kB)", 120, logdata->stat_recv/1024, 0x00FFFF);
    if (elapsed > 0)
    { html_hbar(soap, "SENT(kB/s)", 120, logdata->stat_sent/elapsed/1024, 0x00FFFF);
      html_hbar(soap, "RECV(kB/s)", 120, logdata->stat_recv/elapsed/1024, 0x00FFFF);
    }
  }
  if (getdata)
  { struct tm T;
    T.tm_min = 99;
    T.tm_hour = 99;
    T.tm_yday = 999;
#ifdef HAVE_LOCALTIME_R
    localtime_r(&now, &T);
#else
    localtime(&now, &T);
#endif
    soap_send(soap, "<h2>Requests by the Minute</h2>");
    html_hist(soap, "Minute", 12, 0, 60, minutes, getdata->min, T.tm_min);
    soap_send(soap, "<h2>Requests by the Hour</h2>");
    html_hist(soap, "Hour", 30, 0, 24, hours, getdata->hour, T.tm_hour);
    soap_send(soap, "<h2>Requests by Day of the Year</h2>");
    html_hist(soap, "Day", 2, 0, 365, NULL, getdata->day, T.tm_yday);
  }
  soap_send(soap, "\
<p>This page will automatically reload every minute to refresh the statistics.\
<br><br><br><img src='favicon.gif' align='absmiddle'>Powered by gSOAP\
</body>\
</HTML>");
  return soap_end_send(soap);
}

static size_t html_scaled(char *buf, size_t len)
{ if (len > 1000000)
  { sprintf(buf, "%.2f&#183;10<sup>6</sup>", (float)len/1000000.0);
    return len / 1000000;
  }
  if (len > 1000)
  { sprintf(buf, "%.2f&#183;10<sup>3</sup>", (float)len/1000.0);
    return len / 1000;
  }
  sprintf(buf, "%lu", (unsigned long)len);
  return len;
}

int html_hbar(struct soap *soap, const char *title, size_t pix, size_t len, unsigned long color)
{ char buf[2048]; /* buffer large enough to hold HTML content */
  char lab[32];
  len = html_scaled(lab, len);
  sprintf(buf, "\
<table border='0' cellspacing='0' cellpadding='0' height='30'>\
<tr height='10'>\
<td width='10' height='10' background='bl.gif'></td>\
<td rowspan='2' bgcolor='#666666' width='%lu' height='20'>%s&nbsp;%s</td>\
<td bgcolor='#%.6lx' width='%lu' height='10'></td>\
<td width='10' height='10' background='obls.gif'></td>\
</tr>\
<tr height='10'>\
<td width='10' height='10' background='bl.gif'></td>\
<td height='10' background='ruler.gif'></td>\
<td width='10' height='10' background='ls.gif'></td>\
</tr>\
<tr height='10'>\
<td width='10' height='10' background='otrs.gif'></td>\
<td colspan='2' height='10' background='ts.gif'></td>\
<td width='10' height='10' background='otls.gif'></td>\
</tr>\
</table>", (unsigned long)pix, title ? title : "", lab, color, (unsigned long)len * 2);
  return soap_send(soap, buf);
}

int html_hist(struct soap *soap, const char *title, size_t barwidth, size_t height, size_t num, const char **key, size_t *val, size_t highlight)
{ char buf[2048]; /* buffer large enough to hold HTML content */
  char lab[32];
  size_t i, max;
  float scale;
  max = 0;
  for (i = 0; i < num; i++)
  { if (val[i] > max)
      max = val[i];
  }
  if (height < 20)
  { height = max;
    if (height < 20)
      height = 20;
    else if (height > 256)
      height = 256;
  }
  scale = (float)height / (float)max;
  html_scaled(lab, max);
  sprintf(buf, "\
<a name='%s'></a>\
<table bgcolor='#FFFFFF' border='0' cellspacing='0' cellpadding='0' height='%lu' align='center'>\
<tr height='10'>\
<td width='10' height='10' background='btl.gif'></td><td colspan='%lu' height='10' background='bt.gif'></td><td width='10' height='10' background='btr.gif'></td><td width='10' height='10' background='obls.gif'></td>\
</tr>\
<tr height='%lu' align='center' valign='bottom'>\
<td width='10' height='%lu' background='bl.gif'></td>\
<td bgcolor='#666666' valign='top'>%s</td>", title ? title : "", (unsigned long)height + 50, (unsigned long)num + 1, (unsigned long)height, (unsigned long)height, lab);
  if (soap_send(soap, buf))
    return soap->error;
  for (i = 0; i < num; i++)
  { unsigned long bar = (scale * val[i] + 0.5);
    if (bar >= 1)
      sprintf(buf, "\
<td bgcolor='#FFFFFF'><a onmouseover=\"window.status='%lu';return true\" onmouseout=\"window.status='';return true\" href='#%s'><img src='top.gif' alt='' width='%lu' height='1' align='bottom' border='0'><br><img src='bar.gif' alt='' width='%lu' height='%lu' align='bottom' border='0'></a></td>", (unsigned long)i, title ? title : "", (unsigned long)barwidth, (unsigned long)barwidth, bar - 1);
    else
      sprintf(buf, "\
<td bgcolor='#FFFFFF'><img src='bar.gif' alt='' width='%lu' height='0' align='bottom' border='0'></td>", (unsigned long)barwidth);
    if (soap_send(soap, buf))
      return soap->error;
  }
  sprintf(buf, "\
<td width='10' height='%lu' background='br.gif'></td>\
<td width='10' height='%lu' background='ls.gif'></td>\
</tr>\
<tr bgcolor='#666666' height='20' align='center'>\
<td width='10' height='20' background='bl.gif'></td>\
<td bgcolor='#666666'>%s</td>", (unsigned long)height, (unsigned long)height, title ? title : "");
  if (soap_send(soap, buf))
    return soap->error;
  for (i = 0; i < num; i++)
  { sprintf(buf, "<td%s>%s</td>", (i == highlight) ? " bgcolor='#777777'" : "", key ? key[i] : "<img src='bar.gif'>");
    if (soap_send(soap, buf))
      return soap->error;
  }
  if (soap_send(soap, "\
<td width='10' height='20' background='br.gif'></td>\
<td width='10' height='20' background='ls.gif'></td>"))
    return soap->error;
  sprintf(buf, "\
</tr>\
<tr height='10'>\
<td width='10' height='10' background='bbl.gif'></td><td colspan='%lu' height='10' background='bb.gif'></td><td width='10' height='10' background='bbr.gif'></td><td width='10' height='10' background='ls.gif'></td>\
</tr>\
<tr height='10'>\
<td width='10' height='10' background='otrs.gif'></td>\
<td colspan='%lu' height='10' background='ts.gif'></td>\
<td width='10' height='10' background='otls.gif'></td>\
</tr>\
</table>", (unsigned long)num + 1, (unsigned long)num + 2);
  return soap_send(soap, buf);
}

/******************************************************************************\
 *
 *	OpenSSL
 *
\******************************************************************************/

#ifdef WITH_OPENSSL

#if defined(WIN32)
# define MUTEX_TYPE		HANDLE
# define MUTEX_SETUP(x)		(x) = CreateMutex(NULL, FALSE, NULL)
# define MUTEX_CLEANUP(x)	CloseHandle(x)
# define MUTEX_LOCK(x)		WaitForSingleObject((x), INFINITE)
# define MUTEX_UNLOCK(x)	ReleaseMutex(x)
# define THREAD_ID		GetCurrentThreadId()
#elif defined(_POSIX_THREADS)
# define MUTEX_TYPE		pthread_mutex_t
# define MUTEX_SETUP(x)		pthread_mutex_init(&(x), NULL)
# define MUTEX_CLEANUP(x)	pthread_mutex_destroy(&(x))
# define MUTEX_LOCK(x)		pthread_mutex_lock(&(x))
# define MUTEX_UNLOCK(x)	pthread_mutex_unlock(&(x))
# define THREAD_ID		pthread_self()
#else
# error "You must define mutex operations appropriate for your platform"
# error	"See OpenSSL /threads/th-lock.c on how to implement mutex on your platform"
#endif

struct CRYPTO_dynlock_value
{ MUTEX_TYPE mutex;
};

static MUTEX_TYPE *mutex_buf;

static struct CRYPTO_dynlock_value *dyn_create_function(const char *file, int line)
{ struct CRYPTO_dynlock_value *value;
  value = (struct CRYPTO_dynlock_value*)malloc(sizeof(struct CRYPTO_dynlock_value));
  if (value)
    MUTEX_SETUP(value->mutex);
  return value;
}

static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line)
{ if (mode & CRYPTO_LOCK)
    MUTEX_LOCK(l->mutex);
  else
    MUTEX_UNLOCK(l->mutex);
}

static void dyn_destroy_function(struct CRYPTO_dynlock_value *l, const char *file, int line)
{ MUTEX_CLEANUP(l->mutex);
  free(l);
}

void locking_function(int mode, int n, const char *file, int line)
{ if (mode & CRYPTO_LOCK)
    MUTEX_LOCK(mutex_buf[n]);
  else
    MUTEX_UNLOCK(mutex_buf[n]);
}

unsigned long id_function()
{ return (unsigned long)THREAD_ID;
}

int CRYPTO_thread_setup()
{ int i;
  mutex_buf = (MUTEX_TYPE*)malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
  if (!mutex_buf)
    return SOAP_EOM;
  for (i = 0; i < CRYPTO_num_locks(); i++)
    MUTEX_SETUP(mutex_buf[i]);
  CRYPTO_set_id_callback(id_function);
  CRYPTO_set_locking_callback(locking_function);
  CRYPTO_set_dynlock_create_callback(dyn_create_function);
  CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
  CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
  return SOAP_OK;
}

void CRYPTO_thread_cleanup()
{ int i;
  if (!mutex_buf)
    return;
  CRYPTO_set_id_callback(NULL);
  CRYPTO_set_locking_callback(NULL);
  CRYPTO_set_dynlock_create_callback(NULL);
  CRYPTO_set_dynlock_lock_callback(NULL);
  CRYPTO_set_dynlock_destroy_callback(NULL);
  for (i = 0; i < CRYPTO_num_locks(); i++)
    MUTEX_CLEANUP(mutex_buf[i]);
  free(mutex_buf);
  mutex_buf = NULL;
}

#endif

/******************************************************************************\
 *
 *	SIGPIPE
 *
\******************************************************************************/

void sigpipe_handle(int x) { }

