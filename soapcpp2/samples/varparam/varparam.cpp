/*	varparam.cpp

	Example use of variable parameter lists with the full XML
	schema type hierarchy implemented as a C++ class hierarchy.

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

Demonstrates the use of variable number of parameters and polymorphic
parameters. The 'trick' is to use __size parameters. The __size fields
can be used in structs/classes to embed arrays. Because a request message
of a remote method is essentially a struct, the use of __size in 
parameters of a method has the effect of sending a variable number of
parameters.

Use option -i with soapcpp2 to compile the header file varparam.h

Run the executable as client from the command line with one argument
to test the polymorphic parameter exchange. Run with more arguments
to send the arguments as a variable parameter list to the server.

*/

#include "soapH.h"
#include "varparam.nsmap"

char endpoint[] = "http://websrv.cs.fsu.edu/~engelen/varparam.cgi";

#define N 8

int main(int argc, char **argv)
{ struct soap soap;
  struct ns__varPolyParamTestResponse r;
  int n;
  xsd__anyType *p[N];	// array of polymorphic parameters
  soap_init(&soap);
  if (argc < 2)
  { soap_serve(&soap);
    soap_destroy(&soap);
    soap_end(&soap);
    return 0;
  }
  if (argc < 3)
  { p[0] = new xsd__anyURI(endpoint);
    p[1] = new xsd__string(argv[1]);
    p[2] = new xsd__boolean(true);
    p[3] = new xsd__dateTime(1234567);
    p[4] = new xsd__double(1234567.89);
    p[5] = new xsd__base64Binary("encoded in base64");
    p[6] = new xsd__hexBinary("encoded in hex");
    p[7] = new array(2);
    (*p[7])[0] = new xsd__int(7);
    (*p[7])[1] = new xsd__token("x");
    if (soap_call_ns__varPolyParamTest(&soap, endpoint, "", N, p, r))
      soap_print_fault(&soap, stderr);
    else
    { cout << "Server has echoed:" << endl;
      for (int i = 0; i < r.__size; i++)
        r.param[i]->print(cout);
      cout << endl;
    }
    for (int i = 0; i < N; i++)
      delete p[i];
  }
  else
  { if (soap_call_ns__varStringParamTest(&soap, endpoint, "", argc, argv, n))
      soap_print_fault(&soap, stderr);
    else
      printf("Server has responded to %d strings\n", n);
  }
  soap_destroy(&soap);
  soap_end(&soap);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Remote method implementations
//
////////////////////////////////////////////////////////////////////////////////

int ns__varStringParamTest(struct soap *soap, int __size, char **param, int &return_)
{ return_ = __size;
  return SOAP_OK;
}

int ns__varPolyParamTest(struct soap *soap, int __size, xsd__anyType **param, struct ns__varPolyParamTestResponse &out)
{ out.__size = __size;
  out.param = param;
  return SOAP_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
//  XSD schema class hierarchy
//
////////////////////////////////////////////////////////////////////////////////

xsd__anyType::xsd__anyType()
{ soap = NULL;			// This will be set by gSOAP environment later (upon deserialization or calling soap_new_X()
}
xsd__anyType::~xsd__anyType()
{ soap_unlink(soap, this);	// Let gSOAP not deallocate this object again if this object was explicitly removed
}
xsd__anyType*& xsd__anyType::operator[](int i)
{ static xsd__anyType *p = this;	// trick: we don't expect the operator to be applied, but we need a method
  return p;
}
void xsd__anyType::print(ostream &s) const
{ s << "<anyType>"; }

xsd__anySimpleType::xsd__anySimpleType()
{ }
xsd__anySimpleType::~xsd__anySimpleType()
{ }
void xsd__anySimpleType::print(ostream &s) const
{ s << "<anySimpleType>"; }

xsd__anyURI::xsd__anyURI()
{ }
xsd__anyURI::~xsd__anyURI()
{ }
xsd__anyURI::xsd__anyURI(char *s)
{ __item = s; }
void xsd__anyURI::print(ostream &s) const
{ s << "<anyURI=" << __item << ">"; }

xsd__boolean::xsd__boolean()
{ }
xsd__boolean::~xsd__boolean()
{ }
xsd__boolean::xsd__boolean(bool b)
{ __item = b; }
void xsd__boolean::print(ostream &s) const
{ s << "<boolean=" << __item << ">"; }

xsd__date::xsd__date()
{ }
xsd__date::~xsd__date()
{ }
xsd__date::xsd__date(char *s)
{ __item = s; }
void xsd__date::print(ostream &s) const
{ s << "<date=" << __item << ">"; }

xsd__dateTime::xsd__dateTime()
{ }
xsd__dateTime::~xsd__dateTime()
{ }
xsd__dateTime::xsd__dateTime(time_t t)
{ __item = t; }
void xsd__dateTime::print(ostream &s) const
{ s << "<dateTime=" << __item << ">"; }

xsd__double::xsd__double()
{ }
xsd__double::~xsd__double()
{ }
xsd__double::xsd__double(double d)
{ __item = d; }
void xsd__double::print(ostream &s) const
{ s << "<double=" << __item << ">"; }

xsd__duration::xsd__duration()
{ }
xsd__duration::~xsd__duration()
{ }
xsd__duration::xsd__duration(char *s)
{ __item = s; }
void xsd__duration::print(ostream &s) const
{ s << "<duration=" << __item << ">"; }

xsd__float::xsd__float()
{ }
xsd__float::~xsd__float()
{ }
xsd__float::xsd__float(float f)
{ __item = f; }
void xsd__float::print(ostream &s) const
{ s << "<float=" << __item << ">"; }

xsd__time::xsd__time()
{ }
xsd__time::~xsd__time()
{ }
xsd__time::xsd__time(char *s)
{ __item = s; }
void xsd__time::print(ostream &s) const
{ s << "<time=" << __item << ">"; }

xsd__string::xsd__string()
{ }
xsd__string::~xsd__string()
{ }
xsd__string::xsd__string(char *s)
{ __item = s; }
void xsd__string::print(ostream &s) const
{ s << "<string=" << __item << ">"; }

xsd__normalizedString::xsd__normalizedString()
{ }
xsd__normalizedString::~xsd__normalizedString()
{ }
xsd__normalizedString::xsd__normalizedString(char *s)
{ __item = s; }
void xsd__normalizedString::print(ostream &s) const
{ s << "<normalizedString=" << __item << ">"; }

xsd__token::xsd__token()
{ }
xsd__token::~xsd__token()
{ }
xsd__token::xsd__token(char *s)
{ __item = s; }
void xsd__token::print(ostream &s) const
{ s << "<token=" << __item << ">"; }

xsd__decimal::xsd__decimal()
{ }
xsd__decimal::~xsd__decimal()
{ }
xsd__decimal::xsd__decimal(char *s)
{ __item = s; }
void xsd__decimal::print(ostream &s) const
{ s << "<decimal=" << __item << ">"; }

xsd__integer::xsd__integer()
{ }
xsd__integer::~xsd__integer()
{ }
xsd__integer::xsd__integer(char *s)
{ __item = s; }
void xsd__integer::print(ostream &s) const
{ s << "<integer=" << __item << ">"; }

xsd__nonPositiveInteger::xsd__nonPositiveInteger()
{ }
xsd__nonPositiveInteger::~xsd__nonPositiveInteger()
{ }
xsd__nonPositiveInteger::xsd__nonPositiveInteger(char *s)
{ __item = s; }
void xsd__nonPositiveInteger::print(ostream &s) const
{ s << "<nonPositiveInteger=" << __item << ">"; }

xsd__negativeInteger::xsd__negativeInteger()
{ }
xsd__negativeInteger::~xsd__negativeInteger()
{ }
xsd__negativeInteger::xsd__negativeInteger(char *s)
{ __item = s; }
void xsd__negativeInteger::print(ostream &s) const
{ s << "<negativeInteger=" << __item << ">"; }

xsd__nonNegativeInteger::xsd__nonNegativeInteger()
{ }
xsd__nonNegativeInteger::~xsd__nonNegativeInteger()
{ }
xsd__nonNegativeInteger::xsd__nonNegativeInteger(char *s)
{ __item = s; }
void xsd__nonNegativeInteger::print(ostream &s) const
{ s << "<nonNegativeInteger=" << __item << ">"; }

xsd__positiveInteger::xsd__positiveInteger()
{ }
xsd__positiveInteger::~xsd__positiveInteger()
{ }
xsd__positiveInteger::xsd__positiveInteger(char *s)
{ __item = s; }
void xsd__positiveInteger::print(ostream &s) const
{ s << "<positiveInteger=" << __item << ">"; }

xsd__long::xsd__long()
{ }
xsd__long::~xsd__long()
{ }
xsd__long::xsd__long(LONG64 ll)
{ __item = ll; }
void xsd__long::print(ostream &s) const
{ s << "<long=" << __item << ">"; }

xsd__int::xsd__int()
{ }
xsd__int::~xsd__int()
{ }
xsd__int::xsd__int(int i)
{ __item = i; }
void xsd__int::print(ostream &s) const
{ s << "<int=" << __item << ">"; }

xsd__short::xsd__short()
{ }
xsd__short::~xsd__short()
{ }
xsd__short::xsd__short(short h)
{ __item = h; }
void xsd__short::print(ostream &s) const
{ s << "<short=" << __item << ">"; }

xsd__byte::xsd__byte()
{ }
xsd__byte::~xsd__byte()
{ }
xsd__byte::xsd__byte(char c)
{ __item = c; }
void xsd__byte::print(ostream &s) const
{ s << "<byte=" << __item << ">"; }

xsd__unsignedLong::xsd__unsignedLong()
{ }
xsd__unsignedLong::~xsd__unsignedLong()
{ }
xsd__unsignedLong::xsd__unsignedLong(ULONG64 ull)
{ __item = ull; }
void xsd__unsignedLong::print(ostream &s) const
{ s << "<unsignedLong=" << __item << ">"; }

xsd__unsignedInt::xsd__unsignedInt()
{ }
xsd__unsignedInt::~xsd__unsignedInt()
{ }
xsd__unsignedInt::xsd__unsignedInt(unsigned int ui)
{ __item = ui; }
void xsd__unsignedInt::print(ostream &s) const
{ s << "<unsignedInt=" << __item << ">"; }

xsd__unsignedShort::xsd__unsignedShort()
{ }
xsd__unsignedShort::~xsd__unsignedShort()
{ }
xsd__unsignedShort::xsd__unsignedShort(unsigned short uh)
{ __item = uh; }
void xsd__unsignedShort::print(ostream &s) const
{ s << "<unsignedShort=" << __item << ">"; }

xsd__unsignedByte::xsd__unsignedByte()
{ }
xsd__unsignedByte::~xsd__unsignedByte()
{ }
xsd__unsignedByte::xsd__unsignedByte(unsigned char uc)
{ __item = uc; }
void xsd__unsignedByte::print(ostream &s) const
{ s << "<unsignedByte=" << __item << ">"; }

xsd__base64Binary::xsd__base64Binary()
{ }
xsd__base64Binary::~xsd__base64Binary()
{ }
xsd__base64Binary::xsd__base64Binary(char *s)
{ __size = strlen(s)+1;
  __ptr = (unsigned char*)s;
}
void xsd__base64Binary::print(ostream &s) const
{ s << "<base64Binary=" << __size << " bytes>"; }

xsd__hexBinary::xsd__hexBinary()
{ }
xsd__hexBinary::~xsd__hexBinary()
{ }
xsd__hexBinary::xsd__hexBinary(char *s)
{ __size = strlen(s)+1;
  __ptr = (unsigned char*)s;
}
void xsd__hexBinary::print(ostream &s) const
{ s << "<hexBinary=" << __size << " bytes>"; }

array::array()
{ }
array::~array()
{ }
array::array(int n)
{ __size = n;
  __ptr = (xsd__anyType**)soap_malloc(soap, n*sizeof(xsd__anyType*));
}
xsd__anyType*& array::operator[](int i)
{ return __ptr[i];
}
void array::print(ostream &s) const
{ s << "<array=";
  for (int i = 0; i < __size; i++)
    __ptr[i]->print(s);
  s << ">";
}

