
#include "wsdlH.h"

using namespace std;

const char *qname_token(const char *QName, const char *URI)
{ if (QName && URI && *QName == '"') // QNames are stored in the format "URI":name, unless the URI is in the nsmap
  { int n = strlen(URI);
    if (!strncmp(QName + 1, URI, n) && QName[n + 1] == '"')
      return QName + n + 3;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//
//	wsdl
//
////////////////////////////////////////////////////////////////////////////////

static int warn_ignore(struct soap*, const char*);

wsdl__definitions::wsdl__definitions()
{ soap = NULL;
}

wsdl__definitions::wsdl__definitions(struct soap *soap)
{ soap_default(soap);
  soap_imode(soap, SOAP_XML_TREE | SOAP_C_UTFSTRING);
  soap_omode(soap, SOAP_XML_TREE | SOAP_C_UTFSTRING);
  soap->fignore = warn_ignore;
  soap->encodingStyle = NULL;
}

wsdl__definitions::wsdl__definitions(const char *location)
{ soap = soap_new1(SOAP_XML_TREE | SOAP_C_UTFSTRING);
  soap->encodingStyle = NULL;
  if (location)
  { if (soap_connect(soap, location, NULL))
      cerr << "Cannot connect to " << location << " to retrieve WSDL" << endl;
    else
    { soap_begin_recv(soap);
      this->soap_in(soap, "definitions", NULL);
      if (soap->error)
        cerr << "An error occurred while parsing WSDL from " << location << endl;
      soap_end_recv(soap);
      soap_closesock(soap);
    }
  }
}

wsdl__definitions::~wsdl__definitions()
{ }

int wsdl__definitions::get(struct soap *soap)
{ return traverse();
}

int wsdl__definitions::traverse()
{ cerr << "wsdl definitions " << (name?name:"") << endl;
  // process import first
  for (vector<wsdl__import>::iterator i = import.begin(); i != import.end(); ++i)
    (*i).traverse(*this);
  // then process the types
  if (types)
    types->traverse(*this);
  // process services
  for (vector<wsdl__service>::iterator i = service.begin(); i != service.end(); ++i)
    (*i).traverse(*this);
  // process bindings
  for (vector<wsdl__binding>::iterator i = binding.begin(); i != binding.end(); ++i)
    (*i).traverse(*this);
  // process portTypes
  for (vector<wsdl__portType>::iterator i = portType.begin(); i != portType.end(); ++i)
    (*i).traverse(*this);
  // process messages
  for (vector<wsdl__message>::iterator i = message.begin(); i != message.end(); ++i)
    (*i).traverse(*this);
  return SOAP_OK;
}

int wsdl__definitions::error()
{ return soap->error;
}

void wsdl__definitions::print_fault()
{ soap_print_fault(soap, stderr);
  soap_print_fault_location(soap, stderr);
}

int wsdl__service::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl service " << (name?name:"") << endl;
  // process ports
  for (vector<wsdl__port>::iterator i = port.begin(); i != port.end(); ++i)
    (*i).traverse(definitions);
  return SOAP_OK;
}

int wsdl__port::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl port" << endl;
  // search binding name
  const char *token = qname_token(binding, definitions.targetNamespace);
  bindingRef = NULL;
  if (token)
    for (vector<wsdl__binding>::const_iterator i = definitions.binding.begin(); i != definitions.binding.end(); ++i)
      if (!strcmp((*i).name, token))
      { bindingRef = &(*i);
        cerr << "Found port " << (name?name:"") << " binding " << (token?token:"") << endl;
        break;
      }
  if (!bindingRef)
    cerr << "Not found: port " << (name?name:"") << " binding " << (token?token:"") << endl;
  return SOAP_OK;
}

void wsdl__port::bindingPtr(const wsdl__binding *binding)
{ bindingRef = binding;
}

const wsdl__binding *wsdl__port::bindingPtr() const
{ return bindingRef;
}

int wsdl__binding::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl binding" << endl;
  const char *token = qname_token(type, definitions.targetNamespace);
  portTypeRef = NULL;
  if (token)
  { for (vector<wsdl__portType>::const_iterator i = definitions.portType.begin(); i != definitions.portType.end(); ++i)
    { if (!strcmp((*i).name, token))
      { portTypeRef = &(*i);
        cerr << "Found binding " << (name?name:"") << " portType " << (token?token:"") << endl;
        break;
      }
    }
  }
  if (!portTypeRef)
    cerr << "Not found: binding " << (name?name:"") << " portType " << (token?token:"") << endl;
  for (vector<wsdl__binding_operation>::iterator i = operation.begin(); i != operation.end(); ++i)
    (*i).traverse(definitions, portTypeRef);
  return SOAP_OK;
}

void wsdl__binding::portTypePtr(const wsdl__portType *portType)
{ portTypeRef = portType;
}

const wsdl__portType *wsdl__binding::portTypePtr() const
{ return portTypeRef;
}

