/*	router.c

	Web Service message router (relay server and message forwarding)
	Note: HTTP cookies are not supported

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

	Configure:
	The router uses two routing tables: an internal table (for speed) and
	an external routing file (for flexibility). The internal is always
	checked first. Change the contents of the tables to your needs.
	Internal table: struct t__Routing routing[] (see below)
	External table: provide the name of a default routing file (see below)
			or use router option -r

	Compile:
	soapcpp2 -c router.h
	gcc -o router router.c stdsoap2.c soapC.c

	Usage scenarios
	===============

	Forwarding of messages to a service
	-----------------------------------

	router [-e<endpoint> | -g<endpoint>] [-a<SOAPAction>] [-r<routingfile>] [-t<timeout>] [<msgfile>]

	Examples:

	1.
	router request.soap
	Sends the request message stored in file request.soap and returns
	response to stdout where file request.soap contains a SOAP request with
	HTTP header and SOAP/XML/DIME body. If the SOAPAction in the message is
	present and matches one or more keys in the routing table, the
	alternative service endpoints in the table will be tried first until
	one service endpoint is found to accept the connection. If no
	SOAPAction is given or the SOAPAction does not match any key, then the
	endpoint in the HTTP header in request.soap is searched in the routing
	table. If the endoint matches one or more keys in the routing table,
	the alternative endpoints will be tried first until one endpoint is
	found to accept the connection. Finally, the endpoint in the HTTP
	header of request.soap is used to establish a connection if all other
	service endpoints in the table failed.

	2.
	router -ehttp://domain/path request.soap
	Sends request message to http://domain/path and returns the service
	response to stdout. If http://domain/path matches one or more keys in
	the routing table, then the alternative service endpoints in the table
	will be tried first until one service endpoint is found to accept the
	connection. The http://domain/path endpoint is tried last when all
	other service endpoints in the table failed. File request.soap MAY
	contain an HTTP header but MUST of course contain a body.

	To try this, compile the 'quote' client (samples/quote). Edit the
	'quote.getQuote.req.xml' SOAP/XML request file and replace
	<symbol></symbol> with <symbol>AOL</symbol>. Then run
	router -ehttp://services.xmethods.net/soap -a"" quote.getQuote.req.xml
	The SOAP/XML response is returned.

	3.
	router -aSOAPAction request.soap
	When SOAPAction matches one or more keys in the routing table, then the
	alternative endpoints in the table will be tried first until one
	endpoint is found to accept the connection. When all endpoints fail,
	or when SOAPAction does not match a key, the router fails. File
	request.soap MAY contain an HTTP header but MUST of course contain a
	body.

	4.
	router -rroutingtable.xml request.soap
	Same as 1. but uses routingtable.xml as the routing table after
	checking keys in the internal routing table. The XSD schema of
	routingtable.xml is generated as t.xsd. The default routing table file
	is router.xml.

	5.
	router -t5 request.soap
	Same as 1. but searches the routing table for an endpoint that takes
	less than 5 seconds to connect to. Use negative timeouts to specify a
	timeout value in microseconds. The timeout also specifies the message
	receive timeout AFTER the connection was established.

	6.
	cat request.soap | router -ehttp://domain/path | more
	When request.soap does not contain an HTTP header, the router computes
	the HTTP content length by buffering the entire request message which
	allows you to use it as a filter as in this example. (fstat() is
	generally tried first to determine file length.)

	7.
	router -ghttp://domain/path/file.html
	Sends an HTTP GET request to the host and copies the response to stdout.

	CGI-based relay server
	----------------------

	Install the router as CGI application. The CGI-based relay service uses
	SOAPActions in the messages and HTTP query strings to index the routing
	table.

	Examples:

	Messages addressed to "http://domain/cgi-bin/router?key" will be routed
	by the router to the service endpoint associated with the key in the
	routing table. When messages use SOAPActions, the SOAPActions will be
	used to find service endpoints instead of a query string.

	To tunnel SOAP through firewals to stateful stand-alone Web services:
	run a stand-alone gSOAP Web service on a port, e.g. 18000. Add the
	key-endpoint pair "myservice", "http://localhost:18000" to the router
	table. After installing the router, all requests for endpoint
	http://domain/cgi-bin/router?myservice will be tunneled to the
	stand-alone Web service.

	To add backup services: add multiple key-endpoint pairs to the routing
	table with the same key. Given a key (e.g. SOAPAction or Query string)
	the router will check the endpoints in sequence until it can connect.
	If one or more of the backup services are down, an active service
	endpoint will be selected.

	Multi-threaded stand-alone relay server
	---------------------------------------

	router -p<port> [-r<routingfile>] [-t<timeout>] &
	
	Examples:
	
	router -p18000 -rtable.xml -t5 &
	Runs a stand-alone router on port 18000 using table.xml as the external
	routing table for lookup. Service endpoints are selected from
	alternative endpoints that take less than 5 seconds to connect to.

	Clients connect to the router with a service endpoint such as
	"http://machine:<port>/path" where the endpoint "http://machine/path"
	(note the absence of the port) will be used as a key in the routing
	table to find an endpoint when no SOAPAction is present. For example, a
	stand-alone Web service called "quote" runs on a machine named "zulu"
	port 18080. To address this service through the router, add key
	"http://zulu/quote" and endpoint "http://zulu:18080" to the routing
	table. Run the router on port 18000.  Router requests with endpoint
	"http://zulu:18000/quote" will be relayed to zulu:18080

	Notes
	-----

	* Table lookup algorithm:
	  SOAPActions (if provided) are used first to match routing table keys.
	  Next, HTTP query string in the endpoint URL (CGI only) is used to
	  match routing table keys.
	  Next, the service endpoint is checked to match routing table keys.
	  Finally, the service endpoint URL itself is used to connect.
	* Keys in routing table may contain * (multi-char) and - (single-char)
	  wildcards to match multiple SOAPActions and endpoints.
	* When a match is found but the endpoint is NULL in the table, the
	  search is terminated. This can be used to prevent searches in the
	  routing file for specific patterns.
	* <timeout> is TCP connect and I/O timeout for router-server connection
	  in seconds (use negative value for timeout in microseconds).
	* When an external routing table is once read by a stand-alone router,
	  it will be cached to optimize speed. But this also means that
	  changing the contents of the routing table file does not affect the
	  actual routing while the stand-alone router is running.
	* HTTP POST and HTTP GET styles of SOAP messaging is supported
	  (but CGI-based router does not support HTTP GET)
	* Supports any type of messages (e.g. DIME)
	* HTTP cookies are not handled and will be deleted from the HTTP header
	* Keep-alive support has not been tested and might not work
*/

