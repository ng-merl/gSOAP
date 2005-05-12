
The gSOAP 2 compiler sources are Copyright (C) 2000-2005 Robert A. van Engelen,
Florida State University and Genivia Inc. All rights reserved.

THIS PART OF THE PACKAGE IS INTENDED TO SUPPORT THE MIGRATION OF gSOAP TO
DIFFERENT PLATFORMS. The code has not been cleaned. No documentation is
enclosed.  Because Web service technology and protocols such as SOAP and WSDL
are changing rapidly, periodic updates will be provided. As a consequence, the
use of this code as part of a larger work cannot be guaranteed to work with
future releases of this software and will most likely fail with future
additions. For questions, please contact the author of the software.

The terms and conditions of use of this software do not allow for the removal
of the copyright notice from the main program for visual display. For
integration with other software, a similar copyright notice must be produced
that is visible to users of the software.

The compiler source distribution contains the following files:
README.txt	This file
Makefile	Unix/linux makefile
MakefileMacOSX	Mac OS X Makefile
soapcpp2.h	Main header file
soapcpp2.c	Main application
symbol2.c	Symbol table handling and code generation module
error2.h	Header file for error2.c
error2.c	Error handling routines
init2.c		Compiler symbol table initialization
soapcpp2_lex.l	Flex/Lex tokens
soapcpp2_yacc.y	Yacc/Bison grammar


BUILD NOTES

QNX
===

On QNX the bison.simple file is located in $QNX_HOST/usr/share/bison.simple
Update your .profile to include:

export BISON_SIMPLE=$QNX_HOST/usr/share/bison/bison.simple 
export BISON_HAIRY=$QNX_HOST/usr/share/bison/bison.hairy 

WIN32
=====

Bison 1.6 can crash on Win32 systems if YYINITDEPTH is too small Compile with
/DYYINITDEPTH=1000

