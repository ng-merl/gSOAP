gSOAP UDDI v2

See directory html/index.html for documentation and examples of the UDDI v2 API
generated with Doxygen. The API is written in C++. However, wsdl2h option -c
can be used to generate C code serializers for UDDI v2.

The current implementation supports UDDI "inquiry" bindings. The UDDI "publish"
bindings are incomplete for this release and will be included together with
documentation and examples in the next release.

A Makefile is included to build two example C++ clients, example1 and example2,
to search services and businesses, respectively.

COMPILATION

The build steps are generally as follows:

# Run wsdl2h with mapping specified in uddi2-typemap.dat on the UDDI WSDL:
$ wsdl2h -tuddi2-typemap.dat inquire_v2.wsdl

# Run soapcpp2 on the generated .h file (-pinq saves sources as inqXYZ):
$ soapcpp2 -I.. -pinq inquire_v2.h

# Complete the build:
$ g++ -DWITH_NONAMESPACES -I.. -o main main.cpp inquire_v2.cpp inqC.cpp inqClient.cpp ../stdsoap2.cpp

# To avoid link errors combining multiple services, replace the last step with:
$ echo '' > env.h
$ soapcpp2 -I.. -penv env.h
$ g++ -DWITH_NONAMESPACES -I.. -o main main.cpp inquire_v2.cpp inqClientLib.cpp ../stdsoap2.cpp

USING DOXYGEN

To generate documentation, use Doxygen on the generated header file, e.g.
inquire_v2.h. Part of the documentation is included in the uddi2-typemap.dat
file, which was used to generate the header file(s) with wsdl2h.