#include "soapH.h"
#include <sys/stat.h>	/* need fstat */
#include <pthread.h>	/* use Posix threads */

/* Maximum request backlog */
#define BACKLOG (100)

/* Default file name of external routing table (or NULL if none used) */
#define DEFAULT_ROUTINGFILE "router.xml"

/* Internal routine table (fast) */
static struct t__Routing routing[] =
/* SOAPAction/endpoint	-> target endpoint */
{ {"dime",		"http://websrv.cs.fsu.edu/~engelen/dimesrv.cgi"},
  {"http://*/dime",	"http://websrv.cs.fsu.edu/~engelen/dimesrv.cgi"},
  {"factory",		"http://localhost:18085"},
  {NULL, NULL}
};

#ifdef WIN32
#define OPTION_CHAR '/'
#else
#define OPTION_CHAR '-'
#endif

struct header
{ struct header *next;
  char line[SOAP_HDRLEN];
};

static int port_number = 0;
static const char *input_file = NULL;
static const char *service_endpoint = NULL;
static const char *service_action = NULL;
static const char *routing_file = DEFAULT_ROUTINGFILE;
static int server_timeout = 0;
static int method = SOAP_POST;

void options(int, char**);
void *process_request(void*);
const char *lookup(struct RoutingTable*, const char*);
int copy_header(struct soap*, struct soap*, const char*, const char*);
int create_header(struct soap*, int, const char*, const char*, size_t);
int buffer_body(struct soap*);
int copy_body(struct soap*, struct soap*);

