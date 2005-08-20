/*

md5evp.c

gSOAP HTTP Content-MD5 digest EVP handler for httpmd5 plugin

gSOAP XML Web services tools
Copyright (C) 2000-2005, Robert van Engelen, Genivia Inc., All Rights Reserved.
This part of the software is released under one of the following licenses:
GPL, the gSOAP public license, or Genivia's license for commercial use.
--------------------------------------------------------------------------------
gSOAP public license.

The contents of this file are subject to the gSOAP Public License Version 1.3
(the "License"); you may not use this file except in compliance with the
License. You may obtain a copy of the License at
http://www.cs.fsu.edu/~engelen/soaplicense.html
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.

The Initial Developer of the Original Code is Robert A. van Engelen.
Copyright (C) 2000-2005, Robert van Engelen, Genivia, Inc., All Rights Reserved.
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
*/

#include "md5evp.h"

int md5_handler(struct soap *soap, void **context, enum md5_action action, char *buf, size_t len)
{ EVP_MD_CTX *ctx;
  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int size;
  switch (action)
  { case MD5_INIT:
#ifdef WITH_OPENSSL
      OpenSSL_add_all_digests();
#endif
      if (!*context)
      { *context = (void*)SOAP_MALLOC(soap, sizeof(EVP_MD_CTX));
        EVP_MD_CTX_init((EVP_MD_CTX*)*context);
      }
      ctx = (EVP_MD_CTX*)*context;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "-- MD5 Init %p\n", ctx));
      EVP_DigestInit(ctx, EVP_md5());
      break;
    case MD5_UPDATE:
      ctx = (EVP_MD_CTX*)*context;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "-- MD5 Update %p --\n", ctx));
      DBGMSG(TEST, buf, len);
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "\n--"));
      EVP_DigestUpdate(ctx, (void*)buf, (unsigned int)len);
      break;
    case MD5_FINAL:
      ctx = (EVP_MD_CTX*)*context;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "-- MD5 Final %p --\n", ctx));
      EVP_DigestFinal(ctx, (void*)hash, &size);
      memcpy(buf, hash, 16);
      break;
    case MD5_DELETE:
      ctx = (EVP_MD_CTX*)*context;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "-- MD5 Delete %p --\n", ctx));
      if (ctx)
      { EVP_MD_CTX_cleanup(ctx);
        SOAP_FREE(soap, ctx);
      }
      *context = NULL;
  }
  return SOAP_OK;
}
