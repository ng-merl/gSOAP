WSDL manipulation tools

This directory contains sources to create WSDL manipulation tools with gSOAP.
These tools provide the basis for the new WSDL importer, which will be
available soon.

The following gSOAP header files define the WSDL 1.1 specification (with the
.cpp files implementing the associated class methods):

wsdl.h wsdl.cpp		WSDL specification
soap.h			SOAP extensions
http.h			HTTP extensions
schema.h schema.cpp	XML Schema specification

The wsdl.cpp code and schema.cpp code define class methods to link the various
parts of the WSDL and Schema definitions. For example, class wsdl__part has an
element member. This element is searched and when found the elementRef pointer
is set and accessible via the elementPtr setter/getter.

To compile:
soapcpp2 -ipwsdl wsdl.h
g++ yourmain.cpp wsdl.cpp schema.cpp wsdlC.cpp stdsoap2.cpp

Don't worry about the soapcpp2 warnings. The std::vector members don't need to
be pointers.

To parse and populate a WSDL definitions structure in your code:

	wsdl__definitions definitions(soap_new());
	cin >> definitions;

To print a WSDL definitions structure:

	cout << definitions;
