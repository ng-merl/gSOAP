#include "wsdlH.h"		// cannot include "schemaH.h"

using namespace std;

extern const char *qname_token(const char*, const char*);

////////////////////////////////////////////////////////////////////////////////
//
//	schema
//
////////////////////////////////////////////////////////////////////////////////

xs__schema::xs__schema()
{ soap = NULL;
}

xs__schema::xs__schema(struct soap *soap)
{ soap_default(soap);
  soap_imode(soap, SOAP_XML_TREE | SOAP_C_UTFSTRING);
  soap_omode(soap, SOAP_XML_TREE | SOAP_C_UTFSTRING);
  soap->encodingStyle = NULL;
}

xs__schema::xs__schema(const char *location)
{ soap = soap_new1(SOAP_XML_TREE | SOAP_C_UTFSTRING);
  soap->encodingStyle = NULL;
  if (location)
  { if (soap_connect(soap, location, NULL))
      cerr << "Cannot connect to " << location << " to retrieve schema" << endl;
    else
    { soap_begin_recv(soap);
      this->soap_in(soap, "schema", NULL);
      if (soap->error)
        cerr << "An error occurred while parsing schema from " << location << endl;
      soap_end_recv(soap);
      soap_closesock(soap);
    }
  }
}

xs__schema::~xs__schema()
{ }

int xs__schema::traverse()
{ cerr << "schema " << (targetNamespace?targetNamespace:"") << endl;
  // process include
  for (vector<xs__include>::iterator i = include.begin(); i != include.end(); ++i)
  { (*i).traverse(*this);
    const xs__schema *s = (*i).schemaPtr();
    if (s)
    { include.insert<vector<xs__include>::const_iterator>(include.end(), s->include.begin(), s->include.end());
      import.insert<vector<xs__import>::const_iterator>(import.end(), s->import.begin(), s->import.end());
      attribute.insert<vector<xs__attribute>::const_iterator>(attribute.end(), s->attribute.begin(), s->attribute.end());
      element.insert<vector<xs__element>::const_iterator>(element.end(), s->element.begin(), s->element.end());
      group.insert<vector<xs__group>::const_iterator>(group.end(), s->group.begin(), s->group.end());
      simpleType.insert<vector<xs__simpleType>::const_iterator>(simpleType.end(), s->simpleType.begin(), s->simpleType.end());
      complexType.insert<vector<xs__complexType>::const_iterator>(complexType.end(), s->complexType.begin(), s->complexType.end());
    }
  }
  // process import
  for (vector<xs__import>::iterator i = import.begin(); i != import.end(); ++i)
    (*i).traverse(*this);
  // process attributes
  for (vector<xs__attribute>::iterator i = attribute.begin(); i != attribute.end(); ++i)
    (*i).traverse(*this);
  // process elements
  for (vector<xs__element>::iterator i = element.begin(); i != element.end(); ++i)
    (*i).traverse(*this);
  // process groups
  for (vector<xs__group>::iterator i = group.begin(); i != group.end(); ++i)
    (*i).traverse(*this);
  // process simpleTypes
  for (vector<xs__simpleType>::iterator i = simpleType.begin(); i != simpleType.end(); ++i)
    (*i).traverse(*this);
  // process complexTypes
  for (vector<xs__complexType>::iterator i = complexType.begin(); i != complexType.end(); ++i)
    (*i).traverse(*this);
  return SOAP_OK;
}

int xs__schema::error()
{ return soap->error;
}

void xs__schema::print_fault()
{ soap_print_fault(soap, stderr);
  soap_print_fault_location(soap, stderr);
}

int xs__include::traverse(const xs__schema &schema)
{ cerr << "schema include" << endl;
  if (!schemaRef)
    if (schemaLocation)
      schemaRef = new xs__schema(schemaLocation);
  return SOAP_OK;
}

void xs__include::schemaPtr(const xs__schema *schema)
{ schemaRef = schema;
}

const xs__schema *xs__include::schemaPtr() const
{ return schemaRef;
}

int xs__import::traverse(const xs__schema &schema)
{ cerr << "schema import" << endl;
  if (!schemaRef)
  { struct Namespace *p = namespaces;
    const char *s = NULL;
    if (schemaLocation)
      s = schemaLocation;
    else
      s = namespace_;
    if (s)
    { if (p)
      { for (; p->id; p++)
        { if (p->ns)
            if (!soap_tag_cmp(s, p->ns))
              break;
        }
      }
      if (!p->id) // don't import any of the schemas in the .nsmap table
        schemaRef = new xs__schema(s);
    }
  }
  return SOAP_OK;
}

void xs__import::schemaPtr(const xs__schema *schema)
{ schemaRef = schema;
}

const xs__schema *xs__import::schemaPtr() const
{ return schemaRef;
}

