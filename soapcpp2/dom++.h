/*	dom++.h

	gSOAP XML DOM extensions

	Copyright (C) 2000-2003 Robert A. van Engelen, Genivia inc.
	All Rights Reserved.
*/

extern class soap_dom_element;		// forward declaration
extern class soap_dom_attribute;	// forward declaration
extern class soap_dom_iterator;		// forward declaration

extern class soap_dom_element		// DOM element node
{ public:
  typedef soap_dom_iterator iterator;
  soap_dom_element *next;		// next element (sibling) in sequence (not used at the document root)
  soap_dom_element *prnt;		// parent node
  const char *nstr;			// optional namespace name string (URI)
  char *name;				// element name with optional prefix
  char *data;				// optional element CDATA value (output only, not filled by parser unless SOAP_C_UTFSTRING is set)
  wchar_t *wide;			// optional element CDATA value (wide char string, filled by parser unless SOAP_C_UTFSTRING is set)
  int __type;				// optional type of data pointed to (SOAP_TYPE_X)
  void *node;				// optional pointer to serializable data node
  soap_dom_element *elts;		// optional element children (data must be NULL)
  soap_dom_attribute *atts;		// optional element attributes
  struct soap *soap;			// gSOAP soap struct that manages this instance
  soap_dom_element();
  soap_dom_element(struct soap *soap);
  soap_dom_element(struct soap *soap, const char *nstr, const char *name);
  soap_dom_element(struct soap *soap, const char *nstr, const char *name, const char *data);
  soap_dom_element(struct soap *soap, const char *nstr, const char *name, void *node, int type);
  ~soap_dom_element();
  soap_dom_element &set(const char *nstr, const char *name);	// set namespace and name
  soap_dom_element &set(const char *data);		// set data
  soap_dom_element &set(void *node, int type);	// set node
  soap_dom_element &add(soap_dom_element *elt);	// add child element
  soap_dom_element &add(soap_dom_element &elt);	// add child element
  soap_dom_element &add(soap_dom_attribute *att);	// add attribute
  soap_dom_element &add(soap_dom_attribute &att);	// add attribute
  soap_dom_iterator begin();
  soap_dom_iterator end();
  soap_dom_iterator find(const char *nstr, const char *name);
  soap_dom_iterator find(int type);
  void unlink();
};

extern class soap_dom_attribute		// DOM attribute node
{ public:
  soap_dom_attribute *next;		// next attribute in sequence
  const char *nstr;			// optional attribute namespace name string (URI)
  char *name;				// attribute name
  char *data;				// optional attribute CDATA value (output only, not filled by parser unless SOAP_C_UTFSTRING is set)
  wchar_t *wide;			// optional attribute CDATA value (wide char string, filled by parser unless SOAP_C_UTFSTRING is set)
  struct soap *soap;			// gSOAP soap struct that manages this instance
  soap_dom_attribute();
  soap_dom_attribute(struct soap *soap);
  soap_dom_attribute(struct soap *soap, const char *nstr, const char *name, const char *data);
  ~soap_dom_attribute();
  soap_dom_attribute &set(const char *nstr, const char *name);	// set namespace and name
  soap_dom_attribute &set(const char *data);		// set data
  void unlink();
};

extern class soap_dom_iterator
{ public:
  soap_dom_element *elt;
  const char *nstr;
  const char *name;
  int type;
  soap_dom_iterator();
  soap_dom_iterator(soap_dom_element *elt);
  ~soap_dom_iterator();
  bool operator==(const soap_dom_iterator &iter) const;
  bool operator!=(const soap_dom_iterator &iter) const;
  soap_dom_element &operator*() const;
  soap_dom_iterator &operator++();
};

// custom (de)serializers:
extern typedef soap_dom_element xsd__anyType;
extern typedef soap_dom_attribute xsd__anyTypeAttribute;	// artificial DOM attribute type (to generate code)

extern class std::ostream;
extern class std::istream;
extern std::ostream &operator<<(std::ostream &o, const soap_dom_element &e);
extern std::istream &operator>>(std::istream &i, soap_dom_element &e);
