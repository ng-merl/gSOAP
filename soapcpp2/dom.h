/*	dom.h

	gSOAP XML DOM extensions

	Copyright (C) 2000-2003 Robert A. van Engelen, Genivia inc.
	All Rights Reserved.
*/

extern struct soap_dom_element		// DOM element node
{ struct soap_dom_element *next;	// next element (sibling) in sequence (not used at the document root)
  struct soap_dom_element *prnt;	// parent node
  const char *nstr;			// optional namespace name string (URI)
  char *name;				// element name with optional prefix
  char *data;				// optional element CDATA value (output only, not filled by parser unless SOAP_C_UTFSTRING is set)
  wchar_t *wide;			// optional element CDATA value (wide char string, filled by parser unless SOAP_C_UTFSTRING is set)
  int __type;				// optional the type of data pointed to (SOAP_TYPE_X)
  void *node;				// and optional pointer to serializable data node
  struct soap_dom_element *elts;	// optional element children (data must be NULL)
  struct soap_dom_attribute *atts;	// optional element attributes
};

extern struct soap_dom_attribute	// DOM attribute node
{ struct soap_dom_attribute *next;	// next attribute in sequence
  const char *nstr;			// optional attribute namespace name string (URI)
  char *name;				// attribute name
  char *data;				// optional attribute CDATA value (output only, not filled by parser unless SOAP_C_UTFSTRING is set)
  wchar_t *wide;			// optional attribute CDATA value (wide char string, filled by parser unless SOAP_C_UTFSTRING is set)
};

extern typedef struct soap_dom_element xsd__anyType;
