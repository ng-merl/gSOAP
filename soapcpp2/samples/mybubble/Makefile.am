####### This is the input file for automake, which will generate Makefile.in ##########
## not a GNU package. You can remove this line, if
## you have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

noinst_PROGRAMS=mybubble
SOAPHEADER=mybubble.h

include ../Makefile.defines
SOAP_NS=mybubble.nsmap mybubble.wsdl
include ../Makefile.c_rules

mybubble_SOURCES = mybubble.c $(SOAPHEADER) $(SOAP_C_CLIENT)

