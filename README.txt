Please see soapcpp2/README.txt for more info on the gSOAP package.

This file explains how gSOAP is build on your platform.

Win32 users can start right away. A soapcpp2.exe binary compiler and
wsdl2h.exe WSDL parser are included in this distribution. Please note that
specific Windows, Linux, Sun, and Mac OS X releases with binaries are
available from the SourceForge gsoap2 project.

To build gSOAP on your platform, please enter the following command:
./configure
make
make install

This will install the executables and libraries on your system (and you need
root access to do so).

If you want to install the executables in your local directory, enter:

./configure
make
make install exec_prefix=$HOME

To build your projects, you need the executables:

soapcpp2/src/soapcpp2		gSOAP stub/skeleton compiler
soapcpp2/wsdl/wsdl2h		gSOAP WSDL parser

And you need the following libraries (build from stdsoap2.c[pp]):

soapcpp2/libgsoap++.a		C++ runtime
soapcpp2/libgsoapck++.a		C++ runtime with HTTP cookie support
soapcpp2/libgsoap.a		C runtime
soapcpp2/libgsoapck.a		C runtime with HTTP cookie support

Alternatively, you can use the stdsoap2.c[pp] source codes to build your
project, see the gSOAP documentation on how to enable HTTP cookies,
Zlib compression, and SSL support.

For developers: there is a commandfile ./makemake
It can be used to generate the required ./configure script.
This will create the Makefiles. After this is done run the command:
make

You can also execute the following steps from the commandline:

aclocal
autoheader
automake --add-missing
autoconf
automake
./configure

Then run:
make