int
main(int argc, char **argv)
{ options(argc, argv);
  if (port_number)
  { struct soap soap, *tsoap;
    pthread_t tid;
    int m, s, i;
    soap_init(&soap);
    soap.bind_flags = SO_REUSEADDR; /* don't use this in a non-secure environment. We keep it here so you can quickly restart the router */
    m = soap_bind(&soap, NULL, port_number, BACKLOG);
    if (m < 0)
    { soap_print_fault(&soap, stderr);
      exit(1);
    }
    fprintf(stderr, "Socket connection successful %d\n", m);
    for (i = 1; ; i++)
    { s = soap_accept(&soap);
      if (s < 0)
      { soap_print_fault(&soap, stderr);
        break;
      }
      fprintf(stderr, "Thread %d accepts socket %d connection from IP %d.%d.%d.%d\n", i, s, (int)(soap.ip>>24)&0xFF, (int)(soap.ip>>16)&0xFF, (int)(soap.ip>>8)&0xFF, (int)soap.ip&0xFF);
      tsoap = soap_copy(&soap);
      pthread_create(&tid, NULL, (void*(*)(void*))process_request, (void*)tsoap);
    }
  }
  else
  { struct soap client;
    struct soap server;
    soap_init(&client);
    soap_init(&server);
    soap_begin(&client);
    if (argc <= 1) /* try CGI env vars */
    { char *s = getenv("REQUEST_METHOD");
      if (s && !strcmp(s, "GET"))
        method = SOAP_GET;
      else
      { s = getenv("Content-Length");
        if (s)
          client.length = strtoul(s, NULL, 10);
      }
      service_action = getenv("HTTP_SOAPAction");
      if (!service_action)
        service_action = getenv("QUERY_STRING");
    }
    if (method == SOAP_POST)
    { wchar c;
      if (input_file)
      { client.recvfd = open(input_file, O_RDONLY);
        if (client.recvfd < 0)
        { fprintf(stderr, "router: cannot open file '%s' for reading\n", input_file);
          exit(1);
        }
      }
      c = soap_getchar(&client);
      client.bufidx--; /* instead of unget: enables copying buffer */
      if (c == 'G' || c == 'P') /* simple check to see if HTTP GET/POST header is present */
      { if (copy_header(&client, &server, service_endpoint, service_action))
        { client.error = server.error;
          soap_send_fault(&client);
          exit(1);
        }
      }
      else
      { struct stat sb;
        if (!fstat(client.recvfd, &sb) && sb.st_size > 0)
          client.length = sb.st_size;
        else
          buffer_body(&client);
        if (create_header(&server, SOAP_POST, service_endpoint, service_action, client.length))
        { client.error = server.error;
          soap_send_fault(&client);
          exit(1);
        }
      }
      copy_body(&client, &server);
    }
    else
    { if (create_header(&server, SOAP_GET, service_endpoint, service_action, 0))
      { client.error = server.error;
        soap_send_fault(&client);
        exit(1);
      }
      soap_end_send(&server);
    }
    soap_begin(&server);
    /* should check these for errors: */
    copy_header(&server, &client, NULL, NULL);
    copy_body(&server, &client);
    soap_end(&client);
    soap_end(&server);
    soap_done(&client);
    soap_done(&server);
  }
  return 0;
}

void
options(int argc, char **argv)
{ int i, flag;
  char *arg;
  for (i = 1; i < argc; i++)
  { arg = argv[i];
    if (*arg == OPTION_CHAR)
    { flag = 1;
      while (flag && *++arg)
        switch (*arg)
        { case 'h':
            fprintf(stderr, "Usage: router [-p<port>] [-e<endpoint> | -g<endpoint>] [-a<action>] [-r<routingfile>] [<msgfile>]\n");
            exit(0);
          case 'p':
            flag = 0;
            if (*++arg)
              port_number = atol(arg);
            else if (i < argc && argv[++i])
              port_number = atol(argv[i]);
            else
            { fprintf(stderr, "router: -p requires <port>\n");
              exit(1);
            }
            break;
          case 'g':
	    method = SOAP_GET;
          case 'e':
            flag = 0;
            if (*++arg)
              service_endpoint = arg;
            else if (i < argc && argv[++i])
              service_endpoint = argv[i];
            else
            { fprintf(stderr, "router: -e and -g require <endpoint>\n");
              exit(1);
            }
            break;
          case 'a':
            flag = 0;
            if (*++arg)
              service_action = arg;
            else if (i < argc && argv[++i])
              service_action = argv[i];
            else
            { fprintf(stderr, "router: -a requires <action>\n");
              exit(1);
            }
            break;
          case 'r':
            flag = 0;
            if (*++arg)
              routing_file = arg;
            else if (i < argc && argv[++i])
              routing_file = argv[i];
            else
            { fprintf(stderr, "router: -r requires <routingfile>\n");
              exit(1);
            }
            break;
          case 't':
            flag = 0;
            if (*++arg)
              server_timeout = atol(arg);
            else if (i < argc && argv[++i])
              server_timeout = atol(argv[i]);
            else
            { fprintf(stderr, "router: -t requires <timeout>\n");
              exit(1);
            }
            break;
          default:
            fprintf(stderr, "router: unknown option -%c\n", *arg);
	}
    }
    else
      input_file = arg;
  }
}