int xs__attribute::traverse(const xs__schema &schema)
{ cerr << "schema attribute" << endl;
  const char *token = qname_token(ref, schema.targetNamespace);
  attributeRef = NULL;
  if (token)
  { for (vector<xs__attribute>::const_iterator i = schema.attribute.begin(); i != schema.attribute.end(); ++i)
      if (!strcmp((*i).name, token))
      { attributeRef = &(*i);
        cerr << "Found attribute " << (name?name:"") << " ref " << (token?token:"") << endl;
        break;
      }
  }
  else
  { for (vector<xs__import>::const_iterator i = schema.import.begin(); i != schema.import.end(); ++i)
    { const xs__schema *s = (*i).schemaPtr();
      if (s)
      { token = qname_token(ref, s->targetNamespace);
        if (token)
        { for (vector<xs__attribute>::const_iterator j = s->attribute.begin(); j != s->attribute.end(); ++j)
            if (!strcmp((*j).name, token))
            { attributeRef = &(*j);
              cerr << "Found attribute " << (name?name:"") << " ref " << (token?token:"") << endl;
              break;
            }
          break;
        }
      }
    }
  }
  if (!attributeRef)
    cerr << "Not found: attribute " << (name?name:"") << " ref " << (token?token:"") << endl;
  token = qname_token(type, schema.targetNamespace);
  simpleTypeRef = NULL;
  if (token)
  { for (vector<xs__simpleType>::const_iterator i = schema.simpleType.begin(); i != schema.simpleType.end(); ++i)
      if (!strcmp((*i).name, token))
      { simpleTypeRef = &(*i);
        cerr << "Found attribute " << (name?name:"") << " type " << (token?token:"") << endl;
        break;
      }
  }
  else
  { for (vector<xs__import>::const_iterator i = schema.import.begin(); i != schema.import.end(); ++i)
    { const xs__schema *s = (*i).schemaPtr();
      if (s)
      { token = qname_token(ref, s->targetNamespace);
        if (token)
        { for (vector<xs__simpleType>::const_iterator j = s->simpleType.begin(); j != s->simpleType.end(); ++j)
            if (!strcmp((*j).name, token))
            { simpleTypeRef = &(*j);
              cerr << "Found attribute " << (name?name:"") << " simpleType " << (token?token:"") << endl;
              break;
            }
          break;
        }
      }
    }
  }
  if (!simpleTypeRef)
    cerr << "Not found: attribute " << (name?name:"") << " type " << (token?token:"") << endl;
  return SOAP_OK;
}

void xs__attribute::attributePtr(const xs__attribute *attribute)
{ attributeRef = attribute;
}

void xs__attribute::simpleTypePtr(const xs__simpleType *simpleType)
{ simpleTypeRef = simpleType;
}

const xs__attribute *xs__attribute::attributePtr() const
{ return attributeRef;
}

const xs__simpleType *xs__attribute::simpleTypePtr() const
{ return simpleTypeRef;
}

int xs__element::traverse(const xs__schema &schema)
{ cerr << "schema element" << endl;
  const char *token = qname_token(ref, schema.targetNamespace);
  elementRef = NULL;
  if (token)
  { for (vector<xs__element>::const_iterator i = schema.element.begin(); i != schema.element.end(); ++i)
      if (!strcmp((*i).name, token))
      { elementRef = &(*i);
        cerr << "Found element " << (name?name:"") << " ref " << (token?token:"") << endl;
        break;
      }
  }
  else
  { for (vector<xs__import>::const_iterator i = schema.import.begin(); i != schema.import.end(); ++i)
    { const xs__schema *s = (*i).schemaPtr();
      if (s)
      { token = qname_token(ref, s->targetNamespace);
        if (token)
        { for (vector<xs__element>::const_iterator j = s->element.begin(); j != s->element.end(); ++j)
            if (!strcmp((*j).name, token))
            { elementRef = &(*j);
              cerr << "Found element " << (name?name:"") << " ref " << (token?token:"") << endl;
              break;
            }
          break;
        }
      }
    }
  }
  if (!elementRef)
    cerr << "Not found: element " << (name?name:"") << " ref " << (token?token:"") << endl;
  token = qname_token(type, schema.targetNamespace);
  simpleTypeRef = NULL;
  if (token)
  { for (vector<xs__simpleType>::const_iterator i = schema.simpleType.begin(); i != schema.simpleType.end(); ++i)
      if (!strcmp((*i).name, token))
      { simpleTypeRef = &(*i);
        cerr << "Found element " << (name?name:"") << " simpleType " << (token?token:"") << endl;
        break;
      }
  }
  else
  { for (vector<xs__import>::const_iterator i = schema.import.begin(); i != schema.import.end(); ++i)
    { const xs__schema *s = (*i).schemaPtr();
      if (s)
      { token = qname_token(ref, s->targetNamespace);
        if (token)
        { for (vector<xs__simpleType>::const_iterator j = s->simpleType.begin(); j != s->simpleType.end(); ++j)
            if (!strcmp((*j).name, token))
            { simpleTypeRef = &(*j);
              cerr << "Found element " << (name?name:"") << " simpleType " << (token?token:"") << endl;
              break;
            }
          break;
        }
      }
    }
  }
  if (!simpleTypeRef)
    cerr << "Not found: element " << (name?name:"") << " simpleType " << (token?token:"") << endl;
  token = qname_token(type, schema.targetNamespace);
  complexTypeRef = NULL;
  if (token)
  { for (vector<xs__complexType>::const_iterator i = schema.complexType.begin(); i != schema.complexType.end(); ++i)
      if (!strcmp((*i).name, token))
      { complexTypeRef = &(*i);
        cerr << "Found element " << (name?name:"") << " complexType " << (token?token:"") << endl;
        break;
      }
  }
  else
  { for (vector<xs__import>::const_iterator i = schema.import.begin(); i != schema.import.end(); ++i)
    { const xs__schema *s = (*i).schemaPtr();
      if (s)
      { token = qname_token(ref, s->targetNamespace);
        if (token)
        { for (vector<xs__complexType>::const_iterator j = s->complexType.begin(); j != s->complexType.end(); ++j)
            if (!strcmp((*j).name, token))
            { complexTypeRef = &(*j);
              cerr << "Found element " << (name?name:"") << " complexType " << (token?token:"") << endl;
              break;
            }
          break;
        }
      }
    }
  }
  if (!complexTypeRef)
    cerr << "Not found: element " << (name?name:"") << " complexType " << (token?token:"") << endl;
  return SOAP_OK;
}

