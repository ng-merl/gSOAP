
The 'extras' directory contains contributions and other useful additions.

ckdb.h ckdb.c			Simple Cookie database manager (store/load)
ckdbtest.h ckdbtest.c		Test code for Simple Cookie database manager
fault.cpp			Print SOAP Fault messages to C++ streams
GSoapWinInet.h GSoapWinInet.cpp	WinInet support
logging.cpp			Log send, receive, and trace messages on streams
plugin.h plugin.c		Example gSOAP plugin to view SOAP messages
stringtest.h stringtest.cpp	Example std::string serialization

The soapdefs.h file contains supporting declarations (see notes in the sources).

All contributions are covered by the gSOAP public license. The following
authors provided contributions:

fault.cpp contributed by A. Kelly
logging.cpp contributed by M. Helmick
WinInet support contributed by J. Kastanowitz

Note: string.cpp in older distributions is obsolete and replaced with stl.h
and stl.cpp in the top directory.
