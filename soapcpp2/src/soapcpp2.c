/*	soapcpp2.c
	Main program

The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the
License. You may obtain a copy of the License at
http://www.cs.fsu.edu/~engelen/gsoapcompilerlicense.html
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.

The Original Code is ''gSOAP compiler'' consisting of:
error2.c, error2.h, init2.c, soapcpp2.c, soapcpp2.h, soapcpp2_lex.l, soapcpp2_yacc.y, symbol2.c.
The Initial Developer of the Original Code is Robert A. van Engelen.
Copyright (C) 2000-2003 Robert A. van Engelen, Genivia inc. All Rights Reserved.

*/

#include "soapcpp2.h"

extern int init();
extern int yyparse();
extern FILE *yyin;

int iflag = 0;		/* when set, enable #include and #define directives */
int cflag = 0;		/* when set, generate files with .c extension */
int mflag = 0;		/* when set, generate code that requires array/binary classes to explicitly remove malloced array */
char dirpath[1024];	/* directory path for generated source files */
char *prefix = "soap";	/* file name prefix for generated source files */
char filename[1024];	/* current file name */

/*
IMPORTANT:

The terms and conditions of use of this software do not allow for the removal
of the copyright notice from the main program for visual display. For
integrations with other software, a similar copyright notice must be produced
that is visible to users of the software.
*/

int
main(int argc, char **argv)
{	int i, g;
	char *a;
	strcpy(filename, "<stdin>");
	fprintf(stderr, "\n** The gSOAP Stub and Skeleton Compiler for C and C++ "VERSION"\n** Copyright (C) 2001-2003 Genivia inc. & FSU. All Rights Reserved.\n** This product is provided \"as is\", without any warranty\n\n");
	for (i = 1; i < argc; i++)
	{	a = argv[i];
		if (*a == '-'
#ifdef WIN32
		 || *a == '/'
#endif
		)
		{	g = 1;
			while (g && *++a)
				switch (*a)
				{	case 'c':
						cflag = 1;
						break;
					case 'd':
						a++;
						g = 0;
						if (*a)
							strcpy(dirpath, a);
						else if (i < argc && argv[++i])
							strcpy(dirpath, argv[i]);
						else
							execerror("Option -d requires a directory path");
						if (*dirpath && dirpath[strlen(dirpath)-1] != '/' && dirpath[strlen(dirpath)-1] != '\\')
#ifdef WIN32
							strcat(dirpath, "\\");
#else
							strcat(dirpath, "/");
#endif
						break;
					case '?':
					case 'h':
						fprintf(stderr, "Usage: soapcpp2 [-1|-2] [-d<path>] [-p<name>] [-c] [-i] [-m] [file]\n");
						exit(0);
					case 'i':
						iflag = 1;
						break;
					case 'm':
						mflag = 1;
						break;
					case 'p':
						a++;
						g = 0;
						if (*a)
							prefix = a;
						else if (i < argc && argv[++i])
							prefix = argv[i];
						else
							execerror("Option -p requires an output file name prefix");
						break;
					case '1':
						envURI = "http://schemas.xmlsoap.org/soap/envelope/";
						encURI = "http://schemas.xmlsoap.org/soap/encoding/";
						break;
					case '2':
						envURI = "http://www.w3.org/2002/06/soap-envelope";
						encURI = "http://www.w3.org/2002/06/soap-encoding";
						break;
					default:
						execerror("Unknown option");
				}
		}
		else if (!(yyin = fopen(argv[i], "r")))
		{	sprintf(errbuf, "Cannot open file \"%s\" for reading", argv[i]);
			execerror(errbuf);
		}
		else
			strcpy(filename, argv[i]);
	}
	init();
	if (yyparse())
		synerror("skipping the remaining part of the input");
	return errstat();
}
