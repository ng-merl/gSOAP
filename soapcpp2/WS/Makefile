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
CMFLAGS=
CFLAGS= $(CWFLAGS) $(COFLAGS) $(CIFLAGS) $(CMFLAGS)
all:			example
example:		example.c soapC.o soapClient.o stdsoap2.o
			$(CC) $(CFLAGS) -o example example.c soapC.o soapClient.o stdsoap2.o
soapC.c:		example.h
			$(SOAPCPP) -I.. -c example.h
soapClient.c:		example.h
			$(SOAPCPP) -I.. -c example.h
stdsoap2.o:		../stdsoap2.c
			$(CC) $(CFLAGS) -c $<
.c.o:
			$(CC) $(CFLAGS) -c $<
.cpp.o:
			$(CPP) $(CFLAGS) -c $<
.wsdl.h:
			$(WSDL2H) -tWS-typemap.dat $<
.xsd.h:
			$(WSDL2H) -tWS-typemap.dat $<
clean:		
			rm -f *.o *.log *.xml
distclean:		
			rm -f *.o *.log *.xml *.nsmap soapH.h soapStub.h soap*Proxy.h soap*Object.h soapC.c soapClient*.c soapServer*.c
			rm -f example