void xs__element::elementPtr(const xs__element *element)
{ elementRef = element;
}

void xs__element::simpleTypePtr(const xs__simpleType *simpleType)
{ simpleTypeRef = simpleType;
}

void xs__element::complexTypePtr(const xs__complexType *complexType)
{ complexTypeRef = complexType;
}

const xs__element *xs__element::elementPtr() const
{ return elementRef;
}

const xs__simpleType *xs__element::simpleTypePtr() const
{ return simpleTypeRef;
}

const xs__complexType *xs__element::complexTypePtr() const
{ return complexTypeRef;
}

int xs__simpleType::traverse(const xs__schema &schema)
{ cerr << "schema simpleType" << endl;
  if (list)
    list->traverse(schema);
  else if (restriction)
    restriction->traverse(schema);
  else if (union_)
    union_->traverse(schema);
  return SOAP_OK;
}

int xs__complexType::traverse(const xs__schema &schema)
{ cerr << "schema complexType" << endl;
  if (simpleContent)
    simpleContent->traverse(schema);
  else if (complexContent)
    complexContent->traverse(schema);
  else if (all)
    all->traverse(schema);
  else if (choice)
    choice->traverse(schema);
  else if (sequence)
    sequence->traverse(schema);
  else if (any)
    any->traverse(schema);
  return SOAP_OK;
}

int xs__simpleContent::traverse(const xs__schema &schema)
{ cerr << "schema simpleContent" << endl;
  if (extension)
    extension->traverse(schema);
  else if (restriction)
    restriction->traverse(schema);
  return SOAP_OK;
}

int xs__complexContent::traverse(const xs__schema &schema)
{ cerr << "schema complexContent" << endl;
  if (extension)
    extension->traverse(schema);
  else if (restriction)
    restriction->traverse(schema);
  return SOAP_OK;
}

int xs__extension::traverse(const xs__schema &schema)
{ cerr << "schema extension" << endl;
  if (group)
    group->traverse(schema);
  else if (all)
    all->traverse(schema);
  else if (choice)
    choice->traverse(schema);
  else if (sequence)
    sequence->traverse(schema);
  return SOAP_OK;
}

int xs__restriction::traverse(const xs__schema &schema)
{ cerr << "schema restriction" << endl;
  if (group)
    group->traverse(schema);
  else if (all)
    all->traverse(schema);
  else if (choice)
    choice->traverse(schema);
  else if (sequence)
    sequence->traverse(schema);
  else
    for (vector<xs__enumeration>::iterator i = enumeration.begin(); i != enumeration.end(); ++i)
      (*i).traverse(schema);
  return SOAP_OK;
}

int xs__list::traverse(const xs__schema &schema)
{ cerr << "schema list" << endl;
}

int xs__union::traverse(const xs__schema &schema)
{ cerr << "schema union" << endl;
}

int xs__all::traverse(const xs__schema &schema)
{ cerr << "schema all" << endl;
  for (vector<xs__element>::iterator i = element.begin(); i != element.end(); ++i)
    (*i).traverse(schema);
  return SOAP_OK;
}

