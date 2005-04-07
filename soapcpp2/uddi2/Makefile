.SUFFIXES: .wsdl .xsd
WSDL2H=wsdl2h
SOAPCPP=soapcpp2
CC=gcc
CPP=g++
LIBS=
#COFLAGS=-g -DDEBUG -fstack-check
COFLAGS=
CWFLAGS=-Wall
CIFLAGS=-I..
CMFLAGS=-DWITH_NONAMESPACES
CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)
all:			example1 example2
example1:		example1.cpp inqClientLib.o inquire_v2.o stdsoap2.o envC.o
			$(CPP) $(CFLAGS) -o example1 example1.cpp inqClientLib.o inquire_v2.o stdsoap2.o envC.o
example2:		example2.cpp inqClientLib.o inquire_v2.o stdsoap2.o envC.o
			$(CPP) $(CFLAGS) -o example2 example2.cpp inqClientLib.o inquire_v2.o stdsoap2.o envC.o
inqClientLib.cpp:	inquire_v2.h
			$(SOAPCPP) -I.. -p inq inquire_v2.h
pubClientLib.cpp:	publish_v2.h
			$(SOAPCPP) -I.. -p pub publish_v2.h
envC.cpp:
			echo '' > env.h
			$(SOAPCPP) -I.. -p env env.h
stdsoap2.o:		../stdsoap2.cpp
			$(CPP) $(CFLAGS) -c $<
.c.o:
			$(CPP) $(CFLAGS) -c $<
.cpp.o:
			$(CPP) $(CFLAGS) -c $<
.wsdl.h:
			$(WSDL2H) -tuddi2-typemap.dat $<
.xsd.h:
			$(WSDL2H) -tuddi2-typemap.dat $<
clean:		
			rm -f *.o *.log *.xml *.nsmap inqH.h *Stub.h *Proxy.h *Object.h inqC*.cpp inqS*.cpp pubH.h pubC*.cpp pubS*.cpp env*.h env*.cpp
distclean:		
			rm -f *.o *.log *.xml *.nsmap inqH.h *Stub.h *Proxy.h *Object.h inqC*.cpp inqS*.cpp pubH.h pubC*.cpp pubS*.cpp env*.h env*.cpp
			rm -f example1 example2
