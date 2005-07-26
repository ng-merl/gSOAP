/*

logging.h

Message logging plugin for webserver.

Currently this plugin flushes messages to stderr.

Copyright (C) 2000-2005 Robert A. van Engelen, Genivia inc.
All Rights Reserved.
*/

#include "stdsoap2.h"

#define LOGGING_ID "LOGGING-1.1"

extern const char logging_id[];

struct logging_data
{ short inbound;
  short outbound;
  size_t stat_sent;
  size_t stat_recv;
  int (*fsend)(struct soap*, const char*, size_t); /* to save and use send callback */
  size_t (*frecv)(struct soap*, char*, size_t); /* to save and use recv callback */
};

int logging(struct soap *soap, struct soap_plugin *plugin, void *arg);
