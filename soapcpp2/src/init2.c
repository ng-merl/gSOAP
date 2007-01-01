/*

init2.c

Symbol table initialization.

gSOAP XML Web services tools
Copyright (C) 2000-2005, Robert van Engelen, Genivia Inc. All Rights Reserved.
This part of the software is released under one of the following licenses:
GPL, the gSOAP public license, or Genivia's license for commercial use.
--------------------------------------------------------------------------------
gSOAP public license.

The contents of this file are subject to the gSOAP Public License Version 1.3
(the "License"); you may not use this file except in compliance with the
License. You may obtain a copy of the License at
http://www.cs.fsu.edu/~engelen/soaplicense.html
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.

The Initial Developer of the Original Code is Robert A. van Engelen.
Copyright (C) 2000-2004 Robert A. van Engelen, Genivia inc. All Rights Reserved.
--------------------------------------------------------------------------------
GPL license.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org
--------------------------------------------------------------------------------
A commercial use license is available from Genivia, Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

#include "soapcpp2.h"
#include "soapcpp2_yacc.h"

typedef	struct Keyword
{ char *s;	/* name */
  Token t;	/* token */
} Keyword;

static Keyword keywords[] =
{	{ "asm",		NONE },
	{ "auto",		AUTO },
	{ "bool",		BOOL },
	{ "break",		BREAK },
	{ "case",		CASE },
	{ "catch",		NONE },
	{ "char",		CHAR },
	{ "class",		CLASS },
	{ "const",		CONST },
	{ "const_cast",		NONE },
	{ "continue",		CONTINUE },
	{ "default",		DEFAULT },
	{ "delete",		NONE },
	{ "do",			DO },
	{ "double",		DOUBLE },
	{ "dynamic_cast",	NONE },
	{ "else",		ELSE },
	{ "enum",		ENUM },
	{ "errno",		NONE },
	{ "explicit",		EXPLICIT },
	{ "export",		NONE },
	{ "extern",		EXTERN },
	{ "false",		CFALSE },
	{ "float",		FLOAT },
	{ "for",		FOR },
	{ "friend",		FRIEND },
	{ "goto",		GOTO },
	{ "if",			IF },
	{ "inline",		INLINE },
	{ "int",		INT },
	{ "int8_t",		CHAR },
	{ "int16_t",		SHORT },
	{ "int32_t",		LONG },
	{ "int64_t",		LLONG },
	{ "long",		LONG },
	{ "LONG64",		LLONG },
	{ "mutable",		NONE },
	{ "namespace",		NAMESPACE },
	{ "new",		NONE },
	{ "NULL",		null },
	{ "operator",		OPERATOR },
	{ "private",		PRIVATE },
	{ "protected",		PROTECTED },
	{ "public",		PUBLIC },
	{ "register",		REGISTER },
	{ "reinterpret_cast",	 NONE },
	{ "restrict",		NONE },
	{ "return",		RETURN },
	{ "short",		SHORT },
	{ "signed",		SIGNED },
	{ "size_t",		SIZE },
	{ "sizeof",		SIZEOF },
	{ "static",		STATIC },
	{ "static_cast",	NONE },
	{ "struct",		STRUCT },
	{ "switch",		SWITCH },
	{ "template",		TEMPLATE },
	{ "this",		NONE },
	{ "throw",		NONE },
	{ "time_t",		TIME },
	{ "true",		CTRUE },
	{ "typedef",		TYPEDEF },
	{ "typeid",		NONE },
	{ "typename",		TYPENAME },
	{ "ULONG64",		ULLONG },
	{ "union",		UNION },
	{ "unsigned",		UNSIGNED },
	{ "using",		USING },
	{ "virtual",		VIRTUAL },
	{ "void",		VOID },
	{ "volatile",		VOLATILE },
	{ "wchar_t",		WCHAR },
	{ "while",		WHILE },

	{ "operator!",		NONE },
	{ "operator~",		NONE },
	{ "operator=",		NONE },
	{ "operator+=",		NONE },
	{ "operator-=",		NONE },
	{ "operator*=",		NONE },
	{ "operator/=",		NONE },
	{ "operator%=",		NONE },
	{ "operator&=",		NONE },
	{ "operator^=",		NONE },
	{ "operator|=",		NONE },
	{ "operator<<=",	NONE },
	{ "operator>>=",	NONE },
	{ "operator||",		NONE },
	{ "operator&&",		NONE },
	{ "operator|",		NONE },
	{ "operator^",		NONE },
	{ "operator&",		NONE },
	{ "operator==",		NONE },
	{ "operator!=",		NONE },
	{ "operator<",		NONE },
	{ "operator<=",		NONE },
	{ "operator>",		NONE },
	{ "operator>=",		NONE },
	{ "operator<<",		NONE },
	{ "operator>>",		NONE },
	{ "operator+",		NONE },
	{ "operator-",		NONE },
	{ "operator*",		NONE },
	{ "operator/",		NONE },
	{ "operator%",		NONE },
	{ "operator++",		NONE },
	{ "operator--",		NONE },
	{ "operator->",		NONE },
	{ "operator[]",		NONE },
	{ "operator()",		NONE },

	{ "mustUnderstand",	MUSTUNDERSTAND },

	{ "SOAP_ENV__Header",	ID },
	{ "dummy",		ID },
	{ "soap_header",	ID },

	{ "SOAP_ENV__Fault",	ID },
	{ "SOAP_ENV__Code",	ID },
	{ "SOAP_ENV__Subcode",	ID },
	{ "SOAP_ENV__Reason",	ID },
	{ "SOAP_ENV__Text",	ID },
	{ "SOAP_ENV__Detail",	ID },
	{ "SOAP_ENV__Value",	ID },
	{ "SOAP_ENV__Node",	ID },
	{ "SOAP_ENV__Role",	ID },
	{ "faultcode",		ID },
	{ "faultstring",	ID },
	{ "faultactor",		ID },
	{ "detail",		ID },
	{ "__type",		ID },
	{ "fault",		ID },
	{ "__any",		ID },

	{ "_QName",		TYPE },
	{ "_XML",		TYPE },
	{ "std::string",	TYPE },
	{ "std::wstring",	TYPE },

	{ "/*?*/",		NONE },

	{ 0,			0 }
};

/*
init - initialize symbol table with predefined keywords
*/
void init()
{ struct Keyword *k;
  for (k = keywords; k->s; k++)
    install(k->s, k->t);
}
