/*	dom.c

	gSOAP XML DOM extensions

	Copyright (C) 2000-2003 Robert A. van Engelen, Genivia inc.
	All Rights Reserved.

	Requires gSOAP 2.3 and higher.

	This level-2 DOM parser features automatic XML namespace handling and
	allows mixing with gSOAP's data type (de)serializers.

	DOM element nodes and attribute nodes can have an optional namespace
	names. The namespace prefix and name bindings (xmlns in XML) are
	automatically resolved.
	
	The DOM elements can be used anywhere within a C/C++ data structure and
	also as the arguments to a service method. For example:
	  struct SOAP_ENV__Header
	  { xsd__anyType *authentication;
	  };
	  int ns__test(xsd__anyType in, xsd__anyType *out);
	which defines a custom SOAP Header with an authentication element
	parsed as a DOM.
	
	The parser fills the wide string fields of the DOM node only, unless
	the flag SOAP_C_UTFSTRING is set. These wide fields contain Unicode XML
	cdata.

	The namespace name (URI) parsing is smart and checks the names in the
	nsmap table first. After parsing, the nstr namespace strings will point
	to the table contents in case the names (URIs) match. Otherwise, the
	names are dynamically allocated. This enables quick pointer-based
	checks on the DOM node's namespace names by comparing the pointer
	to the namespace table entries namespaces[i].ns.

	DOM node fields:
	const char *nstr	element namespace name (URI)
	      char *name	element name
	      char *data	optional element data
           wchar_t *wide	optional element data
	           *elts	optional sub elements (list of DOM nodes)
	       int __type	optional type (SOAP_TYPE_X as defined in soapH.h)
	      void *node	and optional element pointer to C/C++ data type
	           *atts	optional element attributes

	DOM Parsing:
	The namespace name (URI) parsing is smart and fills the 'nstr' field
	with the namespace URIs. The algorithm checks the names in the nsmap
	table first. After parsing, the 'nstr' namespace strings will point to
	the nsmap table contents in case the names (URIs) match a table entry.
	Otherwise, the names are dynamically allocated. This enables quick
	pointer-based checks on the DOM node's namespace names by comparing the
	pointer to the namespace table entries 'namespaces[i].ns'.
	The parser fills the 'wide' string fields of the DOM nodes only (the
	'wide' fields contain Unicode XML cdata), unless the input-mode flag
	SOAP_C_UTFSTRING is set. In that case the 'data' fields are set with
	UTF8 contents.

	The following input-mode flags (set with soap_set_imode()) control the
	parsing of C/C++ data types (stored in node and type fields):

	default:	only elements with an 'id' attribute are deserialized
			as C/C++ data types (when a deserializer is available)
	SOAP_XML_TREE:	never deserialize C/C++ types (produces DOM tree)
	SOAP_XML_GRAPH:	always deserialize C/C++ types (when a deserializer is
			available and the xsi:type attribute is present in the
			XML node or the XML element tag matches the type name)

	The following output-mode flag (set with soap_set_omode()) control the
	serialization of XML:

	SOAP_XML_CANONICAL:	serialize XML in canonical form

	The entire deserialized DOM is freed with soap_end();

	Compile:
	soapcpp2 -c dom.h
	gcc -c dom.c

	Add to the application's header file:
	#import "dom.h"

	Then link the application with dom.o

	Development note:
	Reused the gSOAP struct soap id hash table for handling namespace
	bindings when transmitting DOMs

TODO:	Add wide string support for DOM attribute parsing
	Improve mixed content handling
*/

#include "soapH.h"

/* format string for DOM namespace prefixes (<= 16 chars total) */
#define SOAP_DOMID_FORMAT "SOAP-DOM%lu"

/* namespace name (URI) lookup and store routines */
static struct soap_ilist *soap_lookup_ns(struct soap*, const char*);
static struct soap_ilist *soap_enter_ns(struct soap*, const char*, const char*);

/******************************************************************************/
SOAP_FMAC1
void
SOAP_FMAC2
soap_mark_xsd__anyType(struct soap *soap, const struct soap_dom_element *node)
{ if (node)
  { struct soap_dom_element *elt;
    if (node->__type && node->node)
      soap_markelement(soap, node->node, node->__type);
    else if (!node->data && !node->wide)
      for (elt = node->elts; elt; elt = elt->next)
        soap_mark_xsd__anyType(soap, elt);
  }
}

/******************************************************************************/
SOAP_FMAC1
void
SOAP_FMAC2
soap_default_xsd__anyType(struct soap *soap, struct soap_dom_element *node)
{ node->next = NULL;
  node->prnt = NULL;
  node->nstr = NULL;
  node->name = NULL;
  node->data = NULL;
  node->wide = NULL;
  node->elts = NULL;
  node->atts = NULL;
  node->node = NULL;
  node->__type = 0;
}

