
The 'extras' directory contains contributions and other useful additions.

All contributions are covered by the gSOAP public license, unless specifically
stated otherwise in the source. The following authors provided the
contributions included in this directory:

fault.cpp contributed by A. Kelly
logging.cpp contributed by M. Helmick

ckdb.h ckdb.c			Simple Cookie database manager (store/load)
ckdbtest.h ckdbtest.c		Test code for Simple Cookie database manager
fault.cpp			Print SOAP Fault messages to C++ streams
httpget.h httpget.c		HTTP GET plugin to support HTTP GET operations
logging.h logging.c		Message logging and statistics plugin
httpmd5.h httpmd5.c		HTTP Content-MD5 support
httpmd5test.h httpmd5test.c	Example HTTP Content-MD5 test application
logging.cpp			Log send, receive, and trace messages on streams
plugin.h plugin.c		Example gSOAP plugin to view SOAP messages

The soapdefs.h file contains supporting declarations (see notes in the sources).

Note: string.cpp in older distributions is obsolete and replaced with stl.h
