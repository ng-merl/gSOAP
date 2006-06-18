/*	factorytest.cpp

	Test client for remote object factory

	This is a simple example client program that defines its own proxy
	objects. The proxy objects utilize the gSOAP remote interfaces to
	access the remote factory. The remote object interfaces are declared
	in factorytest.h. Since the remote interfaces are C-style, C-based
	client programs cam be developed to access remote object factories.

	Compile:
	soapcpp2 factorytest.h
	c++ -o factorytest factorytest.cpp stdsoap2.cpp soapC.cpp soapClient.cpp

	Run:
	factorytest <factory-endpoint>
	where <factory-endpoint> is the service endpoint of a factory server,
	e.g. http://localhost:18085

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.
*/

#include "soapH.h"
#include "factorytest.nsmap"

#include <iostream.h>

// default factory service endpoint:
const char *factory = "http://localhost:18085";

////////////////////////////////////////////////////////////////////////////////
//
//  Client-side root proxy class
//
////////////////////////////////////////////////////////////////////////////////

class Root
{ public:
  char *endpoint;			// factory service endpoint
  enum t__status status;		// status flag
  struct soap *soap;			// gSOAP environment (for header h__handle)
  Root();
  Root(const char *factory, enum t__object object, char *name);
  virtual ~Root();
  void rename(char *name);
};

Root::Root()
{ endpoint = NULL;
  status = FACTORY_OK;
  soap = NULL;
}

Root::Root(const char *factory, enum t__object object, char *name)
{ soap = soap_new();
  endpoint = (char*)soap_malloc(soap, strlen(factory)+1);
  strcpy(endpoint, factory);
  status = FACTORY_NOTFOUND;
  if (name)
    if (soap_call_ns__lookup(soap, endpoint, "", object, name, status))
      soap_print_fault(soap, stderr);	// for demo, just print
  if (status == FACTORY_NOTFOUND)
    do
    { if (soap_call_ns__create(soap, endpoint, "", object, name, status))
        soap_print_fault(soap, stderr);	// for demo, just print
    } while (status == FACTORY_RETRY);
}

Root::~Root()
{ if (soap_call_ns__release(soap, endpoint, "", status))
    soap_print_fault(soap, stderr);	// for demo, just print
  soap_destroy(soap);
  soap_end(soap);
  soap_done(soap);
  free(soap);
}

void Root::rename(char *name)
{ if (soap_call_ns__rename(soap, endpoint, "", name, status))
    soap_print_fault(soap, stderr);	// for demo, just print
}

////////////////////////////////////////////////////////////////////////////////
//
//  Client-side adder proxy class derived from root
//
////////////////////////////////////////////////////////////////////////////////

class Adder: public Root
{ public:
  Adder() : Root(factory, ADDER, NULL) {};
  Adder(char *name) : Root(factory, ADDER, name) {};
  Adder(const char *factory, enum t__object object, char *name) : Root(factory, object, name) {};
  void set(double val);			// to set the remote adder
  double get();				// to get value of the remote adder
  void add(double val);			// to add a value to the remote adder
};

void Adder::set(double val)
{ if (soap_call_ns__set(soap, endpoint, "", val, status))
    soap_print_fault(soap, stderr);	// for demo, just print
}

double Adder::get()
{ double val;
  if (soap_call_ns__get(soap, endpoint, "", val))
    soap_print_fault(soap, stderr);	// for demo, just print
  return val;
}

void Adder::add(double val)
{ if (soap_call_ns__add(soap, endpoint, "", val, status))
    soap_print_fault(soap, stderr);	// for demo, just print
}

////////////////////////////////////////////////////////////////////////////////
//
//  Client-side counter proxy class derived from adder
//
////////////////////////////////////////////////////////////////////////////////

class Counter: public Adder
{ public:
  Counter() : Adder(factory, COUNTER, NULL) {};
  Counter(char *name) : Adder(factory, COUNTER, name) {};
  Counter(const char *factory, enum t__object object, char *name) : Adder(factory, object, name) {};
  void inc();				// to increment the remote counter
};

void Counter::inc()
{ if (soap_call_ns__inc(soap, endpoint, "", status))
    soap_print_fault(soap, stderr);	// for demo, just print
}

////////////////////////////////////////////////////////////////////////////////
//
//  Main client test program
//
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{ if (argc > 1)
    factory = argv[1];			// use factory from command line arg by default
  cout << "Connecting to factory " << factory << endl;
  Adder adder;				// create unique new remote adder object
  Counter counter1("myCounter");	// new counter object "myCounter" (created if not exists)
  Counter counter2("myCounter");	// lookup and use counter "myCounter" (this is an alias to counter1!)
  adder.set(2.0);
  counter1.set(adder.get());
  adder.add(3.0);
  counter1.inc();
  cout << "Adder=" << adder.get() << endl;
  cout << "Counter=" << counter2.get() << endl;		// counter2 is an alias for counter1 so this prints the value of counter1
  cout << "Sleep for 90 seconds to test factory server purging objects:" << endl;
  // counter is periodically incremented which keeps it alive
  sleep(30);
  counter1.inc();
  cout << "Counter=" << counter2.get() << endl;
  sleep(30);
  counter1.inc();
  cout << "Counter=" << counter2.get() << endl;
  sleep(30);
  counter1.inc();
  cout << "Counter=" << counter2.get() << endl;
  // after 90 secs, the adder should be gone
  cout << "Adder is no longer available:" << endl;
  adder.add(3.0);
  cout << "Adder status = " << adder.status << endl;
  return 0;
}
