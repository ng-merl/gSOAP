Please see soapcpp2/README.txt for more info on the gSOAP package.

This file explains how gSOAP is build on your platform.

Win32 users can start right away. A soapcpp2.exe binary compiler is included
in this distribution. MSVC++ examples can be found in soapcpp2/samples/magic_VC
and soapcpp2/samples/quote_VC.

To build gSOAP on your platform, please enter the following command:
./configure
make
make install

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

