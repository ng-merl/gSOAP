gSOAP WSDL Importer tool 0.11
=============================

INSTALLATION
============

1. Install an XML parser for Java, e.g. Apache Xerces 2.0.2 (xercesImpl.jar
   and xmlParserAPIs.jar from http://xml.apache.org/xerces2-j/index.html)
   or 1.4.4 (xerces.jar), and change the classpath accordingly (e.g.
   setenv CLASSPATH .:~/xerces/xmlParserAPIs.jar:~/xerces/xercesImpl.jar).
   The wsdlcpp.java source of the WSDL import tool is pre-configured for the
   Xerces 2 Dom parser. To use another Java DOM parser, please change the
   Dom parser class import in wsdlcpp.java.

2. javac wsdlcpp.java

USAGE
=====

The creation of proxies (stubs) from WSDL is a two step process.

1. Execute: java wsdlcpp <file>.wsdl

   This generates the following files:
   
   <file>.h	the header file declarations for the gSOAP compiler
   <file>.c	a client program template

   Alternatively, execute: java wsdlcpp -c <file>.wsdl
   to generate pure-C header files and source codes.

   The header file is to be processed by the gSOAP compiler:

2. Execute: soapcpp2 <file>.h

   This generates the following files:

   soapStub.h		similar to <file>.h, but with data type annotations
   soapH.h		header file of soapC.cpp
   soapC.cpp		SOAP/XML (de)serializers for C/C++ data types
   soapClient.cpp	proxy stub routines for remote method calls
   soapServer.cpp	skeleton routines for service implementation
			(not required for client applications)
   soap<srv>.wsdl	a WSDL file, where <srv> is the name of the service
   soap<srv>.nsmap	a namespace mapping table for the client application

   Modify the client program template <file>.c to suit your needs.

   Compile the client program as explained in the gSOAP documentation, e.g.
   g++ <file>.c stdoap2.cpp soapC.cpp soapClient.cpp