int wsdl__binding_operation::traverse(const wsdl__definitions& definitions, const wsdl__portType *portTypeRef)
{ cerr << "wsdl binding operation" << endl;
  if (input)
    input->traverse(definitions);
  if (output)
    output->traverse(definitions);
  for (vector<wsdl__ext_fault>::iterator i = fault.begin(); i != fault.end(); ++i)
    (*i).traverse(definitions);
  operationRef = NULL;
  if (portTypeRef)
    for (vector<wsdl__operation>::const_iterator i = portTypeRef->operation.begin(); i != portTypeRef->operation.end(); ++i)
      if (!strcmp((*i).name, name))
      { operationRef = &(*i);
        cerr << "Found operation " << (name?name:"") << endl;
        break;
      }
  if (!operationRef)
    cerr << "Not found: operation " << (name?name:"") << endl;
  return SOAP_OK;
}

void wsdl__binding_operation::operationPtr(const wsdl__operation *operation)
{ operationRef = operation;
}

const wsdl__operation *wsdl__binding_operation::operationPtr() const
{ return operationRef;
}

int wsdl__ext_input::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl ext input" << endl;
  return SOAP_OK;
}

int wsdl__ext_output::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl ext output" << endl;
  return SOAP_OK;
}

int wsdl__ext_fault::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl ext fault" << endl;
  return SOAP_OK;
}

int wsdl__portType::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl portType" << endl;
  for (vector<wsdl__operation>::iterator i = operation.begin(); i != operation.end(); ++i)
    (*i).traverse(definitions);
  return SOAP_OK;
}

int wsdl__operation::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl operation" << endl;
  if (input)
    input->traverse(definitions);
  if (output)
    output->traverse(definitions);
  for (vector<wsdl__fault>::iterator i = fault.begin(); i != fault.end(); ++i)
    (*i).traverse(definitions);
  return SOAP_OK;
}

int wsdl__input::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl input" << endl;
  const char *token = qname_token(message, definitions.targetNamespace);
  messageRef = NULL;
  if (token)
    for (vector<wsdl__message>::const_iterator i = definitions.message.begin(); i != definitions.message.end(); ++i)
      if (!strcmp((*i).name, token))
      { messageRef = &(*i);
        cerr << "Found input " << (name?name:"") << " message " << (token?token:"") << endl;
        break;
      }
  if (!messageRef)
    cerr << "Not found: input " << (name?name:"") << " message " << (token?token:"") << endl;
  return SOAP_OK;
}

void wsdl__input::messagePtr(const wsdl__message *message)
{ messageRef = message;
}

const wsdl__message *wsdl__input::messagePtr() const
{ return messageRef;
}

int wsdl__output::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl output" << endl;
  const char *token = qname_token(message, definitions.targetNamespace);
  messageRef = NULL;
  if (token)
    for (vector<wsdl__message>::const_iterator i = definitions.message.begin(); i != definitions.message.end(); ++i)
      if (!strcmp((*i).name, token))
      { messageRef = &(*i);
        cerr << "Found output " << (name?name:"") << " message " << (token?token:"") << endl;
        break;
      }
  if (!messageRef)
    cerr << "Not found: output " << (name?name:"") << " message " << (token?token:"") << endl;
  return SOAP_OK;
}

void wsdl__output::messagePtr(const wsdl__message *message)
{ messageRef = message;
}

const wsdl__message *wsdl__output::messagePtr() const
{ return messageRef;
}

int wsdl__fault::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl fault" << endl;
  const char *token = qname_token(message, definitions.targetNamespace);
  messageRef = NULL;
  if (token)
    for (vector<wsdl__message>::const_iterator i = definitions.message.begin(); i != definitions.message.end(); ++i)
      if (!strcmp((*i).name, token))
      { messageRef = &(*i);
        cerr << "Found fault " << (name?name:"") << " message " << (token?token:"") << endl;
        break;
      }
  if (!messageRef)
    cerr << "Not found: fault " << (name?name:"") << " message " << (token?token:"") << endl;
  return SOAP_OK;
}

void wsdl__fault::messagePtr(const wsdl__message *message)
{ messageRef = message;
}

const wsdl__message *wsdl__fault::messagePtr() const
{ return messageRef;
}

int wsdl__message::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl message" << endl;
  for (vector<wsdl__part>::iterator i = part.begin(); i != part.end(); ++i)
    (*i).traverse(definitions);
  return SOAP_OK;
}