int xs__choice::traverse(const xs__schema &schema)
{ cerr << "schema choice" << endl;
  for (vector<xs__element>::iterator i = element.begin(); i != element.end(); ++i)
    (*i).traverse(schema);
  for (vector<xs__group>::iterator i = group.begin(); i != group.end(); ++i)
    (*i).traverse(schema);
  //for (vector<xs__choice>::iterator i = choice.begin(); i != choice.end(); ++i)
    //(*i).traverse(schema);
  //for (vector<xs__sequence>::iterator i = sequence.begin(); i != sequence.end(); ++i)
    //(*i).traverse(schema);
  for (vector<xs__any>::iterator i = any.begin(); i != any.end(); ++i)
    (*i).traverse(schema);
  return SOAP_OK;
}

int xs__sequence::traverse(const xs__schema &schema)
{ cerr << "schema sequence" << endl;
  for (vector<xs__element>::iterator i = element.begin(); i != element.end(); ++i)
    (*i).traverse(schema);
  for (vector<xs__group>::iterator i = group.begin(); i != group.end(); ++i)
    (*i).traverse(schema);
  for (vector<xs__choice>::iterator i = choice.begin(); i != choice.end(); ++i)
    (*i).traverse(schema);
  //for (vector<xs__sequence>::iterator i = sequence.begin(); i != sequence.end(); ++i)
   // (*i).traverse(schema);
  for (vector<xs__any>::iterator i = any.begin(); i != any.end(); ++i)
    (*i).traverse(schema);
  return SOAP_OK;
}

int xs__any::traverse(const xs__schema &schema)
{ cerr << "schema any" << endl;
  for (vector<xs__element>::iterator i = element.begin(); i != element.end(); ++i)
    (*i).traverse(schema);
  return SOAP_OK;
}

int xs__group::traverse(const xs__schema &schema)
{ cerr << "schema group" << endl;
  if (all)
    all->traverse(schema);
  else if (choice)
    choice->traverse(schema);
  else if (sequence)
    sequence->traverse(schema);
  const char *token = qname_token(ref, schema.targetNamespace);
  groupRef = NULL;
  if (token)
  { for (vector<xs__group>::const_iterator i = schema.group.begin(); i != schema.group.end(); ++i)
      if (!strcmp((*i).name, token))
      { groupRef = &(*i);
        cerr << "Found group " << (name?name:"") << " ref " << (token?token:"") << endl;
        break;
      }
  }
  else
  { for (vector<xs__import>::const_iterator i = schema.import.begin(); i != schema.import.end(); ++i)
    { const xs__schema *s = (*i).schemaPtr();
      if (s)
      { token = qname_token(ref, s->targetNamespace);
        if (token)
        { for (vector<xs__group>::const_iterator j = s->group.begin(); j != s->group.end(); ++j)
            if (!strcmp((*j).name, token))
            { groupRef = &(*j);
              cerr << "Found group " << (name?name:"") << " ref " << (token?token:"") << endl;
              break;
            }
          break;
        }
      }
    }
  }
  if (!groupRef)
    cerr << "Not found: group " << (name?name:"") << " ref " << (token?token:"") << endl;
  return SOAP_OK;
}

void xs__group::groupPtr(const xs__group *group)
{ groupRef = group;
}

const xs__group* xs__group::groupPtr() const
{ return groupRef;
}

int xs__enumeration::traverse(const xs__schema &schema)
{ cerr << "schema enumeration" << endl;
  return SOAP_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
//	I/O
//
////////////////////////////////////////////////////////////////////////////////

ostream &operator<<(ostream &o, const xs__schema &e)
{ if (!e.soap)
  { struct soap soap;
    soap_init2(&soap, SOAP_IO_DEFAULT, SOAP_XML_TREE | SOAP_C_UTFSTRING);
    e.soap_serialize(&soap);
    soap_begin_send(&soap);
    e.soap_out(&soap, "schema", 0, NULL);
    soap_end_send(&soap);
    soap_end(&soap);
    soap_done(&soap);
  }
  else
  { ostream *os = e.soap->os;
    e.soap->os = &o;
    e.soap_serialize(e.soap);
    soap_begin_send(e.soap);
    e.soap_out(e.soap, "schema", 0, NULL);
    soap_end_send(e.soap);
    e.soap->os = os;
  }
  return o;
}

istream &operator>>(istream &i, xs__schema &e)
{ if (!e.soap)
    e.soap = soap_new();
  istream *is = e.soap->is;
  e.soap->is = &i;
  if (soap_begin_recv(e.soap)
   || !e.soap_in(e.soap, "schema", NULL)
   || soap_end_recv(e.soap))
    ; // handle error? Note: e.soap->error is set and app should check
  e.soap->is = is;
  return i;
}