static int element(struct soap *soap, const char *name)
{ short part = soap->part;
  soap->part = SOAP_IN_ENVELOPE; /* use this to avoid SOAP encoding and literal encoding issues */
  soap_element(soap, name, 0, NULL);
  soap->part = part;
  return soap->error;
}

static int out_element(struct soap *soap, const struct soap_dom_element *node, const char *prefix, const char *name, const char *nstr)
{ if (!prefix)
  { if (node->__type && node->node)
      return soap_putelement(soap, node->node, name, 0, node->__type);
    return element(soap, name);
  }
  if (node->__type && node->node)
  { char *s = (char*)malloc(strlen(prefix) + strlen(name) + 2);
    if (!s)
      return soap->error = SOAP_EOM;
    sprintf(s, "%s:%s", prefix, name);
    soap_putelement(soap, node->node, s, 0, node->__type);
    free(s);
  }
  else if (strlen(prefix) + strlen(name) < sizeof(soap->msgbuf))
  { sprintf(soap->msgbuf, "%s:%s", prefix, name);
    if (element(soap, soap->msgbuf))
      return soap->error;
    if (nstr)
    { sprintf(soap->msgbuf, "xmlns:%s", prefix);
      soap_attribute(soap, soap->msgbuf, nstr);
    }
  }
  else
  { char *s = (char*)malloc(strlen(prefix) + strlen(name) + 2);
    if (!s)
      return soap->error = SOAP_EOM;
    sprintf(s, "%s:%s", prefix, name);
    if (element(soap, s))
      return soap->error;
    if (nstr)
    { sprintf(s, "xmlns:%s", prefix);
      soap_attribute(soap, s, nstr);
    }
    free(s);
  }
  return soap->error;
}

static int out_attribute(struct soap *soap, const char *prefix, const char *name, const char *data)
{ if (!prefix)
    return soap_attribute(soap, name, data);
  if (strlen(prefix) + strlen(name) < sizeof(soap->msgbuf))
  { sprintf(soap->msgbuf, "%s:%s", prefix, name);
    soap_attribute(soap, soap->msgbuf, data);
  }
  else
  { char *s = (char*)malloc(strlen(prefix) + strlen(name) + 2);
    if (!s)
      return soap->error = SOAP_EOM;
    sprintf(s, "%s:%s", prefix, name);
    soap_attribute(soap, s, data);
    free(s);
  }
  return soap->error;
}