void*
process_request(void *soap)
{ struct soap *client, server;
  wchar c;
  pthread_detach(pthread_self());
  client = (struct soap*)soap;
  soap_init(&server);
  soap_begin(client);
  c = soap_getchar(client);
  client->bufidx--; /* instead of unget: enables copying buffer */
  if (c == 'G' || c == 'P') /* simple check to see if HTTP GET/POST header is present */
  { if (copy_header(client, &server, NULL, NULL))
      client->error = server.error;
  }
  else
  { buffer_body(client);
    if (create_header(&server, method, service_endpoint, service_action, client->length))
      client->error = server.error;
  }
  if (!server.error)
  { copy_body(client, &server);
    soap_begin(&server);
    copy_header(&server, client, NULL, NULL);
    copy_body(&server, client);
  }
  else
    soap_send_fault(client);
  soap_end(client);
  soap_end(&server);
  soap_done(&server);
  soap_done(client);
  free(soap);
  return NULL;
}

const char*
lookup(struct RoutingTable *route, const char *key)
{ static struct RoutingTable routing_table = {NULL, 0}; /* file-based routing table cache */
  if (!key)
    return NULL; /* can't do lookup on nil key */
  if (!route->__ptr)
  { route->__ptr = routing; /* first stage: use internal routing table */
    route->__size = 999999999;
  }
  else if (route->__size)
  { route->__ptr++;
    route->__size--;
  }
  for (;;)
  { if (route->__ptr)
    { while (route->__size && route->__ptr->key)
      { if (!soap_tag_cmp(key, route->__ptr->key))
          return route->__ptr->endpoint;
        route->__ptr++;
        route->__size--;
      }
    }
    if (route->__size) /* second stage: use file-based routing table */
    { if (routing_table.__ptr)
        *route = routing_table; /* table is already cached in memory */
      else if (routing_file) /* else read table from file */
      { struct soap soap;
        soap_init(&soap);
        soap.recvfd = open(routing_file, O_RDONLY);
        if (soap.recvfd < 0) /* no routing file: silently stop */
	{ soap_done(&soap);
	  break;
	}
        if (!soap_begin_recv(&soap))
	  if (!soap_get_RoutingTable(&soap, &routing_table, "router", NULL))
	  { close(soap.recvfd);
	    soap_done(&soap);
	    break;
	  }
	soap_end_recv(&soap);
	close(soap.recvfd);
	soap_done(&soap);
	*route = routing_table;
      }
    }
    else
      break;
  }
  return NULL;
}

int
make_connect(struct soap *server, const char *endpoint)
{ char host[SOAP_TAGLEN];
  int port;
  strcpy(host, server->host);
  port = server->port;
  soap_set_endpoint(server, endpoint);	/* get host, path, and port */
  server->connect_timeout = server_timeout;
  server->recv_timeout = server_timeout;
  server->send_timeout = server_timeout;
  /* server->connect_flags = SO_NOSIGPIPE; */	/* prevents UNIX SIGPIPE */
  /* server->socket_flags = MSG_NOSIGNAL; */	/* prevents UNIX SIGPIPE */
  if (*server->host)
  { if (server->socket < 0 || strcmp(server->host, host) || server->port != port)
    { soap_closesock(server);
      server->socket = server->fopen(server, endpoint, server->host, server->port);
      if (server->socket < 0)
        return server->error;
    }
  }
  return SOAP_OK;
}

int
server_connect(struct soap *server, const char *endpoint, const char *action)
{ short c = 0;
  if (action && *action)
  { struct RoutingTable route;
    route.__ptr = NULL;
    route.__size = 0;
    fprintf(stderr, "Searching services on action %s...\n", action);
    while (lookup(&route, action))
    { fprintf(stderr, "Attempting to connect to '%s'\n", route.__ptr->endpoint);
      if (!make_connect(server, route.__ptr->endpoint))
      { c = 1;
        break;
      }
    }
  }
  if (!c && endpoint && *endpoint)
  { struct RoutingTable route;
    route.__ptr = NULL;
    route.__size = 0;
    fprintf(stderr, "Searching services on endpoint %s...\n", endpoint);
    while (lookup(&route, endpoint))
    { fprintf(stderr, "Attempting to connect to '%s'\n", route.__ptr->endpoint);
      if (!make_connect(server, route.__ptr->endpoint))
      { c = 1;
        break;
      }
    }
  }
  if (!c && endpoint && *endpoint)
  { fprintf(stderr, "Connect to endpoint %s...\n", endpoint);
    if (make_connect(server, endpoint))
      return server->error;
  }
  return SOAP_OK;
}

