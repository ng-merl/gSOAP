
The 'extras' directory contains contributions and other useful additions.

All contributions are covered by the gSOAP public license, unless specifically
stated in the source. The following authors provided the contributions
included in this directory:

fault.cpp contributed by A. Kelly
logging.cpp contributed by M. Helmick
WinInet support contributed by J. Kastanowitz and B. Thiesfield
apache_gsoap constributed by Christian Aberger
mod_gsoap contributed by Christian Aberger

ckdb.h ckdb.c			Simple Cookie database manager (store/load)
ckdbtest.h ckdbtest.c		Test code for Simple Cookie database manager
fault.cpp			Print SOAP Fault messages to C++ streams
GSoapWinInet.h GSoapWinInet.cpp	WinInet support
logging.cpp			Log send, receive, and trace messages on streams
plugin.h plugin.c		Example gSOAP plugin to view SOAP messages
stringtest.h stringtest.cpp	Example std::string serialization
apache_gsoap			Apache module (to run gSOAP within Apache Web server)
mod_gsoap			IIS module (to run gSOAP within IIS)

Please refer to the Christian's Web site for installation intructions for the Apache and
IIS modules for gSOAP:

http://mx.aberger.at/SOAP/

The soapdefs.h file contains supporting declarations (see notes in the sources).

Note: string.cpp in older distributions is obsolete and replaced with stl.h
and stl.cpp in the top directory.