SOAP_FMAC1
int
SOAP_FMAC2
soap_out_xsd__anyType(struct soap *soap, const char *tag, int id, const struct soap_dom_element *node, const char *type)
{ if (node)
  { struct soap_dom_element *elt;
    struct soap_dom_attribute *att;
    register struct soap_ilist *p = NULL, *q;
    struct Namespace *ns = NULL;
    const char *prefix;		/* namespace prefix, if namespace is present */
    size_t colon = 0;
    if (node->name)
      tag = node->name;
    if ((prefix = strchr(tag, ':')))
    { colon = prefix - tag + 1;
      if (colon > sizeof(soap->tag))
        colon = sizeof(soap->tag);
    }
    prefix = NULL;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DOM node '%s'\n", tag));
    if (node->nstr)
    { if ((p = soap_lookup_ns(soap, node->nstr)))
      { prefix = p->id;
        p = NULL;
        if (out_element(soap, node, prefix, tag + colon, NULL))
          return soap->error;
      }
      else if (colon)
      { strncpy(soap->tag, tag, colon - 1);
        soap->tag[colon - 1] = '\0';
        if (!(p = soap_enter_ns(soap, soap->tag, node->nstr)))
          return soap->error = SOAP_EOM;
        prefix = p->id;
        if (out_element(soap, node, prefix, tag + colon, node->nstr))
          return soap->error;
      }
      else
      { for (ns = soap->local_namespaces; ns && ns->id; ns++)
          if (ns->ns == node->nstr || !strcmp(ns->ns, node->nstr))
          { if (soap->encodingStyle || ns == soap->local_namespaces)
              prefix = ns->id;
            if (out_element(soap, node, ns->id, tag + colon, NULL))
              return soap->error;
            break;
          }
        if (!ns || !ns->id)
        { sprintf(soap->tag, SOAP_DOMID_FORMAT, soap->idnum++);
          if (!(p = soap_enter_ns(soap, soap->tag, node->nstr)))
            return soap->error = SOAP_EOM;
          prefix = p->id;
          if (out_element(soap, node, prefix, tag + colon, node->nstr))
            return soap->error;
        }
      }
    }
    else if (out_element(soap, node, NULL, tag + colon, NULL))
      return soap->error;
    if (node->node && node->__type)
      return SOAP_OK;
    for (att = node->atts; att; att = att->next)
    { if (att->name)
      { if (att->nstr)
        { if ((att->nstr == node->nstr || (node->nstr && !strcmp(att->nstr, node->nstr))) && prefix)
	  { if (out_attribute(soap, prefix, att->name, att->data))
	      return soap->error;
	  }
	  else if ((q = soap_lookup_ns(soap, att->nstr)))
	  { if (out_attribute(soap, q->id, att->name, att->data))
	      return soap->error;
	  }
	  else
	  { for (ns = soap->local_namespaces; ns && ns->id; ns++)
            { if (ns->ns == att->nstr || !strcmp(ns->ns, att->nstr))
	      { if (out_attribute(soap, ns->id, att->name, att->data))
	          return soap->error;
	        break;
	      }
	    }
	    if (!ns || !ns->id)
            { sprintf(soap->msgbuf, "xmlns:"SOAP_DOMID_FORMAT, soap->idnum++);
	      if (soap_attribute(soap, soap->msgbuf, att->nstr))
	        return soap->error;
	      strcat(soap->msgbuf, ":");
	      strcat(soap->msgbuf, att->name);
	      if (soap_attribute(soap, soap->msgbuf + 6, att->data))
	        return soap->error;
            }
          }
        }
	else if (soap_attribute(soap, att->name, att->data))
          return soap->error;
      }
    }
    if (soap_element_start_end_out(soap, NULL))
      return soap->error;
    if (node->data || node->wide || node->elts)
    { if (node->data)
      { if (soap_string_out(soap, node->data, 0))
          return soap->error;
      }
      else if (node->wide)
      { if (soap_wstring_out(soap, node->wide, 0))
          return soap->error;
      }
      else
      { for (elt = node->elts; elt; elt = elt->next)
          if (soap_out_xsd__anyType(soap, tag, 0, elt, NULL))
            return soap->error;
      }
    }
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "End of DOM node '%s'\n", tag + colon));
    if (soap_send_raw(soap, "</", 2))
      return soap->error;
    if (prefix)
      if (soap_send(soap, prefix) || soap_send_raw(soap, ":", 1))
        return soap->error;
    if (soap_send(soap, tag + colon) || soap_send_raw(soap, ">", 1))
      return soap->error;
    if (p)
      p->level = 0; /* xmlns binding is out of scope */
  }
  return SOAP_OK;
}