int
copy_header(struct soap *sender, struct soap *receiver, const char *endpoint, const char *action)
{ struct header *h, *p;
  char *s, *t;
  h = (struct header*)malloc(sizeof(struct header));
  for (;;)
  { if (soap_getline(sender, h->line, SOAP_HDRLEN))
      return sender->error = SOAP_EOF;
    t = strchr(h->line, ' ');
    if (!t || strncmp(t, " 100 ", 5))
      break;
    do
      if (soap_getline(sender, h->line, SOAP_HDRLEN))
        return sender->error = SOAP_EOF;
    while (*h->line); 
  }
  p = h;
  for (;;)
  { p = p->next = (struct header*)malloc(sizeof(struct header));
    p->next = NULL;
    if (soap_getline(sender, p->line, SOAP_HDRLEN))
      return sender->error = SOAP_EOF;
    if (!*p->line)
      break;
    s = t = strchr(p->line, ':');
    if (t)
    { *t = '\0';
      do t++;
      while (*t && *t <= 32);
    }
    sender->fparsehdr(sender, p->line, t);
    if (s)
      *s = ':';
  }
  s = strstr(h->line, "HTTP/");
  if (s && (!strncmp(h->line, "GET ", 4) || !strncmp(h->line, "POST ", 5)))
  { int m = strlen(sender->endpoint);
    int n = m + (s - h->line) - 5 - (*h->line == 'P');
    if (n >= (int)sizeof(sender->endpoint))
      n = sizeof(sender->endpoint) - 1;
    strncpy(sender->path, h->line + 4 + (*h->line == 'P'), n - m);
    sender->path[n - m] = '\0';
    strcat(sender->endpoint, sender->path);
  }
  if (!endpoint || !*endpoint)
    endpoint = sender->endpoint;
  if (!action || !*action)
    action = sender->action;
  if (server_connect(receiver, endpoint, action))
    return receiver->error;
  soap_begin_send(receiver);
  while (h)
  { if ((receiver->error = receiver->fposthdr(receiver, h->line, NULL)))
      return receiver->error;
    p = h->next;
    free(h);
    h = p;
  }
  if ((sender->mode & SOAP_IO) == SOAP_IO_CHUNK)
  { if (soap_flush(receiver))
      return receiver->error;
    receiver->mode = sender->mode;
  }
  return SOAP_OK;
}

int
create_header(struct soap *server, int method, const char *endpoint, const char *action, size_t count)
{ if (server_connect(server, endpoint, action))
    return server->error;
  soap_begin_send(server);
  server->status = method;
  return server->error = server->fpost(server, server->endpoint, server->host, server->path, action, count);
}

int
buffer_body(struct soap *sender)
{ char *s;
  if (soap_new_block(sender))
    return sender->error;
  for (;;)
  { if (!(s = (char*)soap_push_block(sender, sender->buflen - sender->bufidx)))
      return SOAP_EOM;
    memcpy(s, sender->buf + sender->bufidx, sender->buflen - sender->bufidx);
    if (soap_recv_raw(sender))
      break;
  }
  if (soap_end_recv(sender))
    return sender->error;
  sender->length = sender->blist->size;
  return SOAP_OK;
}

int
copy_body(struct soap *sender, struct soap *receiver)
{ if (sender->blist)
  { char *p;
    for (p = soap_first_block(sender); p; p = soap_next_block(sender))
      soap_send_raw(receiver, p, soap_block_size(sender));
    soap_end_block(sender);
  }
  else
  { if ((sender->mode & SOAP_IO) == SOAP_IO_CHUNK)
    { sender->chunkbuflen = sender->buflen;
      sender->buflen = sender->bufidx;
      sender->chunksize = 0;
      while (!soap_recv_raw(sender))
      { if (soap_send_raw(receiver, sender->buf + sender->bufidx, sender->buflen - sender->bufidx))
          return receiver->error;
      }
    }
    else
    { soap_send_raw(receiver, sender->buf + sender->bufidx, sender->buflen - sender->bufidx); /* send part after HTTP header */
      if (sender->buflen - sender->bufidx < sender->length)
      { sender->length -= sender->buflen - sender->bufidx;
        while (!soap_recv_raw(sender))
        { if (soap_send_raw(receiver, sender->buf, sender->buflen))
            return receiver->error;
	  if (sender->buflen >= sender->length)
	    break;
          sender->length -= sender->buflen;
        }
      }
    }
    if (soap_end_recv(sender))
      return sender->error;
  }
  if (soap_end_send(receiver))
    return receiver->error;
  return SOAP_OK;
}

struct Namespace namespaces[] =
{ {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/"},
  {"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/"},
  {"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance"},
  {"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema"},
  {"t", "http://tempuri.org"},
  {NULL, NULL}
};
