gSOAP WSDL parser and importer

INSTRUCTIONS

The gSOAP WSDL parser converts WSDL into a gSOAP header file for processing
with the gSOAP compiler to generate the stubs and skeletons.

For examle:

$ wsdl2h -o Amazon.h http://soap.amazon.com/schemas/AmazonWebServices.wsdl

$ soapcpp2 Amazon.h

See Amazon.h for the types and service functions. A C++ client application may
use the generated 'soapAmazonSearchBindingProxy.h' class and
'AmazonSearchBinding.nsmap' XML namespace table to access the Amazon Web
service. Both need to be '#include'-d in your source. Then compile and link
the soapC.cpp, soapClient.cpp, and stdsoap2.cpp sources to complete the build.
More information can be found in the gSOAP documentation.

When parsing a WSDL, the output file name is the WSDL input file name with
extension '.h' instead of '.wsdl'. When an input file is absent or a WSDL file
from a Web location is accessed, the header output will be produced on the
standard output. An input file may also contain a schema and will be handled as
such.

Command line options:

-c	generate pure C header file code
-f	generate flat C++ class hierarchy for schema extensions
-m	create modules for separate compilation
-nname	use name as the base namespace prefix name instead of 'ns'
-ofile	output to file
-p	create polymorphic types with C++ inheritance with base xsd__anyType
-s	do not generate STL code (no std::string and no std::vector)
-tfile	use type map file instead of the default file typemap.dat
-v	verbose output
-?	help

TYPE MAP FILE

The 'typemap.dat' file can be used to provide custom type mappings for binding
XML schema types to C and/or C++ types. The WSDL parser 'wsdl2h' can be used
without the 'typemap.dat' file, because and internal table is used to associate
XML schema types to C or C++ types (-c option). An example 'typemapDIMEC.dat'
and 'typemapDIMECPP.dat' files provide bindings for DIME attachment support.
The 'typemap.dat' file allows you to change the generation of the 'ns1', 'ns2',
'ns3', ... namespace prefixes with custom names.

INSTALLATION

Type 'make' in the 'wsdl' directory to build wsdl2h. You must first install the
gSOAP package and build the gSOAP compiler soapcpp2 to rebuild the wsdl2h WSDL
parser.

USING SSL FOR HTTPS

You must build the WSDL parser with 'make secure' to build an SSL-enabled
version of wsdl2h that can access HTTPS secure sites.

LICENSE

The gSOAP WSDL parser 'wsdl2h' and source code are released under the GPL.
See gpl.txt for more details. A commercial license is available from Genivia.
Please contact Genivia (contact@genivia.com) for more details.

COPYRIGHT NOTICE

gSOAP XML Web services tools
Copyright (C) 2001-2004, Robert van Engelen, Genivia, Inc. All Rights Reserved.