SOAP_FMAC1
struct soap_dom_element *
SOAP_FMAC2
soap_in_xsd__anyType(struct soap *soap, const char *tag, struct soap_dom_element *node, const char *type)
{ register struct soap_attribute *tp;
  register struct soap_dom_attribute **att;
  register struct soap_nlist *np;
  register char *s;
  if (soap_peek_element(soap))
    return NULL;
  if (!node)
    if (!(node = (struct soap_dom_element*)soap_malloc(soap, sizeof(struct soap_dom_element))))
    { soap->error = SOAP_EOM;
      return NULL;
    }
  node->next = NULL;
  node->prnt = NULL;
  node->nstr = NULL;
  node->name = NULL;
  node->data = NULL;
  node->wide = NULL;
  node->elts = NULL;
  node->atts = NULL;
  node->node = NULL;
  node->__type = 0;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DOM node %s\n", soap->tag));
  np = soap->nlist;
  if (!(s = strchr(soap->tag, ':')))
  { while (np && *np->id) /* find default namespace, if present */
      np = np->next;
    s = soap->tag;
  }
  else
  { while (np && (strncmp(np->id, soap->tag, s - soap->tag) || np->id[s - soap->tag]))
      np = np->next;
    s++;
    if (!np)
    { soap->error = SOAP_NAMESPACE;
      return NULL;
    }
  }
  if (np)
  { if (np->index >= 0)
      node->nstr = soap->namespaces[np->index].ns;
    else if (np->ns)
      node->nstr = soap_strdup(soap, np->ns);
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DOM node namespace='%s'\n", node->nstr?node->nstr:""));
  }
  node->name = soap_strdup(soap, soap->tag);
  if ((soap->mode & SOAP_XML_GRAPH) || (!(soap->mode & SOAP_XML_TREE) && *soap->id))
  { if ((node->node = soap_getelement(soap, &node->__type)))
      return node;
    if ((soap->mode & SOAP_XML_GRAPH) && !*soap->type)
    { strcpy(soap->type, soap->tag);
      if ((node->node = soap_getelement(soap, &node->__type)))
        return node;
    }
  }
  att = &node->atts;
  for (tp = soap->attributes; tp; tp = tp->next)
    if (tp->visible)
    { np = soap->nlist;
      if (!(s = strchr(tp->name, ':')))
      { while (np && *np->id) /* find default namespace, if present */
          np = np->next;
        s = tp->name;
      }
      else
      { while (np && (strncmp(np->id, tp->name, s - tp->name) || np->id[s - tp->name]))
          np = np->next;
        s++;
        if (!np)
        { soap->error = SOAP_NAMESPACE;
          return NULL;
        }
      }
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DOM node attribute='%s'\n", tp->name));
      *att = (struct soap_dom_attribute*)soap_malloc(soap, sizeof(struct soap_dom_attribute));
      if (!*att)
      { soap->error = SOAP_EOM;
        return NULL;
      }
      (*att)->next = NULL;
      (*att)->nstr = NULL;
      if (np)
      { if (np->index >= 0)
          (*att)->nstr = soap->namespaces[np->index].ns;
        else if (np->ns)
	  (*att)->nstr = soap_strdup(soap, np->ns);
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DOM attribute namespace='%s'\n", (*att)->nstr?(*att)->nstr:""));
      }
      (*att)->name = soap_strdup(soap, s);
      if (tp->visible == 2)
        (*att)->data = soap_strdup(soap, tp->value);
      else
        (*att)->data = NULL;
      (*att)->wide = NULL;
      att = &(*att)->next;
      tp->visible = 0;
    }
  soap_element_begin_in(soap, NULL);
  if (soap->body)
  { wchar c;
    do c = soap_getchar(soap);
    while (c > 0 && c <= 32);
    if (c == EOF)
    { soap->error = SOAP_EOF;
      return NULL;
    }
    soap_unget(soap, c);
    if (c == '<')
    { struct soap_dom_element **elt;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DOM node '%s' has subelements\n", node->name));
      elt = &node->elts;
      for (;;)
      { if (!(*elt = soap_in_xsd__anyType(soap, NULL, NULL, NULL)))
        { if (soap->error == SOAP_NO_TAG)
            soap->error = SOAP_OK;
          else
            return NULL;
          break;
        }
	(*elt)->prnt = node;
        elt = &(*elt)->next;
      }
    }
    else
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DOM node '%s' has cdata\n", node->name));
      if ((soap->mode & SOAP_C_UTFSTRING))
      { if (!(node->data = soap_string_in(soap, 1)))
          return NULL;
      }
      else if (!(node->wide = soap_wstring_in(soap, 1)))
        return NULL;
    }
    if (soap_element_end_in(soap, node->name))
      return NULL;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "End of DOM node '%s'\n", node->name));
  }
  return node;
}

////////////////////////////////////////////////////////////////////////////////
//
//	DOM attribute custom (de)serializers (to enable linkage)
//
////////////////////////////////////////////////////////////////////////////////

SOAP_FMAC1
void
SOAP_FMAC2
soap_mark_xsd__anyTypeAttribute(struct soap *soap, const struct soap_dom_attribute *a)
{ }

SOAP_FMAC1
struct soap_dom_attribute *
SOAP_FMAC2
soap_in_xsd__anyTypeAttribute(struct soap *soap, char const *tag, struct soap_dom_attribute *a, char const *type)
{ return NULL;
}

SOAP_FMAC1
int
SOAP_FMAC2
soap_out_xsd__anyTypeAttribute(struct soap *soap, char const *tag, int id, struct soap_dom_attribute *a, char const *type)
{ return SOAP_EOF;
}

////////////////////////////////////////////////////////////////////////////////
//
//	Namespace lookup/store routines
//
////////////////////////////////////////////////////////////////////////////////

static struct soap_ilist *
soap_lookup_ns(struct soap *soap, const char *nstr)
{ register struct soap_ilist *ip;
  for (ip = soap->iht[soap_hash(nstr)]; ip; ip = ip->next)
    if (!strcmp((char*)ip->ptr, nstr) && ip->level)
      return ip;
  return NULL;
}

static struct soap_ilist *
soap_enter_ns(struct soap *soap, const char *prefix, const char *nstr)
{ int h;
  register struct soap_ilist *ip;
  for (ip = soap->iht[soap_hash(nstr)]; ip; ip = ip->next)
    if (!strcmp((char*)ip->ptr, nstr) && !ip->level)
    { strcpy(ip->id, prefix);
      ip->level = 1;
      return ip;
    }
  ip = (struct soap_ilist*)malloc(sizeof(struct soap_ilist) + strlen(nstr) + SOAP_TAGLEN);
  if (ip)
  { h = soap_hash(nstr);
    strcpy(ip->id, prefix);
    ip->ptr = ip->id + SOAP_TAGLEN;
    strcpy((char*)ip->ptr, nstr);
    ip->next = soap->iht[h];
    soap->iht[h] = ip;
    ip->clist = NULL;
    ip->level = 1;
    return ip;
  }
  return NULL;
}

