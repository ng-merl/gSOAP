/*	init2.c
	Symbol table initialization

The contents of this file are subject to the gSOAP Public License Version 1.2
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
#include "soapcpp2_yacc.h"

typedef	struct Keyword
{ char *s;	/* name */
  Token t;	/* token */
} Keyword;

static Keyword keywords[] =
{	"auto",		AUTO,
	"bool",		BOOL,
	"break",	BREAK,
	"case",		CASE,
	"char",		CHAR,
	"class",	CLASS,
	"const",	CONST,
	"continue",	CONTINUE,
	"default",	DEFAULT,
	"do",		DO,
	"double",	DOUBLE,
	"else",		ELSE,
	"enum",		ENUM,
	"explicit",	EXPLICIT,
	"extern",	EXTERN,
	"false",	CFALSE,
	"float",	FLOAT,
	"for",		FOR,
	"friend",	FRIEND,
	"goto",		GOTO,
	"if",		IF,
	"inline",	INLINE,
	"int",		INT,
	"long",		LONG,
	"LONG64",	LLONG,
	"namespace",	NAMESPACE,
	"operator",	OPERATOR,
	"private",	PRIVATE,
	"protected",	PROTECTED,
	"public",	PUBLIC,
	"register",	REGISTER,
	"return",	RETURN,
	"short",	SHORT,
	"signed",	SIGNED,
	"size_t",	SIZE,
	"sizeof",	SIZEOF,
	"static",	STATIC,
	"struct",	STRUCT,
	"switch",	SWITCH,
	"template",	TEMPLATE,
	"time_t",	TIME,
	"true",		CTRUE,
	"typedef",	TYPEDEF,
	"ULONG64",	ULLONG,
	"union",	UNION,
	"unsigned",	UNSIGNED,
	"using",	USING,
	"virtual",	VIRTUAL,
	"void",		VOID,
	"volatile",	VOLATILE,
	"wchar_t",	WCHAR,
	"while",	WHILE,

	"operator!",	NONE,
	"operator~",	NONE,
	"operator=",	NONE,
	"operator+=",	NONE,
	"operator-=",	NONE,
	"operator*=",	NONE,
	"operator/=",	NONE,
	"operator%=",	NONE,
	"operator&=",	NONE,
	"operator^=",	NONE,
	"operator|=",	NONE,
	"operator<<=",	NONE,
	"operator>>=",	NONE,
	"operator||",	NONE,
	"operator&&",	NONE,
	"operator|",	NONE,
	"operator^",	NONE,
	"operator&",	NONE,
	"operator==",	NONE,
	"operator!=",	NONE,
	"operator<",	NONE,
	"operator<=",	NONE,
	"operator>",	NONE,
	"operator>=",	NONE,
	"operator<<",	NONE,
	"operator>>",	NONE,
	"operator+",	NONE,
	"operator-",	NONE,
	"operator*",	NONE,
	"operator/",	NONE,
	"operator%",	NONE,
	"operator++",	NONE,
	"operator--",	NONE,
	"operator[]",	NONE,
	"operator()",	NONE,

	"mustUnderstand",	MUSTUNDERSTAND,

	"SOAP_ENV__Header",	ID,
	"dummy",		ID,
	"soap_header",		ID,

	"SOAP_ENV__Fault",	ID,
	"SOAP_ENV__Code",	ID,
	"SOAP_ENV__Reason",	ID,
	"SOAP_ENV__Detail",	ID,
	"SOAP_ENV__Value",	ID,
	"SOAP_ENV__Node",	ID,
	"SOAP_ENV__Role",	ID,
	"faultcode",		ID,
	"faultstring",		ID,
	"faultactor",		ID,
	"detail",		ID,

	"_QName",		TYPE,
	"std::string",		TYPE,
	"std::wstring",		TYPE,

	"/*?*/",		NONE,

	0,			0
};

/*
init - initialize symbol table with predefined keywords
*/
init()
{	struct	Keyword *k;
	for (k = keywords; k->s; k++)
		install(k->s, k->t);
}