int wsdl__part::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl part" << endl;
  elementRef = NULL;
  simpleTypeRef = NULL;
  complexTypeRef = NULL;
  if (definitions.types)
  { for (vector<xs__schema>::const_iterator schema = definitions.types->xs__schema_.begin(); schema != definitions.types->xs__schema_.end(); ++schema)
    { const char *token = qname_token(element, (*schema).targetNamespace);
      if (token)
        for (vector<xs__element>::const_iterator element = (*schema).element.begin(); element != (*schema).element.end(); ++element)
          if (!strcmp((*element).name, token))
          { elementRef = &(*element);
            cerr << "Found part " << (name?name:"") << " element " << (token?token:"") << endl;
            break;
          }
       token = qname_token(type, (*schema).targetNamespace);
       if (token)
         for (vector<xs__simpleType>::const_iterator simpleType = (*schema).simpleType.begin(); simpleType != (*schema).simpleType.end(); ++simpleType)
           if (!strcmp((*simpleType).name, token))
           { simpleTypeRef = &(*simpleType);
             cerr << "Found part " << (name?name:"") << " simpleType " << (token?token:"") << endl;
             break;
           }
       token = qname_token(type, (*schema).targetNamespace);
       if (token)
         for (vector<xs__complexType>::const_iterator complexType = (*schema).complexType.begin(); complexType != (*schema).complexType.end(); ++complexType)
           if (!strcmp((*complexType).name, token))
           { complexTypeRef = &(*complexType);
             cerr << "Found part " << (name?name:"") << " complexType " << (token?token:"") << endl;
             break;
           }
    }
  }
  else
    cerr << "No WSDL <types>" << endl;
  if (!elementRef)
    cerr << "Not found: part " << (name?name:"") << " element " << (element?element:"") << endl;
  if (!simpleTypeRef && !complexTypeRef)
    cerr << "Not found: part " << (name?name:"") << " type " << (type?type:"") << endl;
  return SOAP_OK;
}

void wsdl__part::elementPtr(const xs__element *element)
{ elementRef = element;
}

void wsdl__part::simpleTypePtr(const xs__simpleType *simpleType)
{ simpleTypeRef = simpleType;
}

void wsdl__part::complexTypePtr(const xs__complexType *complexType)
{ complexTypeRef = complexType;
}

const xs__element *wsdl__part::elementPtr() const
{ return elementRef;
}

const xs__simpleType *wsdl__part::simpleTypePtr() const
{ return simpleTypeRef;
}

const xs__complexType *wsdl__part::complexTypePtr() const
{ return complexTypeRef;
}

int wsdl__types::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl types" << endl;
  for (vector<xs__schema>::iterator i = xs__schema_.begin(); i != xs__schema_.end(); ++i)
  { for (vector<xs__import>::iterator j = (*i).import.begin(); j != (*i).import.end(); ++j)
      for (vector<xs__schema>::iterator k = xs__schema_.begin(); k != xs__schema_.end(); ++k)
	if (!strcmp((*j).namespace_, (*k).targetNamespace))
          (*j).schemaPtr(&(*k));
    (*i).traverse();
  }
  return SOAP_OK;
}

int wsdl__import::traverse(const wsdl__definitions& definitions)
{ cerr << "wsdl import" << endl;
  // should get imported WSDLs
  definitionsRef = NULL;
  return SOAP_OK;
}

void wsdl__import::definitionsPtr(const wsdl__definitions *definitions)
{ definitionsRef = definitions;
}

const wsdl__definitions *wsdl__import::definitionsPtr() const
{ return definitionsRef;
}

////////////////////////////////////////////////////////////////////////////////
//
//	streams
//
////////////////////////////////////////////////////////////////////////////////

ostream &operator<<(ostream &o, const wsdl__definitions &e)
{ if (!e.soap)
  { struct soap soap;
    soap_init2(&soap, SOAP_IO_DEFAULT, SOAP_XML_TREE | SOAP_C_UTFSTRING);
    e.soap_serialize(&soap);
    soap_begin_send(&soap);
    e.soap_out(&soap, "definitions", 0, NULL);
    soap_end_send(&soap);
    soap_end(&soap);
    soap_done(&soap);
  }
  else
  { ostream *os = e.soap->os;
    e.soap->os = &o;
    e.soap_serialize(e.soap);
    soap_begin_send(e.soap);
    e.soap_out(e.soap, "definitions", 0, NULL);
    soap_end_send(e.soap);
    e.soap->os = os;
  }
  return o;
}

istream &operator>>(istream &i, wsdl__definitions &e)
{ if (!e.soap)
    e.soap = soap_new();
  istream *is = e.soap->is;
  e.soap->is = &i;
  if (soap_begin_recv(e.soap)
   || !e.soap_in(e.soap, "definitions", NULL)
   || soap_end_recv(e.soap))
    ; // handle error? Note: e.soap->error is set and app should check
  e.soap->is = is;
  return i;
}

////////////////////////////////////////////////////////////////////////////////
//
//	Miscellaneous
//
////////////////////////////////////////////////////////////////////////////////

static int warn_ignore(struct soap *soap, const char *tag)
{ cerr << "Warning: tag " << tag << " at level " << soap->level << " was not recognized and will be ignored" << endl;
  return SOAP_OK;
}

