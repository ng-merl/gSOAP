/*	symbol2.c

	Module for symbol table handling, type graph handling, and code generation

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
#include "soapcpp2_yacc.h"

char *envURI = "http://schemas.xmlsoap.org/soap/envelope/";
char *encURI = "http://schemas.xmlsoap.org/soap/encoding/";
char *xsiURI = "http://www.w3.org/2001/XMLSchema-instance";
char *xsdURI = "http://www.w3.org/2001/XMLSchema";

static	Symbol *symlist = (Symbol*) 0;	/* pointer to linked list of symbols */
static	Symbol *nslist = (Symbol*) 0;	/* pointer to linked list of namespace prefix symbols */

static	Tnode *Tptr[TYPES];

Service *services = NULL;

FILE *fout, *fhead, *fclient, *fserver, *fheader;

static int typeNO = 0;	/* unique no. assigned to all types */

/*
install - add new symbol
*/
Symbol *
install(const char *name, Token token)
{ Symbol *p;
  p = (Symbol*)emalloc(sizeof(Symbol));
  p->name = emalloc(strlen(name)+1);
  strcpy(p->name, name);
  p->token = token;
  p->next = symlist;
  symlist = p;
  return p;
}

/*
lookup - search for an identifier's name. If found, return pointer to symbol table entry. Return pointer 0 if not found.
*/
Symbol *
lookup(const char *name)
{ Symbol *p;
  for (p = symlist; p; p = p->next)
    if (!strcmp(p->name, name))
      return p;
  return NULL;
}

/*
gensym - generate new symbol from base name
*/
Symbol *
gensym(const char *base)
{ static int num = 1;
  int i;
  char *b, buf[1024];
  strcpy(buf, base);
  b = buf+strlen(base);
  for (i = 0; i < num; i++);
    *b++ = '_';
  *b = '\0';
  num++;
  return install(buf, ID);
}

/*
gensymidx - generate new symbol from base name and index
*/
Symbol *
gensymidx(const char *base, int idx)
{ char buf[1024];
  sprintf(buf, "%s%d", base, idx);
  return install(buf, ID);
}

/*
mktable - make a new symbol table incorporating a pointer to a previous table
*/
Table*
mktable(Table *table)
{	Table	*p;
	p = (Table*)emalloc(sizeof(Table));
	p->sym = lookup("/**/");
	p->list = (Entry*) 0;
	if (table == (Table*) 0)
		p->level = INTERNAL;
	else	p->level = table->level+1;
	p->prev = table;
	return p;
}

/*
mkmethod - make a new method by calling mktype
*/
Tnode*
mkmethod(Tnode *ret, Table *args)
{	FNinfo *fn = (FNinfo*)emalloc(sizeof(FNinfo));
	fn->ret = ret;
	fn->args = args;
	return mktype(Tfun, fn, 0);
}

/*
freetable - free space by removing a table
*/
void
freetable(Table *table)
{	Entry	*p, *q;
	if (table == (Table*) 0)
		return;
	for (p = table->list; p != (Entry*) 0; p = q) {
		q = p->next;
		free(p);
	}
	free(table);
}

/*
unlinklast - unlink last entry added to table
*/
Entry *
unlinklast(Table *table)
{	Entry	**p, *q;
	if (table == (Table*)0)
		return (Entry*)0;
	for (p = &table->list; *p != (Entry*)0 && (*p)->next != (Entry*)0;
	     p = &(*p)->next);
	q = *p;
	*p = (Entry*)0;
	return q;
}

/*
enter - enter a symbol in a table. Error if already in the table
*/
Entry	*
enter(table, sym)
Table	*table;
Symbol	*sym;
{ Entry	*p, *q = (Entry*) 0;
  for (p = table->list; p != (Entry*) 0; q = p, p = p->next)
    if (p->sym == sym && p->info.typ->type != Tfun)
    { sprintf(errbuf, "Duplicate declaration of %s (line %d)", sym->name, p->lineno);
      semerror(errbuf);
      return p;
    }
  p = (Entry*) emalloc(sizeof(Entry));
  p->sym = sym;
  p->info.typ = (Tnode*)0;
  p->info.sto = Snone;
  p->info.hasval = False;
  p->info.minOccurs = 1;
  p->info.maxOccurs = 1;
  p->info.offset = 0;
  p->level = table->level;
  p->lineno = yylineno;
  p->next = (Entry*) 0;
  if (q == (Entry*) 0)
    table->list = p;
  else	q->next = p;
  return p;
}

/*
entry - return pointer to table entry of a symbol
*/
Entry	*entry(table, sym)
Table	*table;
Symbol	*sym;
{ Table	*t;
  Entry	*p;
  for (t = table; t; t = t->prev)
    for (p = t->list; p; p = p->next)
      if (p->sym == sym)
	return p;
  return NULL;
}

Entry *
enumentry(Symbol *sym)
{ Table	*t;
  Entry	*p, *q;
  for (t = enumtable; t; t = t->prev)
    for (p = t->list; p; p = p->next)
      if (q = entry(p->info.typ->ref, sym))
	return q;
  return NULL;
}

char *c_ident(Tnode*);
char *c_storage(Storage);
char *c_init(Entry*);
char *c_type(Tnode*);
char *c_type_id(Tnode*, char*);
char *xsi_type(Tnode*);
char *the_type(Tnode*);
char *wsdl_type(Tnode*, char*);
char *base_type(Tnode*, char*);
char *xml_tag(Tnode*);
char *ns_convert(char*);
char *ns_remove(char*);
char *ns_overridden(Table*, Entry*);

int has_ns(Tnode*);
int has_ns_eq(char*, char*);
int has_offset(Tnode*);
int is_response(Tnode*);
Entry *get_response(Tnode*);
int is_primitive_or_string(Tnode*);
int is_primitive(Tnode*);
Entry *is_dynamic_array(Tnode*);
int is_binary(Tnode*);
int is_hexBinary(Tnode*);
int is_string(Tnode*);
int is_wstring(Tnode*);
int get_dimension(Tnode*);
char *has_soapref(Tnode*);

char *xsi_type_Tarray(Tnode*);    
char *xsi_type_Darray(Tnode*);    

void def_table(Table *,Tnode *);
void out_generate(Tnode *);
int no_of_var(Tnode*);
char *pointer_stuff(Tnode*);
void in_defs(Table*);
void in_defs2(Table*);
void out_defs(Table*);
void in_attach(Table*);
void out_attach(Table*);
void serialize(Tnode*);
void mark(Tnode*);
void defaults(Tnode*);
void soap_put(Tnode*);
void soap_out(Tnode*);
void soap_out_Darray(Tnode *);
void soap_get(Tnode*);
void soap_in(Tnode*); 
void soap_in_Darray(Tnode *);
void soap_instantiate_class(Tnode *);
int get_Darraydims(Tnode *typ);

void function_input_output(Table*);
void generate_proto(Table*, Entry*);
void generate_call(Table*, Entry*);
void generate_server(Table*, Entry*);
void generate_header(Table*);
void generate_schema(Table*);
void gen_schema(FILE*,Table*,char*,char*,int,int,char*,char*);
void gen_wsdl(FILE*,Table*,char*,char*,char*,char*,char*,char*);
void gen_proxy(FILE*,Table*,char*,char*,char*,char*,char*,char*);
void gen_data(char*,Table*,char*,char*,char*,char*,char*,char*);
FILE *gen_env(char*,char*,int,Table*,char*,char*,char*,char*,char*,char*);
void gen_field(FILE*,int,Entry*);
void gen_val(FILE*,int,Tnode*);
void gen_atts(FILE*,Table*);

/*
mktype - make a (new) type with a reference to additional information and the
width in bytes required to store objects of that type. A pointer to the
type is returned which can be compared to check if types are identical.
*/
Tnode *
mktype(Type type, void *ref, int width)
{	Tnode	*p;
	if (type != Tstruct && type != Tclass)
	for (p = Tptr[type]; p != (Tnode*) 0; p = p->next)
		if (p->ref == ref && p->sym == (Symbol*) 0 && p->width == width && p->transient == transient)
			return p;	/* type alrady exists in table */
	p = (Tnode*)emalloc(sizeof(Tnode));	/* install new type */
	p->type = type;
	p->ref = ref;
	p->id = lookup("/**/");
	p->base = NULL;
	p->sym = (Symbol*)0;
	p->response = (Entry*)0;
	p->width = width;
	p->generated = False;
	p->wsdl = False;
	p->next = Tptr[type];
	p->transient = transient;
	Tptr[type] = p;
	return p;
}

Tnode *
mksymtype(Tnode *typ, Symbol *sym)
{	Tnode *p;
	p = (Tnode*)emalloc(sizeof(Tnode));	/* install new type */
	p->type = typ->type;
	p->ref = typ->ref;
	p->id = typ->id;
	p->sym = sym;
	p->width = typ->width;
	p->generated = False;
	p->wsdl = False;
	p->next = Tptr[typ->type];
	p->transient = transient;
	Tptr[typ->type] = p;
	return p;
}

/*	DO NOT REMOVE OR ALTER (SEE LICENCE AGREEMENT AND COPYING.txt)	*/
void
copyrightnote(FILE *fd, char *fn)
{ fprintf(fd, "/* %s\n   Generated by gSOAP "VERSION" from %s\n   Copyright (C) 2001-2003 Genivia inc.\n   All Rights Reserved.\n*/", fn, filename);
}

void
ident(FILE *fd, char *fn)
{ time_t t = time(NULL), *p = &t;
  char tmp[256];
  strftime(tmp, 256, "%Y-%m-%d %H:%M:%S GMT", gmtime(p));
  fprintf(fd, "\n\nSOAP_SOURCE_STAMP(\"@(#) %s ver "VERSION" %s\")\n", fn, tmp);
}

void
compile(Table *table)
{	Entry *p;
	Pragma *pragma;
	char *s;
	char base[1024];
	char soapStub[1024];
	char soapH[1024];
	char soapC[1024];
	char soapClient[1024];
	char soapServer[1024];
	char pathsoapStub[1024];
	char pathsoapH[1024];
	char pathsoapC[1024];
	char pathsoapClient[1024];
	char pathsoapServer[1024];
    
	DBGLOG(fprintf(stderr,"\n IN dumptable."));
	if (*dirpath)
	  fprintf(stderr, "Using project directory path: %s\n", dirpath);
	typeNO = 0;

	strcpy(base, prefix);
	if (cflag)
		s = ".c";
	else
		s = ".cpp";
	strcpy(soapStub, base);
	strcat(soapStub, "Stub.h");
	strcpy(pathsoapStub, dirpath);
	strcat(pathsoapStub, soapStub);
	strcpy(soapH, base);
	strcat(soapH, "H.h");
	strcpy(pathsoapH, dirpath);
	strcat(pathsoapH, soapH);
	strcpy(soapC, base);
	strcat(soapC, "C");
	strcat(soapC, s);
	strcpy(pathsoapC, dirpath);
	strcat(pathsoapC, soapC);
	strcpy(soapClient, base);
	strcat(soapClient, "Client");
	strcat(soapClient, s);
	strcpy(pathsoapClient, dirpath);
	strcat(pathsoapClient, soapClient);
	strcpy(soapServer, base);
	strcat(soapServer, "Server");
	strcat(soapServer, s);
	strcpy(pathsoapServer, dirpath);
	strcat(pathsoapServer, soapServer);
	fprintf(stderr, "Saving %s\n", pathsoapStub);
	fheader=fopen(pathsoapStub, "w");
	if (!fheader)
		execerror("Cannot write to file");
	copyrightnote(fheader, soapStub);
	fprintf(stderr, "Saving %s\n", pathsoapH);
	fhead=fopen(pathsoapH,"w");
	if (!fhead)
		execerror("Cannot write to file");
	copyrightnote(fhead, soapH);
	fprintf(stderr, "Saving %s\n", pathsoapC);
	fout=fopen(pathsoapC,"w");
	if (!fout)
		execerror("Cannot write to file");
	copyrightnote(fout, soapC);
	fprintf(stderr, "Saving %s\n", pathsoapClient);
        fclient=fopen(pathsoapClient,"w");
	if (!fclient)
		execerror("Cannot write to file");
	copyrightnote(fclient, soapClient);
	fprintf(stderr, "Saving %s\n", pathsoapServer);
        fserver=fopen(pathsoapServer,"w");
	if (!fserver)
		execerror("Cannot write to file");
	copyrightnote(fserver, soapServer);

	fprintf(fhead,"\n#ifndef %sH_H\n#define %sH_H", prefix, prefix);
	for (pragma = pragmas; pragma; pragma = pragma->next)
	  fprintf(fhead,"\n%s", pragma->pragma);
	fprintf(fhead,"\n#include \"stdsoap2.h\"");
	fprintf(fhead,"\n#include \"%sStub.h\"", prefix);
	if (cflag)
	  fprintf(fhead,"\n#ifdef __cplusplus\nextern \"C\" {\n#endif");
	fprintf(fheader,"\n#ifndef %sStub_H\n#define %sStub_H", prefix, prefix);
	if (cflag)
	  fprintf(fheader,"\n#ifdef __cplusplus\nextern \"C\" {\n#endif");
	generate_header(table);
	generate_schema(table);
	fprintf(fout,"\n#include \"%sH.h\"", prefix);
	if (cflag)
	  fprintf(fout,"\n#ifdef __cplusplus\nextern \"C\" {\n#endif");
	ident(fout, soapC);
        fprintf(fclient,"\n#include \"%sH.h\"", prefix);
	if (cflag)
	  fprintf(fclient,"\n#ifdef __cplusplus\nextern \"C\" {\n#endif");
	ident(fclient, soapClient);
        fprintf(fserver,"\n#include \"%sH.h\"", prefix);
	if (cflag)
	  fprintf(fserver,"\n#ifdef __cplusplus\nextern \"C\" {\n#endif");
	ident(fserver, soapServer);

	DBGLOG(fprintf(stderr,"\n Calling function_input_output( )."));
        function_input_output(table);
	DBGLOG(fprintf(stderr,"\n Completed function_input_output( )."));

	fprintf(fout,"\n\n#ifndef WITH_NOGLOBAL");
        if (entry(classtable, lookup("SOAP_ENV__Header"))->info.typ->type == Tstruct)
	  fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_serializeheader(struct soap *soap)\n{\n\tsoap_serialize_SOAP_ENV__Header(soap, soap->header);\n}");
	else
	  fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_serializeheader(struct soap *soap)\n{\n\tif (soap->header)\n\t\tsoap->header->soap_serialize(soap);\n}");
	fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_putheader(struct soap *soap)\n{\n\tif (soap->header)\n\t{\tsoap->is_in_header = 1;\n\t\tsoap_out_SOAP_ENV__Header(soap, \"SOAP-ENV:Header\", 0, soap->header, NULL);\n\t\tsoap->is_in_header = 0;\n\t}\n\treturn SOAP_OK;\n}");
	fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_getheader(struct soap *soap)\n{\n\tsoap->is_in_header = 1;\n\tsoap->header = soap_in_SOAP_ENV__Header(soap, \"SOAP-ENV:Header\", NULL, NULL);\n\tsoap->is_in_header = 0;\n\treturn soap->header == NULL;\n}");
        if (entry(classtable, lookup("SOAP_ENV__Fault"))->info.typ->type == Tstruct)
	{ fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_fault(struct soap *soap)\n{\n\tif (!soap->fault)\n\t{\tsoap->fault = (struct SOAP_ENV__Fault*)soap_malloc(soap, sizeof(struct SOAP_ENV__Fault));\n\t\tsoap_default_SOAP_ENV__Fault(soap, soap->fault);\n\t}\n\tif (soap->version == 2 && !soap->fault->SOAP_ENV__Code)\n\t{\tsoap->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code*)soap_malloc(soap, sizeof(struct SOAP_ENV__Code));\n\t\tsoap_default_SOAP_ENV__Code(soap, soap->fault->SOAP_ENV__Code);\n\t}\n}");
	  fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_serializefault(struct soap *soap)\n{\n\tsoap_serialize_SOAP_ENV__Fault(soap, soap->fault);\n}");
	}
	else
	{ fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_fault(struct soap *soap)\n{\n\tif (!soap->fault)\n\t{\tsoap->fault = soap_new_SOAP_ENV__Fault(soap, -1);\n\t\tsoap->fault->soap_default(soap);\n\t}\n\tif (soap->version == 2 && !soap->fault->SOAP_ENV__Code)\n\t{\tsoap->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code*)soap_malloc(soap, sizeof(struct SOAP_ENV__Code));\n\t\tsoap_default_SOAP_ENV__Code(soap, soap->fault->SOAP_ENV__Code);\n\t}\n}");
	  fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_serializefault(struct soap *soap)\n{\n\tif (soap->fault)\n\t\tsoap->fault->soap_serialize(soap);\n}");
	}
	fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_putfault(struct soap *soap)\n{\n\treturn soap_out_SOAP_ENV__Fault(soap, \"SOAP-ENV:Fault\", 0, soap->fault, NULL);\n}");
	fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_getfault(struct soap *soap)\n{\n\treturn (soap->fault = soap_in_SOAP_ENV__Fault(soap, \"SOAP-ENV:Fault\", NULL, NULL)) == NULL;\n}");
	fprintf(fout,"\n\nSOAP_FMAC1 const char ** SOAP_FMAC2 soap_faultcode(struct soap *soap)\n{\n\tsoap_fault(soap);\n\tif (soap->version == 2)\n\t\treturn (const char**)&soap->fault->SOAP_ENV__Code->SOAP_ENV__Value;\n\treturn (const char**)&soap->fault->faultcode;\n}");
	fprintf(fout,"\n\nSOAP_FMAC1 const char ** SOAP_FMAC2 soap_faultstring(struct soap *soap)\n{\n\tsoap_fault(soap);\n\tif (soap->version == 2)\n\t\treturn (const char**)&soap->fault->SOAP_ENV__Reason;\n\treturn (const char**)&soap->fault->faultstring;\n}");
	fprintf(fout,"\n\nSOAP_FMAC1 const char ** SOAP_FMAC2 soap_faultdetail(struct soap *soap)\n{\n\tsoap_fault(soap);");
	if (has_Detail_string())
	  fprintf(fout,"\n\tif (soap->version == 2)\n\t\treturn (const char**)&soap->fault->SOAP_ENV__Detail;");
	if (has_detail_string())
	  fprintf(fout,"\n\treturn (const char**)&soap->fault->detail;\n}");
        else
	  fprintf(fout,"\n\treturn NULL;\n}");
	fprintf(fout,"\n\n#endif");

	fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_getindependent(struct soap *soap)\n{");
	fprintf(fout,"\n\tfor (;;)");
        fprintf(fout,"\n\t{\tif (soap_peek_element(soap))\n\t\t\tbreak;");
	fprintf(fout,"\n\t\tswitch (soap_lookup_type(soap, soap->id))\n\t\t{");
	DBGLOG(fprintf(stderr,"\n Calling in_defs( )."));
	fflush(fout);
	in_defs(table);
	DBGLOG(fprintf(stderr,"\n Completed in_defs( )."));
        fprintf(fout,"\n\t\tdefault:\n\t\t\tif (!*soap->id)\n\t\t\t{\tif (soap_ignore_element(soap))\n\t\t\t\t\tbreak;\n\t\t\t\telse\n\t\t\t\t\tcontinue;\n\t\t\t}");
	fflush(fout);
	in_defs2(table);
        fprintf(fout,"\n\t\t}");
        fprintf(fout,"\n\t\tif (soap->error)\n\t\t\tbreak;");
        fprintf(fout,"\n\t}");
        fprintf(fout,"\n\tif (soap->error == SOAP_NO_TAG)");
        fprintf(fout,"\n\t\tsoap->error = SOAP_OK;");
        fprintf(fout,"\n\treturn soap->error;");
        fprintf(fout,"\n}");

	fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_putindependent(struct soap *soap)\n{\n\tint i;\n\tstruct soap_plist *pp;");
	fprintf(fout,"\n\tif (soap->version != 1 || (soap->mode & SOAP_XML_GRAPH) || (soap->mode & SOAP_XML_TREE))\n\t\treturn SOAP_OK;");
	fprintf(fout,"\n\tfor (i = 0; i < SOAP_PTRHASH; i++)");
	fprintf(fout,"\n\t\tfor (pp = soap->pht[i]; pp; pp = pp->next)");
	fprintf(fout,"\n\t\t\tif ((soap->mode & SOAP_IO_LENGTH) ? pp->mark1 == 2 : pp->mark2 == 2)");
	fprintf(fout,"\n\t\t\t\tswitch (pp->type)\n\t\t\t\t{");
	fflush(fout);
        out_defs(table);
	fprintf(fout,"\n\t\t\t\t}\n\t\t\n\treturn SOAP_OK;\n}");

	fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_getattachments(struct soap *soap)\n{");
	fprintf(fout,"\n\tif (!(soap->mode & SOAP_ENC_DIME))\n\t\treturn SOAP_OK;");
	fprintf(fout,"\n\twhile (soap->dime_flags & SOAP_DIME_CF)\n\t{\tif (soap_getdimehdr(soap))\n\t\t\treturn soap->error;\n\t\tif (soap_move(soap, soap->dime_size))\n\t\t\treturn soap->error = SOAP_EOF;\n\t}");
	fprintf(fout,"\n\tif (soap_move(soap, ((soap->dime_size+3)&-4)-soap_tell(soap)))\n\t\treturn soap->error = SOAP_EOF;");
	fprintf(fout,"\n\tfor (;;)");
        fprintf(fout,"\n\t{\tif (soap_getdime(soap) || !soap->dime_id)\n\t\t\tbreak;");
	fprintf(fout,"\n\t\tswitch (soap_lookup_type(soap, soap->dime_id))\n\t\t{");
	fflush(fout);
        in_attach(table);
        fprintf(fout,"\n\t\tdefault:\n\t\t\tsoap->error = SOAP_DIME_ERROR;");
        fprintf(fout,"\n\t\t}");
        fprintf(fout,"\n\t\tif (soap->error)\n\t\t\tbreak;");
        fprintf(fout,"\n\t}");
        fprintf(fout,"\n\tif (soap->error == SOAP_EOD)");
        fprintf(fout,"\n\t\tsoap->error = SOAP_OK;");
        fprintf(fout,"\n\treturn soap->error;");
        fprintf(fout,"\n}");

	fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_putattachments(struct soap *soap)\n{\n\tint i;\n\tstruct soap_plist *pp;");
	fprintf(fout,"\n\tif (!(soap->mode & SOAP_ENC_DIME))\n\t\treturn SOAP_OK;");
	fprintf(fout,"\n\tif (soap_send_raw(soap, \"\\0\\0\\0\", -(int)soap->count&3))\n\t\treturn soap->error;");
	fprintf(fout,"\n\tfor (i = 0; i < SOAP_PTRHASH; i++)");
	fprintf(fout,"\n\t\tfor (pp = soap->pht[i]; pp; pp = pp->next)");
	fprintf(fout,"\n\t\t\tif (pp->mark2 == 3)");
	fprintf(fout,"\n\t\t\t\tswitch (pp->type)\n\t\t\t\t{");
	fflush(fout);
        out_attach(table);
	fprintf(fout,"\n\t\t\t\t}\n\t\t\n\treturn SOAP_OK;\n}");
	
	if (Tptr[Tclass])
	{

	if (cflag)
		semwarn("Option -c conflicts with the use of class declarations");

	fprintf(fhead,"\n\nSOAP_FMAC1 void * SOAP_FMAC2 soap_instantiate(struct soap*, int, const char*, const char*);");
	fprintf(fout,"\n\nSOAP_FMAC1 void * SOAP_FMAC2 soap_instantiate(struct soap *soap, int t, const char *type, const char *arrayType)\n{\n\tswitch (t)\n\t{");
	if (classtable)
	  for (p = classtable->list; p; p = p->next)
	    if (p->info.typ->type == Tclass && !is_transient(p->info.typ))
	      fprintf(fout,"\n\tcase SOAP_TYPE_%s:\n\t\treturn (void*)soap_instantiate_%s(soap, -1, type, arrayType);", c_ident(p->info.typ), c_ident(p->info.typ));
	if (typetable)
	  for (p = typetable->list; p; p = p->next)
	    if (p->info.typ->type == Tclass && is_external(p->info.typ))
	      fprintf(fout,"\n\tcase SOAP_TYPE_%s:\n\t\treturn (void*)soap_instantiate_%s(soap, -1, type, arrayType);", c_ident(p->info.typ), c_ident(p->info.typ));

	fprintf(fout,"\n\t}\n\treturn NULL;\n}");

	fprintf(fhead,"\n\n#undef soap_destroy\n#define soap_destroy(soap) soap_delete(soap, NULL)");
	fprintf(fhead,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_delete(struct soap*, void*);");
	fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_delete(struct soap *soap, void *p)");
	fprintf(fout,"\n{\n\tstruct soap_clist **cp;");
	fprintf(fout,"\n\tcp = &soap->clist;");
	fprintf(fout,"\n\twhile (*cp)");
	fprintf(fout,"\n\t{\tif (!p || p == (*cp)->ptr)");
	fprintf(fout,"\n\t\t{\tstruct soap_clist *q = *cp;");
	fprintf(fout,"\n\t\t\t*cp = q->next;");
	fprintf(fout,"\n\t\t\tswitch (q->type)\n\t\t\t{");
	if (classtable)
	  for (p = classtable->list; p; p = p->next)
	    if (p->info.typ->type == Tclass && !is_transient(p->info.typ))
	    { fprintf(fout,"\n\t\t\tcase SOAP_TYPE_%s:\n\t\tDBGLOG(TEST, SOAP_MESSAGE(fdebug, \"soap_delete(%s, %%d)\\n\", q->size));", c_ident(p->info.typ), c_ident(p->info.typ));
	      fprintf(fout,"\n\t\t\t\tif (q->size < 0)\n\t\t\t\t\tdelete (%s*)q->ptr;\n\t\t\t\telse\n\t\t\t\t\tdelete[] (%s*)q->ptr;\n\t\t\t\tbreak;", c_type(p->info.typ), c_type(p->info.typ));
	    }
	if (typetable)
	  for (p = typetable->list; p; p = p->next)
	    if (p->info.typ->type == Tclass && is_external(p->info.typ))
	    { fprintf(fout,"\n\t\t\tcase SOAP_TYPE_%s:\n\t\t\t\tDBGLOG(TEST, SOAP_MESSAGE(fdebug, \"soap_delete(%s, %%p, %%d)\\n\", q->ptr, q->size));", c_ident(p->info.typ), c_ident(p->info.typ));
	      fprintf(fout,"\n\t\t\t\tif (q->size < 0)\n\t\t\t\t\tdelete (%s*)q->ptr;\n\t\t\t\telse\n\t\t\t\t\tdelete[] (%s*)q->ptr;\n\t\t\t\tbreak;", c_type(p->info.typ), c_type(p->info.typ));
	    }
	fprintf(fout,"\n\t\t\t}");
	fprintf(fout,"\n\t\t\tfree(q);");
	fprintf(fout,"\n\t\t\tif (p)\n\t\t\t\treturn;");
	fprintf(fout,"\n\t\t\tcp = &soap->clist;");
	fprintf(fout,"\n\t\t}");
	fprintf(fout,"\n\t\telse\n\t\t\tcp = &(*cp)->next;");
	fprintf(fout,"\n\t}");
	fprintf(fout,"\n}");

	fprintf(fhead,"\n\nSOAP_FMAC1 void* SOAP_FMAC2 soap_class_id_enter(struct soap*, const char*, void*, int, const char*, const char*);");
	fprintf(fout,"\n\nSOAP_FMAC1 void* SOAP_FMAC2 soap_class_id_enter(struct soap *soap, const char *id, void *p, int t, const char *type, const char *arrayType)");
	fprintf(fout, "\n{\tstruct soap_ilist *ip;");
	fprintf(fout, "\n\tDBGLOG(TEST,SOAP_MESSAGE(fdebug, \"Class id enter id='%%s' t=%%d loc=%%p type='%%s' arrayType='%%s'\\n\", id, t, p, type?type:\"\", arrayType?arrayType:\"\"));");
	fprintf(fout, "\n\tsoap->alloced = 0;\n\tif (*id == '\\0')\n\t{\tif (!p)\n\t\t\treturn soap_instantiate(soap, t, type, arrayType);\n\t\telse\n\t\t\treturn p;\n\t}");
	fprintf(fout, "\n\tip = soap_lookup(soap, id);");
	fprintf(fout, "\n\tif (!ip)");
	fprintf(fout, "\n\t{\tip = soap_enter(soap, id);");
	fprintf(fout, "\n\t\tip->link = NULL;\n\t\tip->copy = NULL;");
	fprintf(fout, "\n\t\tif (!p)\n\t\t\tp = soap_instantiate(soap, t, type, arrayType);");
	fprintf(fout, "\n\t\tip->ptr = p;\n\t\tip->level = 0;");
	fprintf(fout, "\n\t}\n\telse if (ip->ptr)");
	fprintf(fout, "\n\t{\tif (p)\n\t\t{\tsoap->error = SOAP_MULTI_ID;\n\t\t\treturn NULL;\n\t\t}");
	fprintf(fout, "\n\t}\n\telse");
	fprintf(fout, "\n\t{\tif (!p)\n\t\t\tp = soap_instantiate(soap, t, type, arrayType);");
	fprintf(fout, "\n\t\tip->ptr = p;\n\t\tip->level = 0;");
	fprintf(fout, "\n\t\tif (!soap->blist)\n\t\t\tsoap_resolve_ptr(ip);");
	fprintf(fout, "\n\t}\n\treturn ip->ptr;\n}");

	}

	DBGLOG(fprintf(stderr,"\n Calling def_table( )."));
        def_table(table,0);
	DBGLOG(fprintf(stderr,"\n Completed def_table( )."));
	if (cflag)
	  fprintf(fout,"\n#ifdef __cplusplus\n}\n#endif");
	fprintf(fout, "\n\n/* end of %s */\n", soapC);
        fclose(fout);
	if (cflag)
	  fprintf(fhead,"\n#ifdef __cplusplus\n}\n#endif");
	fprintf(fhead, "\n#endif");
	fprintf(fhead, "\n\n/* end of %s */\n", soapH);
	fclose(fhead);
	if (cflag)
	  fprintf(fheader,"\n#ifdef __cplusplus\n}\n#endif");
	fprintf(fheader, "\n#endif");
	fprintf(fheader, "\n\n/* end of %s */\n", soapStub);
	fclose(fheader);
	if (cflag)
	  fprintf(fclient,"\n#ifdef __cplusplus\n}\n#endif");
	fprintf(fclient, "\n\n/* end of %s */\n", soapClient);
        fclose(fclient);
	if (cflag)
	  fprintf(fserver,"\n#ifdef __cplusplus\n}\n#endif");
	fprintf(fserver, "\n\n/* end of %s */\n", soapServer);
 	fclose(fserver);
}

void
gen_class(FILE *fd, Tnode *typ)
{ Entry *Eptr;
  char *x = xsi_type(typ);
  typ->generated = True;
  if (!is_external(typ) && !is_transient(typ))
    fprintf(fd, "#ifndef _SOAP_%s\n#define _SOAP_%s\n", c_ident(typ), c_ident(typ));
  if (is_transient(typ) && typ->ref)
    fprintf(fd, "/* transient type: */\n");
  else if (is_hexBinary(typ))
    fprintf(fd, "/* hexBinary schema type: */\n");
  else if (is_binary(typ))
    fprintf(fd, "/* Base64 schema type: */\n");
  else if (is_dynamic_array(typ))
    if (has_ns(typ) || is_untyped(typ))
      fprintf(fd, "/* Vector %s schema type: */\n", x);
    else
      fprintf(fd, "/* Array of %s schema type: */\n", x);
  else if (is_primclass(typ))
    fprintf(fd, "/* Primitive %s schema type: */\n", x);
  else if (!strcmp(typ->id->name, "SOAP_ENV__Header"))
    fprintf(fd, "/* SOAP Header: */\n");
  else if (!strcmp(typ->id->name, "SOAP_ENV__Fault"))
    fprintf(fd, "/* SOAP Fault: */\n");
  else if (!strcmp(typ->id->name, "SOAP_ENV__Code"))
    fprintf(fd, "/* SOAP Fault Code: */\n");
  fflush(fd);
  if (typ->type == Tstruct)
  { { DBGLOG(fprintf(stderr,"\nstruct %s\n", typ->id->name));
      if (typ->ref)
      { fprintf(fd, "struct %s\n{", typ->id->name );
        for (Eptr = ((Table*)typ->ref)->list; Eptr; Eptr = Eptr->next)
        { fprintf(fd, "\n\t%s", c_storage(Eptr->info.sto));
	  if (Eptr->info.typ->type == Tclass && !is_external(Eptr->info.typ) && Eptr->info.typ->generated == False || Eptr->info.typ->ref && ((Tnode*)Eptr->info.typ->ref)->type == Tclass && !is_external(Eptr->info.typ->ref) && ((Tnode*)Eptr->info.typ->ref)->generated == False)
	    fprintf(fd, "class ");
          fprintf(fd, "%s;", c_type_id(Eptr->info.typ,Eptr->sym->name));
	  if (Eptr->info.sto & Sconst)
	    fprintf(fd, "\t/* const field cannot be deserialized */");
	  else if (is_external(Eptr->info.typ))
	    fprintf(fd, "\t/* external */");
	  else if (is_transient(Eptr->info.typ))
	    fprintf(fd, "\t/* transient */");
	  else if (Eptr->info.sto & Sattribute)
	    fprintf(fd, "\t/* attribute */");
	  else if (Eptr->info.sto & SmustUnderstand)
	    fprintf(fd, "\t/* mustUnderstand */");
	  if (!is_dynamic_array(typ) && !is_primclass(typ))
	  { if (!strncmp(Eptr->sym->name, "__size", 6))
	    { if (!Eptr->next || Eptr->next->info.typ->type != Tpointer)
              { sprintf(errbuf, "Field '__size' is not followed by a pointer field in struct '%s'", typ->id->name);
                semwarn(errbuf);
	      }
	    }
	    else if (!strncmp(Eptr->sym->name, "__", 2))
            { sprintf(errbuf, "Field '%s' with leading __ not used in array or binary type struct '%s'", Eptr->sym->name, typ->id->name);
              semwarn(errbuf);
	    }
	  }
	}
        fprintf(fd, "\n};");
      }
      else if (!is_transient(typ))
      { sprintf(errbuf, "struct '%s' is empty", typ->id->name);
        semwarn(errbuf);
      }
    }
  }
  else
  { DBGLOG(fprintf(stderr,"\nclass %s\n", typ->id->name));
    if (typ->ref)
    { fprintf(fd,"class SOAP_CMAC %s", typ->id->name );
      if (typ->base)
        fprintf(fd," : public %s", typ->base->name);
      fprintf(fd,"\n{ public:");
      for (Eptr = ((Table*)typ->ref)->list; Eptr; Eptr = Eptr->next)
      { fprintf(fd,"\n\t%s", c_storage(Eptr->info.sto));
	if (Eptr->info.typ->type == Tclass && !is_external(Eptr->info.typ) && Eptr->info.typ->generated == False || Eptr->info.typ->ref && ((Tnode*)Eptr->info.typ->ref)->type == Tclass && !is_external(Eptr->info.typ->ref) && ((Tnode*)Eptr->info.typ->ref)->generated == False)
	  fprintf(fd, "class ");
	fprintf(fd,"%s", c_type_id(Eptr->info.typ,Eptr->sym->name));
	if (Eptr->info.sto & Sconstobj)
	  fprintf(fd, " const");
	if (Eptr->info.sto & Sabstract)
	  fprintf(fd, " = 0;");
	else
	  fprintf(fd, ";");
	if (Eptr->info.sto & Sconst)
	  fprintf(fd, "\t/* const field cannot be deserialized */");
	else if (is_external(Eptr->info.typ))
	  fprintf(fd, "\t/* external */");
	else if (is_transient(Eptr->info.typ))
	  fprintf(fd, "\t/* transient */");
	else if (Eptr->info.sto & Sattribute)
	  fprintf(fd, "\t/* attribute */");
	else if (Eptr->info.sto & SmustUnderstand)
	  fprintf(fd, "\t/* mustUnderstand */");
	if (!is_dynamic_array(typ) && !is_primclass(typ))
	{ if (!strncmp(Eptr->sym->name, "__size", 6))
	  { if (!Eptr->next || Eptr->next->info.typ->type != Tpointer)
            { sprintf(errbuf, "Field '__size' is not followed by a pointer field in class '%s'", typ->id->name);
              semwarn(errbuf);
	    }
	  }
	  else if (!strncmp(Eptr->sym->name, "__", 2))
          { sprintf(errbuf, "Field '%s' with leading __ not used in array or binary type class '%s'", Eptr->sym->name, typ->id->name);
            semwarn(errbuf);
	  }
        }
      }
      if (!is_transient(typ))
      { fprintf(fd,"\n\tvirtual void soap_default(struct soap*); ");
        fprintf(fd,"\n\tvirtual void soap_serialize(struct soap*) const;");
        fprintf(fd,"\n\tvirtual void soap_mark(struct soap*) const;");
        fprintf(fd,"\n\tvirtual int soap_put(struct soap*, const char*, const char*) const;");
        fprintf(fd,"\n\tvirtual int soap_out(struct soap*, const char*, int, const char*) const;");
        fprintf(fd,"\n\tvirtual void *soap_get(struct soap*, const char*, const char*);");
        fprintf(fd,"\n\tvirtual void *soap_in(struct soap*, const char*, const char*); ");
      }
      fprintf(fd,"\n};");
    }
    else if (!is_transient(typ))
    { sprintf(errbuf, "class '%s' is empty", typ->id->name);
      semwarn(errbuf);
    }
  }
  if (!is_external(typ) && !is_transient(typ))
    fprintf(fd, "\n#endif");
  fflush(fd);
}

void
generate_header(Table *t)
{ Entry *p, *q;
  fprintf(fheader, "\n\n/* Types With Custom (De)serializers: */");
  fflush(fheader);
  if (typetable)
    for (p = typetable->list; p; p = p->next)
    { if (is_external(p->info.typ))
        fprintf(fheader, "\n%s%s;", c_storage(p->info.sto), c_type_id(p->info.typ, p->sym->name));
    }
  fprintf(fheader, "\n\n/* Enumerations */");
  fflush(fheader);
  if (enumtable)
    for (p = enumtable->list; p; p = p->next)
    { fprintf(fheader, "\n");
      if (is_mask(p->info.typ))
        fprintf(fheader, "\n/* Bitmask: */");
      fprintf(fheader, "\n#ifndef _SOAP_%s\n#define _SOAP_%s", c_ident(p->info.typ), c_ident(p->info.typ));
      fprintf(fheader, "\nenum %s {", p->info.typ->id->name);
      if ((Table*)p->info.typ->ref)
      { q = ((Table*)p->info.typ->ref)->list;
        if (q)
        { fprintf(fheader, "%s = "SOAP_LONG_FORMAT, q->sym->name, q->info.val.i);
          for (q = q->next; q; q = q->next)
            fprintf(fheader, ", %s = "SOAP_LONG_FORMAT, q->sym->name, q->info.val.i);
        }
      }
      fprintf(fheader, "};\n#endif");
    }
  fprintf(fheader, "\n\n/* Classes and Structs */");
  fflush(fheader);
  if (classtable)
    for (p = classtable->list; p; p = p->next)
    { fprintf(fheader, "\n\n");
      gen_class(fheader, p->info.typ);
    }
  fprintf(fheader, "\n\n/* Typedefs */");
  fflush(fheader);
  if (typetable)
    for (p = typetable->list; p; p = p->next)
    { if (!is_primitive(p->info.typ) && !is_external(p->info.typ) && !is_string(p->info.typ) && !is_wstring(p->info.typ) && !is_XML(p->info.typ) && !is_transient(p->info.typ))
      { sprintf(errbuf, "typedef '%s' is not a primitive type: schema definition for '%s' in WSDL file output may be invalid", p->sym->name, p->sym->name);
        semwarn(errbuf);
      }
      if (!is_external(p->info.typ))
        fprintf(fheader,"\n%s%s;", c_storage(p->info.sto), c_type_id(p->info.typ, p->sym->name));
    }
  fprintf(fheader, "\n\n/* Variables */");
  fflush(fheader);
  if (t)
    for (p = t->list; p; p = p->next)
      if (p->info.typ->type != Tfun)
        fprintf(fheader,"\n\nextern %s%s;", c_storage(p->info.sto), c_type_id(p->info.typ, p->sym->name));
  fflush(fheader);
}

void
get_namespace_prefixes()
{ Symbol *p, *q;
  int i, n;
  char buf[256];
  if (nslist)
    return;
  for (p = symlist; p; p = p->next)
  { if (*p->name != '~')
    { n = strlen(p->name)-2;
      for (i = 1; i < n; i++)
      { if (p->name[i] == '_' && p->name[i+1] == '_' && p->name[i+2] && p->name[i+2] != '_')
        { strncpy(buf, p->name, i);
          buf[i] = '\0';
	  if (!strcmp(buf, "SOAP_ENV") || !strcmp(buf, "SOAP_ENC") || !strcmp(buf, "xsd") || !strcmp(buf, "xsi"))
	    goto nsnext;
          for (q = nslist; q; q = q->next)
            if (!strcmp(q->name, buf))
              goto nsnext;
          q = (Symbol*)emalloc(sizeof(Symbol));
          q->name = (char*)emalloc(i+1);
	  strcpy(q->name, buf);
	  q->name[i] = '\0';
	  q->next = nslist;
	  nslist = q;
        }
      }
    }
nsnext:
    ;
  }
  q = (Symbol*)emalloc(sizeof(Symbol));
  q->name = "xsd";
  q->next = nslist;
  nslist = q;
  q = (Symbol*)emalloc(sizeof(Symbol));
  q->name = "xsi";
  q->next = nslist;
  nslist = q;
  q = (Symbol*)emalloc(sizeof(Symbol));
  q->name = "SOAP-ENC";
  q->next = nslist;
  nslist = q;
  q = (Symbol*)emalloc(sizeof(Symbol));
  q->name = "SOAP-ENV";
  q->next = nslist;
  nslist = q;
}

void
generate_schema(Table *t)
{ Entry *p;
  Symbol *ns, *ns1;
  char *name = NULL;
  char *URL = NULL;
  char *executable = NULL;
  char *URI = NULL;
  char *encoding = NULL;
  Service *sp, *sq;
  char buf[1024];
  FILE *fd;
  get_namespace_prefixes();
  for (ns = nslist; ns; ns = ns->next)
  { if (!strcmp(ns->name, "SOAP-ENV") || !strcmp(ns->name, "SOAP-ENC") || !strcmp(ns->name, "xsi") || !strcmp(ns->name, "xsd"))
      continue;
    for (sp = services; sp; sp = sp->next)
    { if (!strcmp(sp->ns, ns->name))
	{	name = sp->name;
		URL = sp->URL;
		executable = sp->executable;
		URI = sp->URI;
		encoding = sp->encoding;
		if (name)
		  fprintf(stderr, "Using %s service name: %s\n", ns->name, name);
		if (URL)
		  fprintf(stderr, "Using %s service location: %s\n", ns->name, URL);
		if (executable)
		  fprintf(stderr, "Using %s service executable: %s\n", ns->name, executable);
		if (URI)
		  fprintf(stderr, "Using %s service namespace: %s\n", ns->name, URI);
		if (encoding)
		  fprintf(stderr, "Using %s service encoding: %s\n", ns->name, encoding);
		break;
  	}
    }
    if (!URI)
    { if (name)
      { URI = emalloc(strlen(name)+19);
        strcpy(URI, "http://tempuri.org/");
        strcat(URI, name);
      }
      else
        URI = "http://tempuri.org";
    }
    if (!name)
  	name = "Service";
    if (!URL)
  	URL = "http://location";
    /*
    if (!executable)
    { executable = emalloc(strlen(name)+5);
      strcpy(executable, name);
      strcat(executable, ".cgi");
    }
    */
    if (t)
      for (p = t->list; p; p = p->next)
      { if (p->info.typ->type == Tfun && has_ns_eq(ns->name, p->sym->name))
        { if (sp && sp->name)
	    sprintf(buf, "%s%s.wsdl", dirpath, name);
	  else
	    sprintf(buf, "%s%s.wsdl", dirpath, ns_convert(ns->name));
	  fprintf(stderr, "Saving %s Web Service description\n", buf);
          fd = fopen(buf, "w");
	  if (!fd)
	    execerror("Cannot write WSDL file");
          gen_wsdl(fd, t, ns->name, name, URL, executable, URI, encoding);
          fclose(fd);
          if (sp && sp->name)
	  { sprintf(buf, "%s%s%sProxy.h", dirpath, prefix, name);
	    fprintf(stderr, "Saving %s client proxy class\n", buf);
            fd = fopen(buf, "w");
	    if (!fd)
	      execerror("Cannot write proxy header file");
	    sprintf(buf, "%s%sProxy.h", prefix, name);
	    copyrightnote(fd, buf);
            gen_proxy(fd, t, ns->name, name, URL, executable, URI, encoding);
            fclose(fd);
	  }
	  strcpy(buf, dirpath);
          if (sp && sp->name)
	    strcat(buf, name);
	  else
	    strcat(buf, ns_convert(ns->name));
	  strcat(buf, ".");
          gen_data(buf, t, ns->name, name, URL, executable, URI, encoding);
          if (sp && sp->name)
	    sprintf(buf, "%s%s.nsmap", dirpath, name);
	  else
	    sprintf(buf, "%s%s.nsmap", dirpath, ns_convert(ns->name));
	  fprintf(stderr, "Saving %s namespace mapping table\n", buf);
          fd = fopen(buf, "w");
	  if (!fd)
	    execerror("Cannot write nsmap file");
          fprintf(fd, "\n#include \"%sH.h\"\nSOAP_NMAC struct Namespace namespaces[] =\n{\n", prefix);
          for (ns1 = nslist; ns1; ns1 = ns1->next)
            if (ns1 != ns)
	    { for (sq = services; sq; sq = sq->next)
		if (!strcmp(sq->ns, ns1->name) && sq->URI)
		  break;
	      if (sq)
		fprintf(fd, "  {\"%s\", \"%s\"},\n", ns_convert(ns1->name), sq->URI);
	      else if (!strcmp(ns1->name, "SOAP-ENV"))
		fprintf(fd, "  {\"SOAP-ENV\", \"%s\", \"http://www.w3.org/*/soap-envelope\"},\n", envURI);
	      else if (!strcmp(ns1->name, "SOAP-ENC"))
		fprintf(fd, "  {\"SOAP-ENC\", \"%s\", \"http://www.w3.org/*/soap-encoding\"},\n", encURI);
	      else if (!strcmp(ns1->name, "xsi"))
		fprintf(fd, "  {\"xsi\", \"%s\", \"http://www.w3.org/*/XMLSchema-instance\"},\n", xsiURI);
	      else if (!strcmp(ns1->name, "xsd"))
		fprintf(fd, "  {\"xsd\", \"%s\", \"http://www.w3.org/*/XMLSchema\"},\n", xsdURI);
              else
		fprintf(fd, "  {\"%s\", \"%s/%s.xsd\"},\n", ns_convert(ns1->name), URI, ns_convert(ns1->name)); 
	    }
          fprintf(fd, "  {\"%s\", \"%s\"},\n", ns_convert(ns->name), URI);
          fprintf(fd, "  {NULL, NULL}\n};\n");
          fclose(fd);
	  break;
        }
      }
    sprintf(buf, "%s%s.xsd", dirpath, ns_convert(ns->name));
    fprintf(stderr, "Saving %s XML schema description\n", buf);
    fd = fopen(buf, "w");
    if (!fd)
      execerror("Cannot write schema file");
    fprintf(fd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!--\n");
    copyrightnote(fd, ns->name);
    fprintf(fd, "-->\n");
    gen_schema(fd, t, ns->name, ns->name, 1, 0, URL, URI);
    fclose(fd);
  }
}

char *
nsofhdr(char *part)
{ Entry *p;
  Service *sp;
  char *s;
  p = entry(classtable, lookup("SOAP_ENV__Header"));
  for (p = ((Table*)p->info.typ->ref)->list; p; p = p->next)
  { s = strstr(p->sym->name, "__");
    if (!strcmp(part, p->sym->name) || s && strcmp(part, s+2))
    { for (sp = services; sp; sp = sp->next)
        if (sp->URI && s && !strncmp(sp->ns, p->sym->name, s-p->sym->name))
	  return sp->URI;
      sprintf(errbuf, "Cannot save header part reference in WSDL: SOAP_ENV__Header \"%s\" field has no namespace", p->sym->name);
      semwarn(errbuf);
      return "";
    }
  }
  sprintf(errbuf, "Cannot save header part reference in WSDL: SOAP_ENV__Header has no \"%s\" field", part);
  semwarn(errbuf);
  return "";
}      

void
gen_wsdl(FILE *fd, Table *t, char *ns, char *name, char *URL, char *executable, char *URI, char *encoding)
{ Entry *p, *q;
  Symbol *s;
  Service *sp, *sp2;
  Method *m;
  char *action, *comment;
  fprintf(fd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(fd, "<definitions name=\"%s\"\n xmlns=\"http://schemas.xmlsoap.org/wsdl/\"\n xmlns:SOAP=\"http://schemas.xmlsoap.org/wsdl/soap/\"\n xmlns:WSDL=\"http://schemas.xmlsoap.org/wsdl/\"\n", name);
  for (sp = services; sp; sp = sp->next)
    if (!strcmp(sp->ns, ns))
      break;
  if (sp && sp->WSDL)
    fprintf(fd, " targetNamespace=\"%s\"\n xmlns:tns=\"%s\"", sp->WSDL, sp->WSDL);
  else
    fprintf(fd, " targetNamespace=\"%s/%s.wsdl\"\n xmlns:tns=\"%s/%s.wsdl\"", URL, name, URL, name);
  for (s = nslist; s; s = s->next)
  { for (sp2 = services; sp2; sp2 = sp2->next)
      if (!strcmp(sp2->ns, s->name) && sp2->URI)
        break;
    if (sp2)
      fprintf(fd, "\n xmlns:%s=\"%s\"", ns_convert(s->name), sp2->URI);
    else if (!strcmp(s->name, "SOAP-ENV"))
      fprintf(fd, "\n xmlns:SOAP-ENV=\"%s\"", envURI);
    else if (!strcmp(s->name, "SOAP-ENC"))
      fprintf(fd, "\n xmlns:SOAP-ENC=\"%s\"", encURI);
    else if (!strcmp(s->name, "xsi"))
      fprintf(fd, "\n xmlns:xsi=\"%s\"", xsiURI);
    else if (!strcmp(s->name, "xsd"))
      fprintf(fd, "\n xmlns:xsd=\"%s\"", xsdURI);
    else if (!strcmp(s->name, ns))
      fprintf(fd, "\n xmlns:%s=\"%s\"", ns_convert(s->name), URI);
    else
      fprintf(fd, "\n xmlns:%s=\"%s/%s.xsd\"", ns_convert(s->name), URI, ns_convert(s->name));
  }
  fprintf(fd, ">\n\n");
  fprintf(fd, "<types>\n");
  for (s = nslist; s; s = s->next)
    gen_schema(fd, t, ns, s->name, !strcmp(s->name, ns), 1, URL, URI);
  fprintf(fd, "</types>\n\n");
  fflush(fd);
  if (t)
  { for (p = t->list; p; p = p->next)
      if (p->info.typ->type == Tfun && has_ns_eq(ns, p->sym->name))
      { fprintf(fd, "<message name=\"%sRequest\">\n", ns_remove(p->sym->name));
        fflush(fd);
  	q = entry(classtable, p->sym);
  	if (q)
	  for (q = ((Table*)q->info.typ->ref)->list; q; q = q->next)
	    if (is_XML(q->info.typ))
	      fprintf(fd, " <part name=\"body\" element=\"%s\"/>\n", ns_convert(p->sym->name));
	    else if (!is_transient(q->info.typ) && !(q->info.sto & Sattribute) && q->info.typ->type != Tfun)
	      fprintf(fd, " <part name=\"%s\" type=\"%s\"/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns));
        fprintf(fd, "</message>\n\n");
        fflush(fd);
	q = (Entry*)p->info.typ->ref;
	if (q && is_transient(q->info.typ))
	  ;
	else if (q && !is_response(q->info.typ))
	  if (is_XML(q->info.typ->ref))
	    fprintf(fd, "<message name=\"%sResponse\">\n <part name=\"body\" element=\"%sResponse\"/>\n</message>\n\n", ns_remove(p->sym->name), ns_convert(p->sym->name));
	  else
	    fprintf(fd, "<message name=\"%sResponse\">\n <part name=\"%s\" type=\"%s\"/>\n</message>\n\n", ns_remove(p->sym->name), ns_convert(q->sym->name), wsdl_type(q->info.typ, ns));
        else if (q && q->info.typ->wsdl == False)
	{ q->info.typ->wsdl = True;
	  fprintf(fd, "<message name=\"%s\">\n", ns_remove(((Tnode*)q->info.typ->ref)->id->name));
	  if (((Tnode*)q->info.typ->ref)->ref)
	  { for (q = ((Table*)((Tnode*)q->info.typ->ref)->ref)->list; q; q = q->next)
	      if (is_XML(q->info.typ))
	        fprintf(fd, " <part name=\"body\" element=\"%s\"/>\n", ns_convert(((Tnode*)((Entry*)p->info.typ->ref)->info.typ->ref)->id->name));
	      else if (!is_transient(q->info.typ) && !(q->info.sto & Sattribute) && q->info.typ->type != Tfun)
	        fprintf(fd, " <part name=\"%s\" type=\"%s\"/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns));
	  }
          fprintf(fd, "</message>\n\n");
	}
        fflush(fd);
      }
    if (custom_header)
    { fprintf(fd, "<message name=\"%sHeader\">\n", name);
      for (q = ((Table*)entry(classtable, lookup("SOAP_ENV__Header"))->info.typ->ref)->list; q; q = q->next)
	if (!is_transient(q->info.typ) && !(q->info.sto & Sattribute) && q->info.typ->type != Tfun)
	  fprintf(fd, " <part name=\"%s\" type=\"%s\"/>\n", ns_remove(q->sym->name), wsdl_type(q->info.typ, ns));
      fprintf(fd, "</message>\n\n");
    }
    if (custom_fault)
    { fprintf(fd, "<message name=\"%sFault\">\n", name);
      for (q = ((Table*)entry(classtable, lookup("SOAP_ENV__Fault"))->info.typ->ref)->list; q; q = q->next)
	if (!is_transient(q->info.typ) && !(q->info.sto & Sattribute) && q->info.typ->type != Tfun)
	  fprintf(fd, " <part name=\"%s\" type=\"%s\"/>\n", ns_remove(q->sym->name), wsdl_type(q->info.typ, ns));
      fprintf(fd, "</message>\n\n");
    }
    fflush(fd);
    if (sp && sp->port)
      fprintf(fd, "<portType name=\"%s\">\n", sp->port);
    else
      fprintf(fd, "<portType name=\"%sPortType\">\n", name);
    for (p = t->list; p; p = p->next)
      if (p->info.typ->type == Tfun && has_ns_eq(ns, p->sym->name))
      { fprintf(fd, " <operation name=\"%s\">\n", ns_remove(p->sym->name));
	{ char *s;
          comment = NULL;
	  if (sp && (s = strstr(p->sym->name, "__")))
	    for (m = sp->list; m; m = m->next)
	      if (m->mess == COMMENT && m->part && !strcmp(m->name, s+2))
	      { comment = m->part;
	        break;
	      }
	}
        if (comment)
          fprintf(fd, "  <documentation>%s</documentation>\n", comment);
        else
          fprintf(fd, "  <documentation>Service definition of function %s</documentation>\n", p->sym->name);
        fprintf(fd, "  <input message=\"tns:%sRequest\"/>\n", ns_remove(p->sym->name));
	q = (Entry*)p->info.typ->ref;
	if (q && is_transient(q->info.typ))
	  ;
	else if (q && !is_response(q->info.typ))
	  fprintf(fd, "  <output message=\"tns:%sResponse\"/>\n", ns_remove(p->sym->name));
        else if (q)
	  fprintf(fd, "  <output message=\"tns:%s\"/>\n", ns_remove(((Tnode*)q->info.typ->ref)->id->name));
	if (custom_fault)
	  fprintf(fd, "  <fault name=\"%sFault\" message=\"tns:%sFault\"/>\n", name, name);
        fprintf(fd, " </operation>\n");
      }
    fprintf(fd, "</portType>\n\n");
    if (encoding && !strcmp(encoding, "literal"))
      if (sp && sp->port)
        fprintf(fd, "<binding name=\"%sBinding\" type=\"tns:%s\">\n <SOAP:binding style=\"document\" transport=\"http://schemas.xmlsoap.org/soap/http\"/>\n", name, sp->port);
      else
        fprintf(fd, "<binding name=\"%sBinding\" type=\"tns:%sPortType\">\n <SOAP:binding style=\"document\" transport=\"http://schemas.xmlsoap.org/soap/http\"/>\n", name, name);
    else
      if (sp && sp->port)
        fprintf(fd, "<binding name=\"%sBinding\" type=\"tns:%s\">\n <SOAP:binding style=\"rpc\" transport=\"http://schemas.xmlsoap.org/soap/http\"/>\n", name, sp->port);
      else
        fprintf(fd, "<binding name=\"%sBinding\" type=\"tns:%sPortType\">\n <SOAP:binding style=\"rpc\" transport=\"http://schemas.xmlsoap.org/soap/http\"/>\n", name, name);
    fflush(fd);
    for (p = t->list; p; p = p->next)
      if (p->info.typ->type == Tfun && has_ns_eq(ns, p->sym->name))
      { fprintf(fd, " <operation name=\"%s\">\n", ns_remove(p->sym->name));
	{ char *s;
	  action = "";
	  if (sp && (s = strstr(p->sym->name, "__")))
	    for (m = sp->list; m; m = m->next)
	      if (m->mess == ACTION && m->part && !strcmp(m->name, s+2))
	      { action = m->part;
	        break;
	      }
	}
        if (encoding && !strcmp(encoding, "literal"))
          if (*action == '"')
	    fprintf(fd, "  <SOAP:operation soapAction=%s style=\"document\"/>\n", action);
          else
	    fprintf(fd, "  <SOAP:operation soapAction=\"%s\" style=\"document\"/>\n", action);
	else if (*action == '"')
          fprintf(fd, "  <SOAP:operation soapAction=%s/>\n", action);
        else
          fprintf(fd, "  <SOAP:operation soapAction=\"%s\"/>\n", action);
  	q = entry(classtable, p->sym);
  	if (encoding && !strcmp(encoding, "literal") || q && (q = (((Table*)q->info.typ->ref)->list)) && q && is_XML(q->info.typ))
	  fprintf(fd, "  <input>\n   <SOAP:body use=\"literal\"/>\n");
        else if (encoding)
	  fprintf(fd, "  <input>\n   <SOAP:body use=\"encoded\" namespace=\"%s\" encodingStyle=\"%s\"/>\n", URI, encoding);
        else
	  fprintf(fd, "  <input>\n   <SOAP:body use=\"encoded\" namespace=\"%s\" encodingStyle=\"%s\"/>\n", URI, encURI);
	if (custom_header)
	{ int f = 0;
	  char *s;
	  m = NULL;
	  if (sp && (s = strstr(p->sym->name, "__")))
	    for (m = sp->list; m; m = m->next)
	      if (!strcmp(m->name, s+2) && (m->mess&HDRIN))
	      { f = 1;
	        fprintf(fd, "   <SOAP:header use=\"encoded\" namespace=\"%s\" encodingStyle=\"%s\" message=\"tns:%sHeader\" part=\"%s\"/>\n", nsofhdr(m->part), encURI, name, ns_remove(m->part));
	      }
	  if (!f && !sp)
	    fprintf(fd, "   <SOAP:header use=\"encoded\" encodingStyle=\"%s\" message=\"tns:%sHeader\"/>\n", encURI, name);
	}
	fprintf(fd, "  </input>\n");
	q = (Entry*)p->info.typ->ref;
	if (!q || !q->info.typ->ref)
	{ fprintf(fd, " </operation>\n");
	  continue;
	}
	if (encoding && !strcmp(encoding, "literal") || is_XML(q->info.typ->ref))
	  fprintf(fd, "  <output>\n   <SOAP:body use=\"literal\"/>\n");
	else if (encoding)
	  fprintf(fd, "  <output>\n   <SOAP:body use=\"encoded\" namespace=\"%s\" encodingStyle=\"%s\"/>\n", URI, encoding);
	else
	  fprintf(fd, "  <output>\n   <SOAP:body use=\"encoded\" namespace=\"%s\" encodingStyle=\"%s\"/>\n", URI, encURI);
	if (custom_header)
	{ Method *m = NULL;
	  int f = 0;
	  char *s;
	  if (sp && (s = strstr(p->sym->name, "__")))
	    for (m = sp->list; m; m = m->next)
	      if (!strcmp(m->name, s+2) && (m->mess&HDROUT))
	      { f = 1;
	        fprintf(fd, "   <SOAP:header use=\"encoded\" namespace=\"%s\" encodingStyle=\"%s\" message=\"tns:%sHeader\" part=\"%s\"/>\n", nsofhdr(m->part), encURI, name, ns_remove(m->part));
	      }
	  if (!f & !sp)
	    fprintf(fd, "   <SOAP:header use=\"encoded\" encodingStyle=\"%s\" message=\"tns:%sHeader\"/>\n", encURI, name);
	}
	fprintf(fd, "  </output>\n");
	if (custom_fault)
	  fprintf(fd, "   <fault>\n   <SOAP:fault name=\"%sFault\" use=\"encoded\" encodingStyle=\"%s\" message=\"tns:%sFault\"/>\n   </fault>\n", name, encURI, name);
	fprintf(fd, " </operation>\n");
        fflush(fd);
      }
    fprintf(fd, "</binding>\n\n");
  }
  fprintf(fd, "<service name=\"%s\">\n", name);
  if (sp && sp->documentation)
    fprintf(fd, " <documentation>%s</documentation>\n", sp->documentation);
  else
    fprintf(fd, " <documentation>gSOAP "VERSION" generated service definition</documentation>\n");
  if (executable)
    fprintf(fd, " <port name=\"%s\" binding=\"tns:%sBinding\">\n  <SOAP:address location=\"%s/%s\"/>\n </port>\n</service>\n\n</definitions>\n", name, name, URL, executable);
  else
    fprintf(fd, " <port name=\"%s\" binding=\"tns:%sBinding\">\n  <SOAP:address location=\"%s\"/>\n </port>\n</service>\n\n</definitions>\n", name, name, URL);
}

char *
default_value(Entry *e)
{ Entry *q;
  static char buf[1024];
  buf[0] = '\0';
  if (e->info.hasval)
    switch (e->info.typ->type)
    { case Tchar:
      case Twchar:
      case Tuchar:
      case Tshort:
      case Tushort:
      case Tint:
      case Tuint:
      case Tlong:
      case Tllong:
      case Tulong:
      case Tullong:
        sprintf(buf, " default=\""SOAP_LONG_FORMAT"\"", e->info.val.i);
	break;
      case Tfloat:
      case Tdouble:
        sprintf(buf, " default=\"%f\"", e->info.val.r);
	break;
      case Ttime:
        break; /* should get value? */
      case Tenum:
	for (q = ((Table*)e->info.typ->ref)->list; q; q = q->next)
	  if (q->info.val.i == e->info.val.i)
	  { sprintf(buf, " default=\"%s\"", ns_convert(q->sym->name));
	    break;
	  }
        break;
      default:
	if (e->info.val.s && strlen(e->info.val.s) < sizeof(buf)-12)
          sprintf(buf, " default=\"%s\"", e->info.val.s);
	break;
    }
  return buf;
}

void
gen_schema(FILE *fd, Table *t, char *ns1, char *ns, int all, int wsdl, char *URL, char *URI)
{ int i, d;
  char cbuf[4];
  Entry *p, *q;
  Tnode *n;
  Symbol *s;
  Service *sp;
  if (!strcmp(ns, "SOAP-ENV") || !strcmp(ns, "SOAP-ENC") || !strcmp(ns, "xsi") || !strcmp(ns, "xsd"))
    return;
  fprintf(fd, " <schema ");
  for (sp = services; sp; sp = sp->next)
    if (!strcmp(sp->ns, ns) && sp->URI)
      break;
  if (sp)
    fprintf(fd, "targetNamespace=\"%s\"", sp->URI);
  else if (all)
    fprintf(fd, "targetNamespace=\"%s\"", URI);
  else
    fprintf(fd, "targetNamespace=\"%s/%s.xsd\"", URI, ns_convert(ns));
  for (s = nslist; s; s = s->next)
  { for (sp = services; sp; sp = sp->next)
      if (!strcmp(sp->ns, s->name) && sp->URI)
        break;
    if (sp)
      fprintf(fd, "\n  xmlns:%s=\"%s\"", ns_convert(s->name), sp->URI);
    else if (!strcmp(s->name, "SOAP-ENV"))
      fprintf(fd, "\n  xmlns:SOAP-ENV=\"%s\"", envURI);
    else if (!strcmp(s->name, "SOAP-ENC"))
      fprintf(fd, "\n  xmlns:SOAP-ENC=\"%s\"", encURI);
    else if (!strcmp(s->name, "xsi"))
      fprintf(fd, "\n  xmlns:xsi=\"%s\"", xsiURI);
    else if (!strcmp(s->name, "xsd"))
      fprintf(fd, "\n  xmlns:xsd=\"%s\"", xsdURI);
    else if (!strcmp(s->name, ns1))
      fprintf(fd, "\n  xmlns:%s=\"%s\"", ns_convert(s->name), URI);
    else
      fprintf(fd, "\n  xmlns:%s=\"%s/%s.xsd\"", ns_convert(s->name), URI, ns_convert(s->name));
  }
  fprintf(fd, "\n  xmlns=\"%s\"\n  elementFormDefault=\"unqualified\"\n  attributeFormDefault=\"unqualified\">\n\n", xsdURI);
  fflush(fd);
  if (typetable)
    for (p = typetable->list; p; p = p->next)
    { if (!is_transient(p->info.typ) && !is_external(p->info.typ) && ((has_ns_eq(ns, p->sym->name))))
      { if (is_primitive(p->info.typ) || is_string(p->info.typ) || is_wstring(p->info.typ))
        { fprintf(fd, "  <simpleType name=\"%s\">\n   <restriction base=\"%s\">\n", ns_remove(p->sym->name), base_type(p->info.typ, ns));
          fprintf(fd, "   </restriction>\n  </simpleType>\n\n");
        }
        else
	{ fprintf(fd, "  <complexType name=\"%s\">\n   <restriction base=\"%s\">\n", ns_remove(p->sym->name), base_type(p->info.typ, ns));
          fprintf(fd, "   </restriction>\n  </simpleType>\n\n");
        }
      }
    }
  fflush(fd);
  if (enumtable)
    for (p = enumtable->list; p; p = p->next)
    { if (!is_transient(p->info.typ) && (!has_ns(p->info.typ) && all || has_ns_eq(ns, p->sym->name)))
      { if (is_mask(p->info.typ))
        { fprintf(fd, "  <simpleType name=\"%s\">\n   <list>\n    <restriction base=\"xsd:string\">\n", wsdl_type(p->info.typ, NULL));
          if ((Table*)p->info.typ->ref)
            for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
              fprintf(fd, "     <enumeration value=\"%s\"/>\n", ns_convert(q->sym->name));
          fprintf(fd, "    </restriction>\n   </list>\n  </simpleType>\n\n");
	}
	else
        { fprintf(fd, "  <simpleType name=\"%s\">\n   <restriction base=\"xsd:string\">\n", wsdl_type(p->info.typ, NULL));
          if ((Table*)p->info.typ->ref)
            for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
              fprintf(fd, "    <enumeration value=\"%s\"/>\n", ns_convert(q->sym->name));
          fprintf(fd, "   </restriction>\n  </simpleType>\n\n");
        }
      }
    }
  fflush(fd);
  if (classtable)
    for (p = classtable->list; p; p = p->next)
    { for (q = t->list; q; q = q->next)
        if (q->info.typ->type == Tfun && p == get_response(q->info.typ))
	  /* || is_response(((Entry*)q->info.typ->ref)->info.typ) && p->info.typ == ((Entry*)q->info.typ->ref)->info.typ->ref))
	   */
	  break;
      if (q)
        continue;
      if (p->info.typ->ref && is_binary(p->info.typ))
      { if (!has_ns_eq("xsd", p->info.typ->id->name) && !has_ns_eq("SOAP-ENC", p->info.typ->id->name))
	{ fprintf(fd, "  <simpleType name=\"%s\">\n   <restriction base=\"xsd:base64Binary\">\n", wsdl_type(p->info.typ, NULL));
          for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
	  { if (q->info.sto & Sattribute)
              fprintf(fd, "    <attribute name=\"%s\" type=\"%s\"%s/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns1), default_value(q));
            fflush(fd);
	  }
          fprintf(fd, "   </restriction>\n  </simpleType>\n\n");
        }
      }
      else if (p->info.typ->ref && !is_transient(p->info.typ) && is_primclass(p->info.typ))
      { if ((!has_ns(p->info.typ) && all || has_ns_eq(ns, p->sym->name)))
        { q = ((Table*)p->info.typ->ref)->list;
          fprintf(fd, "  <simpleType name=\"%s\">\n   <restriction base=\"%s\">\n", ns_remove(p->sym->name), wsdl_type(q->info.typ, ns1));
          for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
	  { if (q->info.sto & Sattribute)
              fprintf(fd, "    <attribute name=\"%s\" type=\"%s\"%s/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns1), default_value(q));
            fflush(fd);
	  }
          fprintf(fd, "   </restriction>\n  </simpleType>\n\n");
        }
      }
      else if (p->info.typ->ref && !is_transient(p->info.typ))
      { q = ((Table*)p->info.typ->ref)->list;
        if (entry(t, p->sym) && (!q || !is_XML(q->info.typ)))
          ;
        else if (is_dynamic_array(p->info.typ))
          if (!has_ns(p->info.typ) && !is_untyped(p->info.typ))
          { if (all)
	      if (wsdl)
	      { d = get_Darraydims(p->info.typ)-1;
	        for (i = 0; i < d; i++)
	          cbuf[i] = ',';
	        cbuf[i] = '\0';
		if (q->info.maxOccurs == 1)
	          fprintf(fd, "  <complexType name=\"%s\">\n   <complexContent>\n    <restriction base=\"SOAP-ENC:Array\">\n     <sequence>\n      <element name=\"%s\" type=\"%s\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>\n     </sequence>\n     <attribute ref=\"SOAP-ENC:arrayType\" WSDL:arrayType=\"%s[%s]\"/>\n    </restriction>\n   </complexContent>\n  </complexType>\n\n", wsdl_type(p->info.typ, NULL), q->sym->name[5]?ns_convert(q->sym->name+5):"item", wsdl_type(q->info.typ, ns1), wsdl_type(q->info.typ, ns1), cbuf);
                else
	          fprintf(fd, "  <complexType name=\"%s\">\n   <complexContent>\n    <restriction base=\"SOAP-ENC:Array\">\n     <sequence>\n      <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\"/>\n     </sequence>\n     <attribute ref=\"SOAP-ENC:arrayType\" WSDL:arrayType=\"%s[%s]\"/>\n    </restriction>\n   </complexContent>\n  </complexType>\n\n", wsdl_type(p->info.typ, NULL), q->sym->name[5]?ns_convert(q->sym->name+5):"item", wsdl_type(q->info.typ, ns1), q->info.minOccurs, q->info.maxOccurs, wsdl_type(q->info.typ, ns1), cbuf);
	      }
              else
		if (q->info.maxOccurs == 1)
	          fprintf(fd, "  <complexType name=\"%s\">\n   <complexContent>\n    <restriction base=\"SOAP-ENC:Array\">\n     <sequence>\n      <element name=\"%s\" type=\"%s\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>\n     </sequence>\n    </restriction>\n   </complexContent>\n  </complexType>\n\n", wsdl_type(p->info.typ, NULL), q->sym->name[5]?ns_convert(q->sym->name+5):"item", wsdl_type(q->info.typ, ns1));
                else
	          fprintf(fd, "  <complexType name=\"%s\">\n   <complexContent>\n    <restriction base=\"SOAP-ENC:Array\">\n     <sequence>\n      <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\"/>\n     </sequence>\n    </restriction>\n   </complexContent>\n  </complexType>\n\n", wsdl_type(p->info.typ, NULL), q->sym->name[5]?ns_convert(q->sym->name+5):"item", wsdl_type(q->info.typ, ns1),  q->info.minOccurs, q->info.maxOccurs);
          }
	  else
	    if (q->info.maxOccurs == 1)
	      fprintf(fd, "  <complexType name=\"%s\">\n   <sequence>\n    <element name=\"%s\" type=\"%s\" minOccurs=\"0\" maxOccurs=\"unbounded\" nillable=\"true\"/>\n   </sequence>\n  </complexType>\n\n", ns_remove(p->sym->name), q->sym->name[5]?ns_convert(q->sym->name+5):"item", wsdl_type(q->info.typ, ns1));
            else
	      fprintf(fd, "  <complexType name=\"%s\">\n   <sequence>\n    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\" nillable=\"true\"/>\n   </sequence>\n  </complexType>\n\n", ns_remove(p->sym->name), q->sym->name[5]?ns_convert(q->sym->name+5):"item", wsdl_type(q->info.typ, ns1), q->info.minOccurs, q->info.maxOccurs);
        else if (p->info.typ->type == Tstruct && (!has_ns(p->info.typ) && all || has_ns_eq(ns, p->sym->name)))
        { if (p->info.typ->ref)
          { fprintf(fd, "  <complexType name=\"%s\">\n   <all>\n", ns_remove(p->sym->name));
            for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
	    { if (is_transient(q->info.typ) || (q->info.sto & Sattribute) || q->info.typ->type == Tfun)
	        continue;
	      else if (is_repetition(q))
              { if (((Tnode*)q->next->info.typ->ref)->type == Tpointer)
	          if (q->info.maxOccurs == 1)
		    fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"unbounded\" nillable=\"true\"/>\n", ns_convert(q->next->sym->name), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs);
                  else
		    fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\" nillable=\"true\"/>\n", ns_convert(q->next->sym->name), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs, q->info.maxOccurs);
                else
	          if (q->info.maxOccurs == 1)
	            fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"unbounded\"/>\n", ns_convert(q->next->sym->name), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs);
		  else
	            fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\"/>\n", ns_convert(q->next->sym->name), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs, q->info.maxOccurs);
	        q = q->next;
	      }
              else if (q->info.typ->type == Tpointer || q->info.typ->type == Tarray || is_dynamic_array(q->info.typ))
                fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\" nillable=\"true\"%s/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns1), q->info.minOccurs, q->info.maxOccurs, default_value(q));
	      else
                fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\"%s/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns1), q->info.minOccurs, q->info.maxOccurs, default_value(q));
              fflush(fd);
	    }
            fprintf(fd, "   </all>\n");
            for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
	    { if (q->info.sto & Sattribute)
                fprintf(fd, "   <attribute name=\"%s\" type=\"%s\"%s/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns1), default_value(q));
              fflush(fd);
	    }
            fprintf(fd, "  </complexType>\n\n");
          }
        }
        else if (p->info.typ->type == Tclass && (!has_ns(p->info.typ) && all || has_ns_eq(ns, p->sym->name)))
        { if (p->info.typ->ref)
            if (((Table*)p->info.typ->ref)->prev && !is_transient(entry(classtable, ((Table*)p->info.typ->ref)->prev->sym)->info.typ))
            { fprintf(fd, "  <complexType name=\"%s\">\n   <complexContent>\n    <extension base=\"%s\">\n     <sequence>\n", ns_remove(p->sym->name), ns_convert(((Table*)p->info.typ->ref)->prev->sym->name));
              for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
	      { if (is_transient(q->info.typ) || (q->info.sto & Sattribute) || q->info.typ->type == Tfun)
	          continue;
	        else if (is_repetition(q))
                { if (((Tnode*)q->next->info.typ->ref)->type == Tpointer)
	            if (q->info.maxOccurs == 1)
                      fprintf(fd, "     <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"unbounded\" nillable=\"true\"/>\n", ns_overridden(p->info.typ->ref, q->next), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs);
		    else
                      fprintf(fd, "     <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\" nillable=\"true\"/>\n", ns_overridden(p->info.typ->ref, q->next), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs, q->info.maxOccurs);
		  else
	            if (q->info.maxOccurs == 1)
                      fprintf(fd, "     <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"unbounded\"/>\n", ns_overridden(p->info.typ->ref, q->next), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs);
		    else
                      fprintf(fd, "     <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\"/>\n", ns_overridden(p->info.typ->ref, q->next), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs, q->info.maxOccurs);
	          q = q->next;
	        }
                else if (q->info.typ->type == Tpointer || q->info.typ->type == Tarray || is_dynamic_array(q->info.typ))
                  fprintf(fd, "      <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\" nillable=\"true\"%s/>\n", ns_overridden(p->info.typ->ref, q), wsdl_type(q->info.typ, ns1), q->info.minOccurs, q->info.maxOccurs, default_value(q));
	        else
	          fprintf(fd, "      <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\"%s/>\n", ns_overridden(p->info.typ->ref, q), wsdl_type(q->info.typ, ns1), q->info.minOccurs, q->info.maxOccurs, default_value(q));
                fflush(fd);
	      }
              fprintf(fd, "     </sequence>\n    </extension>\n   </complexContent>\n  </complexType>\n\n");
	    }
	    else
            { fprintf(fd, "  <complexType name=\"%s\">\n   <sequence>\n", ns_remove(p->sym->name));
              for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
	        if (is_transient(q->info.typ) || (q->info.sto & Sattribute) || q->info.typ->type == Tfun)
	          continue;
	        else if (is_repetition(q))
                { if (((Tnode*)q->next->info.typ->ref)->type == Tpointer)
                    fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"\" maxOccurs=\"unbounded\" nillable=\"true\"/>\n", ns_convert(q->next->sym->name), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs);
		  else
                    fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"unbounded\"/>\n", ns_convert(q->next->sym->name), wsdl_type(q->next->info.typ->ref, ns1), q->info.minOccurs);
	          q = q->next;
	        }
                else if (q->info.typ->type == Tpointer || q->info.typ->type == Tarray || is_dynamic_array(q->info.typ))
                  fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\" nillable=\"true\"%s/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns1), q->info.minOccurs, q->info.maxOccurs, default_value(q));
	        else
                  fprintf(fd, "    <element name=\"%s\" type=\"%s\" minOccurs=\"%d\" maxOccurs=\"%d\"%s/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns1), q->info.minOccurs, q->info.maxOccurs, default_value(q));
              fprintf(fd, "   </sequence>\n");
              for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
	      { if (q->info.sto & Sattribute)
                  fprintf(fd, "   <attribute name=\"%s\" type=\"%s\"%s/>\n", ns_convert(q->sym->name), wsdl_type(q->info.typ, ns1), default_value(q));
                fflush(fd);
	      }
              fprintf(fd, "  </complexType>\n\n");
            }
        }
      }
    }
  fflush(fd);
  for (n = Tptr[Tarray]; n; n = n->next)
  { if (is_transient(n))
      continue;
    else if (wsdl)
      fprintf(fd, "  <complexType name=\"%s\">\n   <complexContent>\n    <restriction base=\"SOAP-ENC:Array\">\n     <attribute ref=\"SOAP-ENC:arrayType\" WSDL:arrayType=\"%s[]\"/>\n    </restriction>\n   </complexContent>\n  </complexType>\n\n", c_ident(n), wsdl_type(n->ref, ns1));
    else
      fprintf(fd, "  <complexType name=\"%s\">\n   <complexContent>\n    <restriction base=\"SOAP-ENC:Array\">\n     <element name=\"item\" type=\"%s\" maxOccurs=\"unbounded\"/>\n    </restriction>\n   </complexContent>\n  </complexType>\n\n", c_ident(n), xsi_type(n->ref));
    fflush(fd);
  }
  fprintf(fd, " </schema>\n");
}

void
gen_proxy(FILE *fd, Table *table, char *ns, char *name, char *URL, char *executable, char *URI, char *encoding)
{ Entry *p, *q, *r;
  Table *t, *output;
  char *s;
  Service *sp;
  Method *m;
  int flag;
  char *name1;
  name1 = emalloc(strlen(name)+1);
  strcpy(name1, name);
  for (s = name1+1; *s; s++)
  { if (!isalnum(*s))
    { *s = '\0';
      break;
    }
  }
  for (sp = services; sp; sp = sp->next)
    if (!strcmp(sp->ns, ns))
      break;
  fprintf(fd, "\n\n#ifndef %s_H\n#define %s_H\n#include \"%sH.h\"\nclass %s\n{   public:\n\tstruct soap *soap;\n\tconst char *endpoint;\n", name1, name1, prefix, name1);
  fprintf(fd, "\t%s() { soap = soap_new(); endpoint = \"%s\"; };\n", name1, URL);
  fprintf(fd, "\t~%s() { if (soap) { soap_destroy(soap); soap_end(soap); soap_done(soap); free((void*)soap); } };\n", name1);
  fflush(fd);
  for (r = table->list; r; r = r->next)
    if (r->info.typ->type == Tfun && has_ns_eq(ns, r->sym->name))
    { p = entry(table, r->sym);
      if (p)
        q = (Entry*)p->info.typ->ref;
      else
        fprintf(stderr, "Internal error: no table entry\n");
      p = entry(classtable, r->sym);
      if (!p)
        fprintf(stderr, "Internal error: no parameter table entry\n");
      output = (Table*)p->info.typ->ref;
      fprintf(fd, "\tint %s(", ns_remove(r->sym->name));
      flag = 0;
      for (t = output; t; t = t->prev)
      { p = t->list;
        if (p)
        { fprintf(fd, "%s%s%s", c_storage(p->info.sto), c_type_id(p->info.typ, p->sym->name), c_init(p));
          for (p = p->next; p; p = p->next)
            fprintf(fd, ", %s%s%s", c_storage(p->info.sto), c_type_id(p->info.typ, p->sym->name), c_init(p));
	  flag = 1;
        }
      }
      if (is_transient(q->info.typ))
        fprintf(fd,") { return soap ? soap_send_%s(soap, endpoint, NULL", r->sym->name);
      else if (flag)
        fprintf(fd,", %s%s) { return soap ? soap_call_%s(soap, endpoint, NULL", c_storage(q->info.sto), c_type_id(q->info.typ, q->sym->name), r->sym->name);
      else
        fprintf(fd,"%s%s) { return soap ? soap_call_%s(soap, endpoint, NULL", c_storage(q->info.sto), c_type_id(q->info.typ, q->sym->name), r->sym->name);
      /* the action is now handled by the soap_call/soap_send operation when we pass NULL
      m = NULL;
      if (sp && (s = strstr(r->sym->name, "__")))
        for (m = sp->list; m; m = m->next)
          if (m->part && m->mess == ACTION && !strcmp(m->name, s+2))
          { if (*m->part == '"')
	      fprintf(fd, "%s", m->part);
            else
	      fprintf(fd, "\"%s\"", m->part);
	    break;
	  }
      if (!m)
        fprintf(fd, "NULL");
      */
      for (t = output; t; t = t->prev)
        for (p = t->list; p; p = p->next)
          fprintf(fd, ", %s", p->sym->name);
      if (is_transient(q->info.typ))
        fprintf(fd,") : SOAP_EOM; };\n");
      else
        fprintf(fd,", %s) : SOAP_EOM; };\n", q->sym->name);
      fflush(fd);
    }
  fprintf(fd, "};\n#endif\n");
}

void
gen_data(char *buf, Table *t, char *ns, char *name, char *URL, char *executable, char *URI, char *encoding)
{ Entry *p, *q, *r;
  FILE *fd;
  if (t)
  { for (p = t->list; p; p = p->next)
      if (p->info.typ->type == Tfun && has_ns_eq(ns, p->sym->name))
      { fd = gen_env(buf, ns_remove(p->sym->name), 0, t, ns, name, URL, executable, URI, encoding);
	if (!fd)
	  return;
  	q = entry(classtable, p->sym);
        fprintf(fd, "  <%s", ns_convert(p->sym->name));
  	if (q)
	{ gen_atts(fd, q->info.typ->ref);
          fprintf(fd, "\n");
	  for (q = ((Table*)q->info.typ->ref)->list; q; q = q->next)
	    gen_field(fd, 3, q);
	}
        fprintf(fd, "  </%s>\n", ns_convert(p->sym->name));
        fprintf(fd, " </SOAP-ENV:Body>\n</SOAP-ENV:Envelope>\n", ns_convert(p->sym->name));
        fclose(fd);
	q = (Entry*)p->info.typ->ref;
	if (q && !is_transient(q->info.typ))
        { fd = gen_env(buf, ns_remove(p->sym->name), 1, t, ns, name, URL, executable, URI, encoding);
	  if (!fd)
	    return;
	  if (q && !is_response(q->info.typ))
	    if (is_XML(q->info.typ->ref))
	      fprintf(fd, "  <%sResponse>\n  </%sResponse>\n", ns_convert(p->sym->name), ns_convert(p->sym->name));
	    else
	    { fprintf(fd, "  <%sResponse>\n", ns_convert(p->sym->name));
	      gen_field(fd, 3, q);
	      fprintf(fd, "  </%sResponse>\n", ns_convert(p->sym->name));
	    }
          else if (q && q->info.typ->ref && ((Tnode*)q->info.typ->ref)->ref)
	  { fprintf(fd, "  <%s", ns_convert(((Tnode*)q->info.typ->ref)->id->name));
	    gen_atts(fd, ((Tnode*)q->info.typ->ref)->ref);
            fprintf(fd, "\n");
	    for (r = ((Table*)((Tnode*)q->info.typ->ref)->ref)->list; r; r = r->next)
	      gen_field(fd, 3, r);
	    fprintf(fd, "  </%s>\n", ns_convert(((Tnode*)q->info.typ->ref)->id->name));
	  }
          fflush(fd);
          fprintf(fd, " </SOAP-ENV:Body>\n</SOAP-ENV:Envelope>\n", ns_convert(p->sym->name));
          fclose(fd);
	}
      }
  }
}

void
gen_field(FILE *fd, int n, Entry *p)
{ Entry *q;
  char *s, tmp[32];
  int i, d;
  if (!(p->info.sto & Sattribute) && !is_transient(p->info.typ) && p->info.typ->type != Tfun && strncmp(p->sym->name, "__size", 6))
  { if (is_XML(p->info.typ))
      fprintf(fd, "%*s<%s/>\n", n, "", ns_convert(p->sym->name));
    else
    { if (!is_string(p->info.typ) && n >= 10 && (p->info.typ->type == Tpointer || p->info.typ->type == Treference))
        fprintf(fd, "%*s<%s xsi:nil=\"true\">", n, "", ns_convert(p->sym->name));
      else if (p->info.typ->type == Tarray)
      { i = ((Tnode*) p->info.typ->ref)->width;
        if (i)
        { i = p->info.typ->width / i;
          if (i > 100)
            i = 100;
	}
        fprintf(fd, "%*s<%s xsi:type=\"SOAP-ENC:Array\" SOAP-ENC:arrayType=\"%s[%d]\">", n, "", ns_convert(p->sym->name), xsi_type_Tarray(p->info.typ), i);
        fflush(fd);
	gen_val(fd, n, p->info.typ);
      }
      else if (is_dynamic_array(p->info.typ) && !is_binary(p->info.typ))
      { if (has_ns(p->info.typ) || is_untyped(p->info.typ))
          fprintf(fd, "%*s<%s xsi:type=\"%s\">", n, "", ns_convert(p->sym->name), xsi_type(p->info.typ));
        else
	{ d = get_Darraydims(p->info.typ);
	  if (d)
	  { for (i = 0; i < d-1; i++)
	    { tmp[2*i] = ',';
	      tmp[2*i+1] = '1';
	    }
	    tmp[2*d-2] = '\0';
	  }
	  else
	    *tmp = '\0';
          fprintf(fd, "%*s<%s xsi:type=\"SOAP-ENC:Array\" SOAP-ENC:arrayType=\"%s[1%s]\">", n, "", ns_convert(p->sym->name), xsi_type_Darray(p->info.typ), tmp);
	}
        fflush(fd);
        gen_val(fd, n, p->info.typ);
      }
      else if ((p->info.typ->type == Tpointer || p->info.typ->type == Treference)
             && is_dynamic_array(p->info.typ->ref) && !is_binary(p->info.typ->ref))
      { if (has_ns(p->info.typ->ref) || is_untyped(p->info.typ->ref))
          fprintf(fd, "%*s<%s xsi:type=\"%s\">", n, "", ns_convert(p->sym->name), xsi_type(p->info.typ->ref));
        else
	  fprintf(fd, "%*s<%s xsi:type=\"SOAP-ENC:Array\" SOAP-ENC:arrayType=\"%s[1]\">", n, "", ns_convert(p->sym->name), xsi_type_Darray(p->info.typ->ref));
      }
      else if (p->info.typ->type == Tstruct || p->info.typ->type == Tclass)
      { if ((s = xsi_type(p->info.typ)) && *s)
          fprintf(fd, "%*s<%s xsi:type=\"%s\"", n, "", ns_convert(p->sym->name), s);
        else
          fprintf(fd, "%*s<%s", n, "", ns_convert(p->sym->name));
        gen_atts(fd, p->info.typ->ref);
      }
      else if ((p->info.typ->type == Tpointer || p->info.typ->type == Treference)
             && (((Tnode*)p->info.typ->ref)->type == Tstruct || ((Tnode*)p->info.typ->ref)->type == Tclass))
      { if ((s = xsi_type(p->info.typ)) && *s)
          fprintf(fd, "%*s<%s xsi:type=\"%s\"", n, "", ns_convert(p->sym->name), s);
        else
          fprintf(fd, "%*s<%s", n, "", ns_convert(p->sym->name));
        gen_atts(fd, ((Tnode*)p->info.typ->ref)->ref);
      }
      else if ((s = xsi_type(p->info.typ)) && *s)
        fprintf(fd, "%*s<%s xsi:type=\"%s\">", n, "", ns_convert(p->sym->name), s);
      else
        fprintf(fd, "%*s<%s>", n, "", ns_convert(p->sym->name));
      switch (p->info.typ->type)
      { case Tchar:
        case Tshort:
        case Tint:
        case Tlong:
        case Tllong:
        case Tuchar:
        case Tushort:
        case Tuint:
        case Tulong:
        case Tullong:
          if (p->info.hasval)
	    fprintf(fd, SOAP_LONG_FORMAT, p->info.val.i);
	  else
	    fprintf(fd, "0");
	  break;
        case Tfloat:
        case Tdouble:
          if (p->info.hasval)
	    fprintf(fd, "%f", p->info.val.r);
	  else
	    fprintf(fd, "0.0");
          break;
        case Tenum:
	  if (p->info.hasval && p->info.typ->ref)
	  { for (q = ((Table*)p->info.typ->ref)->list; q; q = q->next)
	      if (p->info.val.i == q->info.val.i)
	      { fprintf(fd, "%s", ns_convert(q->sym->name));
		break;
	      }
	  }
	  else
	    gen_val(fd, n+1, p->info.typ);
	  break;
        case Tpointer:
        case Treference:
	  if (is_string(p->info.typ))
	  { if (p->info.hasval)
	      fprintf(fd, "%s", p->info.val.s);
	  }
	  else if (n < 10)
	    gen_val(fd, n, p->info.typ->ref);
	  break;
        case Tclass:
        case Tstruct:
	  if (!is_dynamic_array(p->info.typ))
	    gen_val(fd, n, p->info.typ);
	  break;
        }
	fprintf(fd, "</%s>\n", ns_convert(p->sym->name));
        fflush(fd);
    }
  }
}

void
gen_atts(FILE *fd, Table *p)
{ Entry *q, *r;
  if (p)
  for (q = p->list; q; q = q->next)
    if (q->info.sto & Sattribute)
    { fprintf(fd, " %s=\"", ns_convert(q->sym->name));
      switch (q->info.typ->type)
      { case Tchar:
        case Tshort:
        case Tint:
        case Tlong:
        case Tllong:
        case Tuchar:
        case Tushort:
        case Tuint:
        case Tulong:
        case Tullong:
          if (q->info.hasval)
	    fprintf(fd, SOAP_LONG_FORMAT, q->info.val.i);
          else
            fprintf(fd, "0");
          break;
        case Tfloat:
        case Tdouble:
          if (q->info.hasval)
	    fprintf(fd, "%f", q->info.val.r);
	  else
	    fprintf(fd, "0.0");
          break;
        case Ttime:
	  break; /* should print value? */
        case Tenum:
	  if (q->info.hasval && q->info.typ->ref)
	  { for (r = ((Table*)q->info.typ->ref)->list; r; r = r->next)
	      if (r->info.val.i == q->info.val.i)
	      { fprintf(fd, "%s", ns_convert(r->sym->name));
		break;
	      }
	  }
	  break;
        case Tpointer:
        case Treference:
	  if (is_string(q->info.typ))
	  { if (q->info.hasval)
	      fprintf(fd, "%s", q->info.val.s);
	  }
	  break;
      }
      fprintf(fd, "\"");
    }
  fprintf(fd, ">");
  fflush(fd);
}

void
gen_val(FILE *fd, int n, Tnode *p)
{ Entry *q;
  int i;
  if (!is_transient(p) && p->type != Tfun && !is_XML(p)) 
  { if (p->type == Tarray)
    { i = ((Tnode*) p->ref)->width;
      if (i)
      { i = p->width / i;
        if (i > 100)
          i = 100;
        fprintf(fd, "\n");
        for (; i > 0; i--)
        { fprintf(fd, "%*s<item>", n+1, "");
          gen_val(fd, n+1, p->ref);
          fprintf(fd, "</item>\n");
        }
        fprintf(fd, "%*s", n, "");
      }
    }
    else if (is_dynamic_array(p))
    { if (!is_binary(p))
      { fprintf(fd, "\n");
        gen_field(fd, n+1, ((Table*)p->ref)->list);
        fprintf(fd, "%*s", n, "");
      }
    }
    switch (p->type)
    { case Tchar:
      case Tshort:
      case Tint:
      case Tlong:
      case Tllong:
      case Tuchar:
      case Tushort:
      case Tuint:
      case Tulong:
      case Tullong:
	fprintf(fd, "0");
	break;
      case Tfloat:
      case Tdouble:
	fprintf(fd, "0.0");
        break;
      case Tenum:
        if (p->ref && (q = ((Table*)p->ref)->list))
          fprintf(fd, "%s", ns_convert(q->sym->name));
        else
          fprintf(fd, "0");
        break;
      case Tpointer:
      case Treference:
	if (!is_string(p) && !is_wstring(p) && n < 10)
	  gen_val(fd, n+1, p->ref);
	break;
      case Tclass:
      case Tstruct:
	if (!is_dynamic_array(p) && !is_primclass(p) && p->ref)
	{ fprintf(fd, "\n");
	  for (q = ((Table*)p->ref)->list; q; q = q->next)
	    gen_field(fd, n+1, q);
	  fprintf(fd, "%*s", n, "");
        }
        break;
    }
  }
}

FILE *
gen_env(char *buf, char *method, int response, Table *t, char *ns, char *name, char *URL, char *executable, char *URI, char *encoding)
{ Symbol *s;
  Service *sp;
  char tmp[1024];
  FILE *fd;
  strcpy(tmp, buf);
  strcpy(strrchr(tmp, '.')+1, method);
  if (!response)
  { strcat(tmp, ".req.xml");
    fprintf(stderr, "Saving %s sample SOAP/XML request\n", tmp);
  }
  else
  { strcat(tmp, ".res.xml");
    fprintf(stderr, "Saving %s sample SOAP/XML response\n", tmp);
  }
  fd = fopen(tmp, "w");
  if (!fd)
    execerror("Cannot write XML file");
  fprintf(fd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(fd, "<SOAP-ENV:Envelope");
  for (s = nslist; s; s = s->next)
  { for (sp = services; sp; sp = sp->next)
      if (!strcmp(sp->ns, s->name) && sp->URI)
        break;
    if (sp)
      fprintf(fd, "\n xmlns:%s=\"%s\"", ns_convert(s->name), sp->URI);
    else if (!strcmp(s->name, "SOAP-ENV"))
      fprintf(fd, "\n xmlns:SOAP-ENV=\"%s\"", envURI);
    else if (!strcmp(s->name, "SOAP-ENC"))
      fprintf(fd, "\n xmlns:SOAP-ENC=\"%s\"", encURI);
    else if (!strcmp(s->name, "xsi"))
      fprintf(fd, "\n xmlns:xsi=\"%s\"", xsiURI);
    else if (!strcmp(s->name, "xsd"))
      fprintf(fd, "\n xmlns:xsd=\"%s\"", xsdURI);
    else if (!strcmp(s->name, ns))
      fprintf(fd, "\n xmlns:%s=\"%s\"", ns_convert(s->name), URI);
    else
      fprintf(fd, "\n xmlns:%s=\"%s/%s.xsd\"", ns_convert(s->name), URI, ns_convert(s->name));
  }
  fprintf(fd, ">\n <SOAP-ENV:Body");
  if (!encoding)
    fprintf(fd, " encodingStyle=\"%s\"", encURI);
  else if (strcmp(encoding, "literal"))
    fprintf(fd, " encodingStyle=\"%s\"", encoding);
  fprintf(fd, ">\n");
  return fd;
}

char *
emalloc(unsigned int n)
{ char	*p;
  if ((p = (char*)malloc(n)) == (char*)0)
    execerror("out of memory");
  return p;
}

void
function_input_output(Table *table)
{
  int i=1;
  Entry * p;
  fprintf(fserver,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_serve(struct soap *soap)"); 
  fprintf(fserver,"\n{\n\tunsigned int n = SOAP_MAXKEEPALIVE;\n\tdo\n\t{\tsoap_begin(soap);");

  fprintf(fserver,"\n\t\tif (soap_begin_recv(soap) || soap_envelope_begin_in(soap) || soap_recv_header(soap) || soap_body_begin_in(soap))\n\t\t\treturn soap_send_fault(soap);");

  fprintf(fserver,"\n\t\tif (!--n)\n\t\t\tsoap->keep_alive = 0;");

  for (p = table->list; p != (Entry*) 0; p = p->next)
    {
      if(p->info.typ->type==Tfun)
	{              
	  if(i == 1){
	    /* First method to be called */
	    fprintf(fserver,"\n\t\tsoap->error = soap_serve_%s(soap);", p->sym->name);   
	  }
	  else{
	    /* Methods following other methods */
	    fprintf(fserver,"\n\t\tif (soap->error == SOAP_NO_METHOD)");
	    fprintf(fserver,"\n\t\t\tsoap_serve_%s(soap);", p->sym->name);
	  }
	  i++;
	}
    }
  
  fprintf(fserver,"\n\t\tif (soap->error)");
  fprintf(fserver,"\n\t\t\treturn soap_send_fault(soap);");
  fprintf(fserver,"\n\t} while (soap->keep_alive);");
  fprintf(fserver,"\n\treturn SOAP_OK;");
  fprintf(fserver,"\n}");
  
  fprintf(fheader, "\n\n/* Remote Methods */");
  for (p = table->list; p != (Entry*) 0; p = p->next)
    if (p->info.typ->type==Tfun)
	generate_proto(table, p);
  fprintf(fheader, "\n\n/* Stubs */");
  for (p = table->list; p != (Entry*) 0; p = p->next)
    if (p->info.typ->type==Tfun)
	generate_call(table, p);
  fprintf(fheader, "\n\n/* Skeletons */\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_serve(struct soap*);");
  for (p = table->list; p != (Entry*) 0; p = p->next)
    if (p->info.typ->type==Tfun)
	generate_server(table, p);
}

void
generate_proto(Table *table, Entry *param)
{ Entry *pin,*q,*pout;
  Table *output,*t;
  q=entry(table, param->sym);
  if (q)
    pout = (Entry*)q->info.typ->ref;
  else	fprintf(stderr, "Internal error: no table entry\n");
  q=entry(classtable, param->sym);
  output=(Table*) q->info.typ->ref;
  fprintf(fheader, "\n\nSOAP_FMAC1 int SOAP_FMAC2 %s(struct soap*",param->sym->name);
  for(t=output;t!=(Table*) 0;t=t->prev)
  { for (pin = t->list; pin != (Entry*) 0; pin = pin->next)
    {	if (pin->info.typ->type == Treference)
    	{	sprintf(errbuf, "Cannot pass input argument \"%s\" of remote method function \"%s\" by reference: use pointer", pin->sym->name, param->sym->name);
		semerror(errbuf);
	}
	  fprintf(fheader,", %s%s",c_storage(pin->info.sto), c_type(pin->info.typ));
    }
  }
  if (is_transient(pout->info.typ))
    fprintf(fheader,");");
  else
    fprintf(fheader,", %s%s);", c_storage(pout->info.sto), c_type(pout->info.typ));
}

void
generate_call(Table *table, Entry *param)
{ Service *sp;
  Entry *pin,*q,*pout,*response=NULL;
  Tnode *temp;
  Table *output,*t;
  char *s;
  Method *m;
  int cardinality, element_width, i, flag = 0;
  q=entry(table, param->sym);
  if (q)
    pout = (Entry*)q->info.typ->ref;
  else	fprintf(stderr, "Internal error: no table entry\n");
  q=entry(classtable, param->sym);
  output=(Table*) q->info.typ->ref;

  if (!is_response(pout->info.typ))
  { response = get_response(param->info.typ);
  }
  
  /* soapStub.h*/
  if (is_transient(pout->info.typ))
  { fprintf(fheader, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_send_%s(struct soap*, const char*, const char*",param->sym->name);
    fprintf(fclient, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_send_%s(struct soap *soap, const char *URL, const char *action",param->sym->name);
  }
  else
  { fprintf(fheader, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_call_%s(struct soap*, const char*, const char*",param->sym->name);
    fprintf(fclient, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_call_%s(struct soap *soap, const char *URL, const char *action",param->sym->name);
  }
  fflush(fheader);
  fflush(fclient);

  /* Parameters being passed */
  for(t=output;t!=(Table*) 0;t=t->prev)
  { for (pin = t->list; pin != (Entry*) 0; pin = pin->next)
    { fprintf(fclient,", %s%s",c_storage(pin->info.sto), c_type_id(pin->info.typ,pin->sym->name));
      fprintf(fheader,", %s%s",c_storage(pin->info.sto), c_type(pin->info.typ));
    }
  }

  /* Return value */
  fflush(fclient);
  if (!is_transient(pout->info.typ))
  { fprintf(fclient, ", %s%s)\n{", c_storage(pout->info.sto), c_type_id(pout->info.typ, pout->sym->name));
    fprintf(fheader,", %s%s);", c_storage(pout->info.sto), c_type(pout->info.typ));
  }
  else
  { fprintf(fclient, ")\n{");
    fprintf(fheader,");");
  }
  fflush(fclient);
  
  fprintf(fclient,"\n\tstruct %s soap_tmp_%s;",param->sym->name,param->sym->name);
  if (!is_response(pout->info.typ) && response)
  { fprintf(fclient,"\n\tstruct %s *soap_tmp_%s;",c_ident(response->info.typ), c_ident(response->info.typ));
  } 
  for (sp = services; sp; sp = sp->next)
  { if (has_ns_eq(sp->ns, param->sym->name))
    { if (sp->encoding && sp->URI)
      { flag = 1;
        fprintf(fclient, "\n\tconst char *soap_bak_encodingStyle = soap->encodingStyle;");
        fprintf(fclient, "\n\tconst char *soap_bak_defaultNamespace = soap->defaultNamespace;");
        fprintf(fclient, "\n\tint soap_bak_omode = soap->omode;");
        if (!strcmp(sp->encoding, "literal"))
        { fprintf(fclient, "\n\tsoap->encodingStyle = NULL;");
          fprintf(fclient, "\n\tsoap->defaultNamespace = \"%s\";", sp->URI);
	}
        else
        { fprintf(fclient, "\n\tsoap->encodingStyle = \"%s\";", sp->encoding);
          fprintf(fclient, "\n\tsoap->defaultNamespace = \"%s\";", sp->URI);
	}
        fprintf(fclient, "\n\tsoap->omode |= SOAP_XML_TREE;");
	break;
      }
      if ((s = strstr(param->sym->name, "__")))
        for (m = sp->list; m; m = m->next)
          if (m->part && m->mess == ACTION && !strcmp(m->name, s+2))
          { fprintf(fclient, "\n\tif (!action)\n\t\taction = ");
            if (*m->part == '"')
              fprintf(fclient, "%s;", m->part);
            else
              fprintf(fclient, "\"%s\";", m->part);
            break;
          }
    }
  }
  fflush(fclient);
  for(t=output;t!=(Table*) 0;t=t->prev) 
    for (pin = t->list; pin != (Entry*) 0; pin = pin->next)
      {
	if(pin->info.typ->type==Tarray)
	  {
	    temp = pin->info.typ;
	    cardinality = 0;
	    while(temp->type == Tarray){
	      cardinality ++;
	      temp = temp->ref;
	    }
	    element_width = temp->width;
	    
	    fprintf(fclient,"\n\tmemcpy(&(soap_tmp_%s.%s",
		    param->sym->name,pin->sym->name);
	    for(i=0;i<cardinality;i++){
	      fprintf(fclient,"[0]");
	    }
	    fprintf(fclient,"), ");
            fprintf(fclient,"&(%s", pin->sym->name);
            for(i=0;i<cardinality;i++){
              fprintf(fclient,"[0]");
            }
            fprintf(fclient,"), %d*sizeof(%s));", (pin->info.typ->width)/(element_width), c_type(temp));      
	  }			
	else fprintf (fclient,"\n\tsoap_tmp_%s.%s=%s;",
		      param->sym->name,pin->sym->name,pin->sym->name);
      }	
  fprintf(fclient,"\n\tsoap_begin(soap);");
  fprintf(fclient,"\n\tsoap_serializeheader(soap);");
  fprintf(fclient,"\n\tsoap_serialize_%s(soap, &soap_tmp_%s);", param->sym->name,param->sym->name);
  fprintf(fclient,"\n\tsoap_begin_count(soap);");
  fprintf(fclient,"\n\tif (soap->mode & SOAP_IO_LENGTH)");
  fprintf(fclient,"\n\t{\tsoap_envelope_begin_out(soap);");
  fprintf(fclient,"\n\t\tsoap_putheader(soap);");
  fprintf(fclient,"\n\t\tsoap_body_begin_out(soap);");
  fprintf(fclient,"\n\t\tsoap_put_%s(soap, &soap_tmp_%s, \"%s\", \"\");", param->sym->name,param->sym->name, ns_convert(param->sym->name)); 
  fprintf(fclient,"\n\t\tsoap_body_end_out(soap);");
  fprintf(fclient,"\n\t\tsoap_envelope_end_out(soap);");
  fprintf(fclient,"\n\t}");
  fprintf(fclient,"\n\tif (soap_connect(soap, URL, action)");
  fprintf(fclient,"\n\t || soap_envelope_begin_out(soap)");
  fprintf(fclient,"\n\t || soap_putheader(soap)");
  fprintf(fclient,"\n\t || soap_body_begin_out(soap)");
  fprintf(fclient,"\n\t || soap_put_%s(soap, &soap_tmp_%s, \"%s\", \"\")", param->sym->name,param->sym->name,ns_convert(param->sym->name)); 
  fprintf(fclient,"\n\t || soap_body_end_out(soap)");
  fprintf(fclient,"\n\t || soap_envelope_end_out(soap)");
  fprintf(fclient,"\n\t || soap_putattachments(soap)");
  fprintf(fclient,"\n\t || soap_end_send(soap))");
  if (flag)
  { fprintf(fclient, "\n\t{\tsoap->encodingStyle = soap_bak_encodingStyle;");
    fprintf(fclient, "\n\t\tsoap->defaultNamespace = soap_bak_defaultNamespace;");
    fprintf(fclient, "\n\t\tsoap->omode = soap_bak_omode;");
    fprintf(fclient, "\n\t\treturn soap->error;\n\t}");
  }
  else
    fprintf(fclient, "\n\t\treturn soap->error;");
  fflush(fclient);
  
  if (is_transient(pout->info.typ))
  { fprintf(fclient, "\n\treturn SOAP_OK;\n}");
    fprintf(fheader, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_recv_%s(struct soap*, ",param->sym->name);
    fprintf(fclient, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_recv_%s(struct soap *soap, ",param->sym->name);
    fprintf(fheader,"struct %s*);\n",param->sym->name);
    fprintf(fclient,"struct %s *%s)\n{",param->sym->name,pout->sym->name);
    fprintf(fclient,"\n\tsoap_default_%s(soap, %s);", param->sym->name,pout->sym->name);
    fprintf(fclient,"\n\tsoap_begin(soap);");
  }
  else if(pout->info.typ->type == Tarray)
    fprintf(fclient,"\n\tsoap_default_%s(soap, %s);", c_ident(pout->info.typ), pout->sym->name);
  else if (pout->info.typ->type == Treference && ((Tnode*)pout->info.typ->ref)->type == Tclass && !is_external(pout->info.typ->ref))
    fprintf(fclient,"\n\tif (&%s)\n\t\t%s.soap_default(soap);", pout->sym->name, pout->sym->name);
  else if (((Tnode*)pout->info.typ->ref)->type == Tclass && !is_external(pout->info.typ->ref))
    fprintf(fclient,"\n\tif (%s)\n\t\t%s->soap_default(soap);", pout->sym->name, pout->sym->name);
  else if (pout->info.typ->type == Treference && is_XML(pout->info.typ->ref))
    fprintf(fclient,"\n\t%s = NULL;", pout->sym->name);
  else if (pout->info.typ->type == Tpointer && is_XML(pout->info.typ->ref))
    fprintf(fclient,"\n\t*%s = NULL;", pout->sym->name);
  else if (pout->info.typ->type == Treference)
    fprintf(fclient,"\n\tsoap_default_%s(soap, &%s);", c_ident((Tnode*)pout->info.typ->ref), pout->sym->name);
  else if (!is_void(pout->info.typ))
    fprintf(fclient,"\n\tsoap_default_%s(soap, %s);", c_ident((Tnode*)pout->info.typ->ref), pout->sym->name);
  fflush(fclient);

  fprintf(fclient,"\n\tif (soap_begin_recv(soap)");
  fprintf(fclient,"\n\t || soap_envelope_begin_in(soap)");
  fprintf(fclient,"\n\t || soap_recv_header(soap)");
  fprintf(fclient,"\n\t || soap_body_begin_in(soap))");
  fprintf(fclient,"\n\t\treturn soap->error;");
  fflush(fclient);

  if (is_transient(pout->info.typ))
  {
  fprintf(fclient,"\n\tsoap_get_%s(soap, %s, \"%s\", NULL);", param->sym->name, pout->sym->name, ns_convert(param->sym->name));
  fprintf(fclient,"\n\tif (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)\n\t\tsoap->error = SOAP_NO_METHOD;");
  fprintf(fclient,"\n\tif (soap->error)");
  fprintf(fclient,"\n\t\treturn soap->error;");
  fprintf(fclient,"\n\tif (soap_body_end_in(soap)");
  fprintf(fclient,"\n\t || soap_envelope_end_in(soap)");
  fprintf(fclient,"\n\t || soap_getattachments(soap)");
  fprintf(fclient,"\n\t || soap_end_recv(soap))");
  fprintf(fclient,"\n\t\treturn soap->error;");
  fprintf(fclient,"\n\treturn SOAP_OK;\n}");
  fflush(fclient);
  return;
  }
  
  if(pout->info.typ->type == Treference && ((Tnode *) pout->info.typ->ref)->type == Tclass && !is_external(pout->info.typ->ref) && !is_dynamic_array(pout->info.typ->ref))
    fprintf(fclient,"\n\tif (&%s)\n\t\t%s.soap_get(soap, \"%s\", \"%s\");\n\telse\n\t\tsoap_get_%s(soap, &%s, \"%s\", \"%s\");", pout->sym->name,
    pout->sym->name, xml_tag(pout->info.typ), xsi_type(pout->info.typ), c_ident(pout->info.typ->ref), pout->sym->name, xml_tag(pout->info.typ), xsi_type(pout->info.typ));
  else if(pout->info.typ->type == Tpointer && ((Tnode *) pout->info.typ->ref)->type == Tclass && !is_external(pout->info.typ->ref) && !is_dynamic_array(pout->info.typ->ref))
    fprintf(fclient,"\n\tif (%s)\n\t\t%s->soap_get(soap, \"%s\", \"%s\");\n\telse\n\t\tsoap_get_%s(soap, %s, \"%s\", \"%s\");", pout->sym->name,
    pout->sym->name, xml_tag(pout->info.typ), xsi_type(pout->info.typ), c_ident(pout->info.typ->ref), pout->sym->name, xml_tag(pout->info.typ), xsi_type(pout->info.typ));
  else if(pout->info.typ->type == Treference && ((Tnode *) pout->info.typ->ref)->type == Tstruct && !is_dynamic_array(pout->info.typ->ref))
    fprintf(fclient,"\n\tsoap_get_%s(soap, &%s, \"%s\", \"%s\");", c_ident(pout->info.typ->ref), pout->sym->name, xml_tag(pout->info.typ), xsi_type(pout->info.typ));
  else if(pout->info.typ->type == Tpointer && ((Tnode *) pout->info.typ->ref)->type == Tstruct && !is_dynamic_array(pout->info.typ->ref))
    fprintf(fclient,"\n\tsoap_get_%s(soap, %s, \"%s\", \"%s\");", c_ident(pout->info.typ->ref), pout->sym->name, xml_tag(pout->info.typ), xsi_type(pout->info.typ));
  else
  {
    fprintf(fclient,"\n\tsoap_tmp_%s = soap_get_%s(soap, NULL, \"%s\", \"%s\");", c_ident(response->info.typ), c_ident(response->info.typ), xml_tag(response->info.typ), xsi_type(response->info.typ));
  fflush(fclient);
  }
  fflush(fclient);
  fprintf(fclient,"\n\tif (soap->error)");
  fprintf(fclient,"\n\t{\tif (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)\n\t\t\tsoap_recv_fault(soap);");
  fprintf(fclient,"\n\t\treturn soap->error;");
  fprintf(fclient,"\n\t}");

  fprintf(fclient,"\n\tif (soap_body_end_in(soap)");
  fprintf(fclient,"\n\t || soap_envelope_end_in(soap)");
  fprintf(fclient,"\n\t || soap_getattachments(soap)");
  fprintf(fclient,"\n\t || soap_end_recv(soap))");
  fprintf(fclient,"\n\t\treturn soap->error;");
  fprintf(fclient,"\n\tsoap_closesock(soap);");
  if (!is_response(pout->info.typ))
  { if (pout->info.typ->type == Tarray)
	{    temp = pout->info.typ;
	    cardinality = 0;
	    while(temp->type == Tarray){
	      cardinality ++;
	      temp = temp->ref;
	    }
	    element_width = temp->width;
	    
	    fprintf(fclient,"\n\tmemcpy(&(%s", pout->sym->name);
	    for(i=0;i<cardinality;i++){
	      fprintf(fclient,"[0]");
	    }
	    fprintf(fclient,"), ");
            fprintf(fclient,"&(soap_tmp_%s->%s", c_ident(response->info.typ), pout->sym->name);
            for(i=0;i<cardinality;i++){
              fprintf(fclient,"[0]");
            }
            fprintf(fclient,"), %d*sizeof(%s));", (pout->info.typ->width)/(element_width), c_type(temp));      
	}
    else if (pout->info.typ->type == Treference)
      fprintf(fclient,"\n\t%s = soap_tmp_%s->%s;", pout->sym->name, c_ident(response->info.typ), pout->sym->name);
    else
    { fprintf(fclient,"\n\tif (soap_tmp_%s->%s)", c_ident(response->info.typ), pout->sym->name);
      fprintf(fclient,"\n\t\t*%s = *soap_tmp_%s->%s;", pout->sym->name, c_ident(response->info.typ), pout->sym->name);
    }
  }
  fprintf(fclient,"\n\treturn SOAP_OK;");
  fprintf(fclient ,"\n}");
  fflush(fclient);
}

void
generate_server(Table *table, Entry *param)
{ Service *sp;
  int flag = 0;
  Entry *q,*pin, *pout, *response;
  Table *t, *input;
  q=entry(table, param->sym);
  if (q)
    pout = (Entry*)q->info.typ->ref;
  else
    fprintf(stderr, "Internal error: no table entry");
  if (!is_response(pout->info.typ))
    response = get_response(param->info.typ);
  fprintf(fheader, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_serve_%s(struct soap*);",param->sym->name);
  fprintf(fserver, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_serve_%s(struct soap *soap)",param->sym->name);
  fprintf(fserver, "\n{\tstruct %s soap_tmp_%s;", param->sym->name, param->sym->name);
  if (!is_transient(pout->info.typ))
  for (sp = services; sp; sp = sp->next)
    if (has_ns_eq(sp->ns, param->sym->name))
      if (sp->encoding && sp->URI)
      { flag = 1;
        fprintf(fserver, "\n\tconst char *soap_bak_encodingStyle = soap->encodingStyle;");
        fprintf(fserver, "\n\tconst char *soap_bak_defaultNamespace = soap->defaultNamespace;");
        fprintf(fserver, "\n\tshort soap_bak_omode = soap->omode;");
	break;
      }
  fflush(fserver);
  if (is_transient(pout->info.typ))
    ;
  else if(pout->info.typ->type == Tarray){
    /* ARRAY */
    fprintf(fserver,"\n\tstruct %s soap_tmp_%s;", c_ident(response->info.typ), c_ident(response->info.typ));
    fprintf(fserver,"\n\tsoap_default_%s(soap, &soap_tmp_%s);", c_ident(response->info.typ),c_ident(response->info.typ));
  }else if(((Tnode *)pout->info.typ->ref)->type == Tclass && !is_external(pout->info.typ->ref) && !is_dynamic_array(pout->info.typ->ref)){
    /* CLASS */
    fprintf(fserver, "\n\t%s;", c_type_id((Tnode*)pout->info.typ->ref, pout->sym->name));
    fprintf(fserver,"\n\t%s.soap_default(soap);", pout->sym->name);
  }else if(((Tnode *)pout->info.typ->ref)->type == Tstruct && !is_dynamic_array(pout->info.typ->ref)){
    fprintf(fserver, "\n\t%s;", c_type_id((Tnode*)pout->info.typ->ref, pout->sym->name));
    fprintf(fserver,"\n\tsoap_default_%s(soap, &%s);", c_ident((Tnode *)pout->info.typ->ref),pout->sym->name);  
  } else if (pout->info.typ->type == Tpointer) {
    fprintf(fserver,"\n\tstruct %s soap_tmp_%s;", c_ident(response->info.typ), c_ident(response->info.typ));
    fprintf(fserver,"\n\t%s soap_tmp_%s;", c_type(pout->info.typ->ref), c_ident(pout->info.typ->ref));
    fprintf(fserver,"\n\tsoap_default_%s(soap, &soap_tmp_%s);", c_ident(response->info.typ),c_ident(response->info.typ));
    if (((Tnode*)pout->info.typ->ref)->type == Tclass && !is_external(pout->info.typ->ref))
      fprintf(fserver,"\n\tsoap_tmp_%s.soap_default(soap);", c_ident(pout->info.typ->ref));
    else
      fprintf(fserver,"\n\tsoap_default_%s(soap, &soap_tmp_%s);", c_ident(pout->info.typ->ref),c_ident(pout->info.typ->ref));
    fprintf(fserver,"\n\tsoap_tmp_%s.%s = &soap_tmp_%s;", c_ident(response->info.typ),pout->sym->name, c_ident(pout->info.typ->ref));
  } else  {
    fprintf(fserver,"\n\tstruct %s soap_tmp_%s;", c_ident(response->info.typ), c_ident(response->info.typ));
    fprintf(fserver,"\n\tsoap_default_%s(soap, &soap_tmp_%s);", c_ident(response->info.typ),c_ident(response->info.typ));
  }
  fprintf(fserver,"\n\tsoap_default_%s(soap, &soap_tmp_%s);", param->sym->name,param->sym->name);
  fflush(fserver);
  fprintf(fserver,"\n\tsoap_get_%s(soap, &soap_tmp_%s, \"%s\", NULL);", param->sym->name, param->sym->name, ns_convert(param->sym->name));
  fprintf(fserver,"\n\tif (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)\n\t\tsoap->error = SOAP_NO_METHOD;");
  fprintf(fserver,"\n\tif (soap->error)");
  fprintf(fserver,"\n\t\treturn soap->error;");
  fprintf(fserver,"\n\t");
  fprintf(fserver,"\n\tif (soap_body_end_in(soap)");
  fprintf(fserver,"\n\t || soap_envelope_end_in(soap)");
  fprintf(fserver,"\n\t || soap_getattachments(soap)");
  fprintf(fserver,"\n\t || soap_end_recv(soap))\n\t\treturn soap->error;");
  fprintf(fserver, "\n\tsoap->error = %s(soap",param->sym->name);
  fflush(fserver);
  q=entry(classtable, param->sym);
  input=(Table*) q->info.typ->ref;

  for (t=input; t!=(Table*)0; t=t->prev) 
    for (pin = t->list; pin != (Entry*) 0; pin = pin->next)
      fprintf(fserver,", soap_tmp_%s.%s",param->sym->name, pin->sym->name);

  if (is_transient(pout->info.typ))
    fprintf(fserver, ");");
  else  if(pout->info.typ->type == Tarray){
    fprintf(fserver, ", soap_tmp_%s.%s);", c_ident(response->info.typ), pout->sym->name);
  }else if(pout->info.typ->type == Treference && (((Tnode*)pout->info.typ->ref)->type == Tstruct || ((Tnode*)pout->info.typ->ref)->type == Tclass) && !is_external(pout->info.typ->ref) && !is_dynamic_array(pout->info.typ->ref)) {
    fprintf(fserver, ", %s);", pout->sym->name);
  }else if((((Tnode*)pout->info.typ->ref)->type == Tstruct || ((Tnode*)pout->info.typ->ref)->type == Tclass) && !is_external(pout->info.typ->ref) && !is_dynamic_array(pout->info.typ->ref)) {
    fprintf(fserver, ", &%s);", pout->sym->name);
  }else if(pout->info.typ->type == Treference){
    fprintf(fserver, ", soap_tmp_%s.%s);", c_ident(response->info.typ), pout->sym->name);
  }else{
    fprintf(fserver, ", &soap_tmp_%s);", c_ident(pout->info.typ->ref));
  }
  fprintf(fserver,"\n\tif (soap->error)\n\t\treturn soap->error;");
  
  if (!is_transient(pout->info.typ))
  {
  if (flag)
  {     if (!strcmp(sp->encoding, "literal"))
        { fprintf(fserver, "\n\tsoap->encodingStyle = NULL;");
          fprintf(fserver, "\n\tsoap->defaultNamespace = \"%s\";", sp->URI);
	}
        else
        { fprintf(fserver, "\n\tsoap->encodingStyle = \"%s\";", sp->encoding);
          fprintf(fserver, "\n\tsoap->defaultNamespace = \"%s\";", sp->URI);
	}
        fprintf(fserver, "\n\tsoap->mode &= SOAP_XML_TREE;");
  }
  fprintf(fserver,"\n\tsoap_serializeheader(soap);");
  DBGLOG(fprintf(stderr,"\n##%d##\n",pout->info.typ->type == Tarray));
  if(pout->info.typ->type == Tarray){
    fprintf(fserver, "\n\tsoap_serialize_%s(soap, &soap_tmp_%s);", c_ident(response->info.typ), c_ident(response->info.typ));
  }else if(((Tnode *)pout->info.typ->ref)->type == Tclass && !is_external(pout->info.typ->ref) && !is_dynamic_array(pout->info.typ->ref)){
    fprintf(fserver, "\n\t%s.soap_serialize(soap);", pout->sym->name);
  }else if(((Tnode *)pout->info.typ->ref)->type == Tstruct && !is_dynamic_array(pout->info.typ->ref)){
    fprintf(fserver, "\n\tsoap_serialize_%s(soap, &%s);", c_ident((Tnode*)pout->info.typ->ref), pout->sym->name);
  }else{
    fprintf(fserver, "\n\tsoap_serialize_%s(soap, &soap_tmp_%s);", c_ident(response->info.typ), c_ident(response->info.typ));
  }
  fprintf(fserver, "\n\tsoap_begin_count(soap);");
  fprintf(fserver, "\n\tif (soap->mode & SOAP_IO_LENGTH)");
  fprintf(fserver, "\n\t{\tsoap_envelope_begin_out(soap);");
  fprintf(fserver,"\n\t\tsoap_putheader(soap);");
  fprintf(fserver,"\n\t\tsoap_body_begin_out(soap);");
  if(pout->info.typ->type == Tarray){
    fprintf(fserver,"\n\t\tsoap_put_%s(soap, &soap_tmp_%s.%s, \"%s\", \"\");",
	    c_ident(response->info.typ), c_ident(response->info.typ), pout->sym->name, xml_tag(pout->info.typ->ref));
  }else if(((Tnode *)pout->info.typ->ref)->type == Tclass && !is_external(pout->info.typ->ref) && !is_dynamic_array(pout->info.typ->ref)){
    fprintf(fserver, "\n\t\t%s.soap_put(soap, \"%s\", \"\");", pout->sym->name, xml_tag(pout->info.typ->ref));
  }else if(((Tnode *)pout->info.typ->ref)->type == Tstruct && !is_dynamic_array(pout->info.typ->ref)){
    fprintf(fserver, "\n\t\tsoap_put_%s(soap, &%s, \"%s\", \"\");",
	    c_ident((Tnode*)pout->info.typ->ref), pout->sym->name, xml_tag(pout->info.typ->ref));
  }else 
  { 
    fprintf(fserver,"\n\t\tsoap_put_%s(soap, &soap_tmp_%s, \"%s\", \"\");",
	    c_ident(response->info.typ), c_ident(response->info.typ), xml_tag(response->info.typ));
  }
  fprintf(fserver,"\n\t\tsoap_body_end_out(soap);");
  fprintf(fserver,"\n\t\tsoap_envelope_end_out(soap);");
  fprintf(fserver,"\n\t};");

  /* PRINTING PHASE */
  fprintf(fserver,"\n\tif (soap_response(soap, SOAP_OK)");
  fprintf(fserver,"\n\t || soap_envelope_begin_out(soap)");
  fprintf(fserver,"\n\t || soap_putheader(soap)");
  fprintf(fserver,"\n\t || soap_body_begin_out(soap)");
  if(pout->info.typ->type == Tarray){
    /* ARRAY */
    fprintf(fserver,"\n\t || soap_put_%s(soap, &soap_tmp_%s.%s, \"%s\", \"\")",
	    c_ident(response->info.typ), c_ident(response->info.typ), pout->sym->name, xml_tag(pout->info.typ->ref));
  }else if(((Tnode *)pout->info.typ->ref)->type == Tclass && !is_external(pout->info.typ->ref) && !is_dynamic_array(pout->info.typ->ref)){
    /* CLASS */
    fprintf(fserver, "\n\t || %s.soap_put(soap, \"%s\", \"\")", pout->sym->name, xml_tag(pout->info.typ->ref));
  }else if(((Tnode *)pout->info.typ->ref)->type == Tstruct && !is_dynamic_array(pout->info.typ->ref)){
    fprintf(fserver, "\n\t || soap_put_%s(soap, &%s, \"%s\", \"\")",
	    c_ident((Tnode*)pout->info.typ->ref), pout->sym->name, xml_tag(pout->info.typ->ref));
  }else 
  { 
    fprintf(fserver,"\n\t || soap_put_%s(soap, &soap_tmp_%s, \"%s\", \"\")",
	    c_ident(response->info.typ), c_ident(response->info.typ), xml_tag(response->info.typ));
  }
  fprintf(fserver,"\n\t || soap_body_end_out(soap)");
  fprintf(fserver,"\n\t || soap_envelope_end_out(soap)");
  fprintf(fserver,"\n\t || soap_putattachments(soap)");
  fprintf(fserver,"\n\t || soap_end_send(soap))");
  if (flag)
  { fprintf(fserver, "\n\t{\tsoap->encodingStyle = soap_bak_encodingStyle;");
    fprintf(fserver, "\n\t\tsoap->defaultNamespace = soap_bak_defaultNamespace;");
    fprintf(fserver, "\n\t\tsoap->omode = soap_bak_omode;");
    fprintf(fserver, "\n\t\treturn soap->error;\n\t}");
  }
  else
    fprintf(fserver, "\n\t\treturn soap->error;");
  }

  if (flag)
  { fprintf(fserver, "\n\tsoap->encodingStyle = soap_bak_encodingStyle;");
    fprintf(fserver, "\n\tsoap->defaultNamespace = soap_bak_defaultNamespace;");
    fprintf(fserver, "\n\tsoap->omode = soap_bak_omode;");
  }
  fprintf(fserver,"\n\tsoap_closesock(soap);");
  fprintf(fserver,"\n\treturn SOAP_OK;");
  fprintf(fserver,"\n}");
  fflush(fserver);
}

int
is_qname(Tnode *p)
{ return p->sym && is_string(p) && (!strcmp(p->sym->name, "xsd__QName") || !strcmp(p->sym->name, "QName"));
}

int
is_XML(Tnode *p)
{ return p->sym && (is_string(p) || is_wstring(p)) && !strcmp(p->sym->name, "XML");
}

int
is_response(Tnode *p)
{ return (p->type == Tpointer || p->type == Treference) && p->ref && ((((Tnode*)p->ref)->type == Tstruct || ((Tnode*)p->ref)->type == Tclass) && !is_external(p->ref) && !is_primclass(p->ref) && !is_dynamic_array(p->ref));
}

Entry*
get_response(Tnode *p)
{ if (p->type == Tfun)
    return p->response;
  return 0;
}

int
is_unmatched(Symbol *sym)
{ return *sym->name == '_';
}

int
is_untyped(Tnode *typ)
{ Tnode *p;
  if (typ->sym)
    return is_unmatched(typ->sym);
  if (typ->type == Tstruct || typ->type == Tclass)
    if (is_dynamic_array(typ) && !has_ns(typ) && !is_binary(typ))
    { p = ((Table*)typ->ref)->list->info.typ->ref;
      if (p->type == Tpointer)
        if (is_primitive(p->ref) || is_string(p->ref) || is_wstring(p->ref))
	  return 0;
        else
          return is_untyped(p);
      else if (is_primitive(p) || is_string(p) || is_wstring(p))
        return 0;
      else
        return is_untyped(p);
    }
    else
      return is_unmatched(typ->id);
  if (typ->type == Tpointer || typ->type == Treference || typ->type == Tarray)
    return is_untyped(typ->ref);
  if (typ->type == Tenum)
    return !has_ns(typ);
  if ((is_primitive(typ) || is_string(typ) || is_wstring(typ)))
    return 1;
  return 0;
}

int
is_primclass(Tnode *typ)
{ Table *t;
  if (typ->type == Tstruct || typ->type == Tclass)
  { if (!is_dynamic_array(typ))
      for (t = (Table*)typ->ref; t; t = t->prev)
        if (t->list && !strcmp(t->list->sym->name, "__item"))
          return 1;
  }
  else if (typ->type == Tpointer || typ->type == Treference)
    return is_primclass(typ->ref);
  return 0;
}

int
is_mask(Tnode *typ)
{ return (typ->type == Tenum && typ->width == 8);
}

int
is_void(Tnode *typ)
{ if (typ->type == Tvoid)
    return 1;
  if (typ->type == Tpointer)
    return is_void(typ->ref); 
  if (typ->type == Treference)
    return is_void(typ->ref); 
  if (typ->type == Tarray)
    return is_void(typ->ref); 
  return 0;
}

int
is_transient(Tnode *typ)
{ if (is_external(typ))
    return 0;
  if (typ->transient || typ->type == Tvoid || typ->type == Tunion || typ->type == Tnone)
    return 1;
  switch (typ->type)
  { case Tpointer:
    case Treference:
    case Tarray:
      return is_transient(typ->ref);
    case Tstruct:
      return typ->id == lookup("soap");
  }
  return 0;
}

int
is_external(Tnode* typ)
{ return typ->transient < 0;
}

int
is_repetition(Entry *p)
{ if (p)
    return p->next && p->next->info.typ->type == Tpointer && (p->info.typ->type == Tint || p->info.typ->type == Tuint) && !strncmp(p->sym->name, "__size", 6);
  return 0;
} 

int
has_detail_string()
{ Entry *p = entry(classtable, lookup("SOAP_ENV__Fault"));
  if (p && (p->info.typ->type == Tstruct || p->info.typ->type == Tclass))
  { Entry *e = entry(p->info.typ->ref, lookup("detail"));
    return e && is_string(e->info.typ);
  }
  return 0;
}

int
has_Detail_string()
{ Entry *p = entry(classtable, lookup("SOAP_ENV__Fault"));
  if (p && (p->info.typ->type == Tstruct || p->info.typ->type == Tclass))
  { Entry *e = entry(p->info.typ->ref, lookup("SOAP_ENV__Detail"));
    return e && is_string(e->info.typ);
  }
  return 0;
}

int
has_ns(Tnode *typ)
{ char *s;
  if (typ->type == Tstruct || typ->type == Tclass || typ->type == Tenum)
  { s = strstr(typ->id->name, "__");
    return s && s[2] && s[2] != '_';
  }
  return 0;
}

int
has_ns_eq(char *ns, char *name)
{ int n;
  if ((n = strlen(ns)) < strlen(name))
    return name[n] == '_' && name[n+1] == '_' && !strncmp(ns, name, n);
  return 0;
}

char *
ns_overridden(Table *t, Entry *p)
{ Entry *q;
  Symbol *s = t->sym;
  char *n;
  if (s)
    while (t = t->prev)
      for (q = t->list; q; q = q->next)
        if (!strcmp(q->sym->name, p->sym->name))
        { n = (char*)emalloc(strlen(s->name)+strlen(p->sym->name)+2);
	  strcpy(n, s->name);
	  strcat(n, ".");
	  strcat(n, p->sym->name);
	  return ns_convert(n);
        }
  return ns_convert(p->sym->name);
}

/*c_ident gives the names of the function for a type*/

char *
c_ident(Tnode *typ)
{ char *p;
  
  /*if( dumptypename(typetable,typ)!=0)
    {
      p=(char*) malloc(strlen(dumptypename(typetable,typ))*sizeof(char));
      strcpy(p,dumptypename(typetable,typ));
      DBGLOG(fprintf(stderr,"\n Return before switch in c_ident() %s\n", p));
      return p;
    }*/
  if (typ->sym)
    return typ->sym->name;

  switch(typ->type){
  case Tnone :
    return "";
  case Tvoid :
    return "void";
  case Tchar :
    return "byte";
  case Twchar :
    return "wchar";
  case Tshort :
    return "short";
  case Tint  :
    return "int";
  case Tlong  :
    return "long";
  case Tllong  :
    return "LONG64";
  case Tfloat:
    return "float";
  case Tdouble:
    return "double";
  case Tuchar:
    return "unsignedByte";
  case Tushort:
    return "unsignedShort";
  case Tuint:
    return "unsignedInt";
  case Tulong:
    return "unsignedLong";
  case Tullong:
    return "unsignedLONG64";
  case Ttime:
    return "time";
  case Tstruct:
  case Tclass:
  case Tunion:
  case Tenum:
    if (typ->ref == booltable)
      return "bool";
    return typ->id->name;
  case Treference:
    return c_ident(typ->ref);
  case Tpointer:
    if(((Tnode*)typ->ref)->type == Tchar) 
	return "string";
    if(((Tnode*)typ->ref)->type == Twchar) 
	return "wstring";
    p=(char*) malloc((10+strlen(c_ident(typ->ref)))*sizeof(char));
    strcpy(p,"PointerTo");
    strcat(p,c_ident(typ->ref));
    return p;
  case Tarray :p=(char*) malloc((16+strlen(c_ident(typ->ref)))*sizeof(char));
    if (((Tnode*)typ->ref)->width)
      sprintf(p, "Array%dOf%s",typ->width / ((Tnode*) typ->ref)->width,c_ident(typ->ref));
    else
      sprintf(p, "ArrayOf%s", c_ident(typ->ref));
    return p;
  }
  return "anyType";
}

char *
ns_convert(char *tag)
{ char *t, *s;
  int i, n;
  if (*tag == '_')
    if (!strncmp(tag, "__ptr", 5))
      if (tag[5])
        tag += 5;
      else
        tag = "item";
    else
      tag++;
  for (n = strlen(tag); n > 0; n--)
    if (tag[n-1] != '_')
      break;
  s = t = (char*)malloc(n+1);
  for (i = 0; i < n; i++)
  { if (tag[i] == '_')
      if (tag[i+1] == '_')
        break;
      else if (!strncmp(tag+i, "_DOT_", 5))
      { *s++ = '.';
        i += 4;
      }
      else if (!strncmp(tag+i, "_USCORE_", 8))
      { *s++ = '_';
        i += 7;
      }
      else if (!strncmp(tag+i, "_CDOT_", 6))
      { *s++ = 0xB7;
        i += 5;
      }
      else
        *s++ = '-';
    else
      *s++ = tag[i];
  }
  if (i < n)
  { *s++ = ':';
    for (i += 2; i < n; i++)
      if (tag[i] == '_')
        if (!strncmp(tag+i, "_DOT_", 5))
        { *s++ = '.';
          i += 4;
        }
        else if (!strncmp(tag+i, "_USCORE_", 8))
        { *s++ = '_';
          i += 7;
        }
        else if (!strncmp(tag+i, "_CDOT_", 6))
        { *s++ = 0xB7;
          i += 5;
        }
	else
	  *s++ = '-';
      else
        *s++ = tag[i];
  }
  *s = '\0';
  return t;
}

char *
ns_remove(char *tag)
{ char *t, *s = ns_convert(tag);
  if (t = strchr(s, ':'))
    return t+1;
  return s;
}

char *
xsi_type(Tnode *typ)
{ if (!typ)
    return "NULL";
  if (is_dynamic_array(typ) && !has_ns(typ))
    return xsi_type_Darray(typ);
  if (typ->type == Tarray)
    return xsi_type_Tarray(typ);
  if (is_untyped(typ))
    return "";
  if (typ->sym)
    if (!strncmp(typ->sym->name, "SOAP_ENV__", 10))
      return "";
    else
      return ns_convert(typ->sym->name);
  if (is_string(typ) || is_wstring(typ))
    return "string";
  switch(typ->type){
  case Tchar :
    return "byte";
  case Twchar :
    return "wchar";
  case Tshort :
    return "short";
  case Tint  :
    return "int";
  case Tlong  :
  case Tllong  :
    return "long";
  case Tfloat:
    return "float";
  case Tdouble:
    return "double";
  case Tuchar:
    return "unsignedByte";
  case Tushort:
    return "unsignedShort";
  case Tuint:
    return "unsignedInt";
  case Tulong:
  case Tullong:
    return "unsignedLong";
  case Ttime:
    return "dateTime";
  case Tpointer:
  case Treference:
    return xsi_type(typ->ref);
  case Tenum:
    if (typ->ref == booltable)
      return "boolean";
  case Tstruct:
  case Tclass:
    if (!has_ns(typ) || !strncmp(typ->id->name, "SOAP_ENV__", 10))
      return "";
    return ns_convert(typ->id->name);
  }
  return "";
}

char *
xml_tag(Tnode *typ)
{ if (!typ)
    return "NULL";
  if (typ->type == Tpointer || typ->type == Treference)
    return xml_tag(typ->ref);
  if (typ->sym)
    return ns_convert(typ->sym->name);
  return the_type(typ);
}

char *
wsdl_type(Tnode *typ, char *ns)
{ if (!typ)
    return "NULL";
  if (typ->sym)
    if (ns)
      return ns_convert(typ->sym->name);
    else
      return ns_remove(typ->sym->name);
   return base_type(typ, ns);
}

char *
base_type(Tnode *typ, char *ns)
{ int d;
  char *s, *t;
  if (is_string(typ) || is_wstring(typ))
    if (ns)
      return "xsd:string";
    else
      return "string";
  if (is_dynamic_array(typ) && !has_ns(typ) && !is_untyped(typ))
  { s = ns_remove(wsdl_type(((Table*)typ->ref)->list->info.typ, NULL));
    if (ns && *ns)
    { t = (char*)malloc(strlen(s)+strlen(ns_convert(ns))+13);
      strcpy(t, ns_convert(ns));
      strcat(t, ":");
      strcat(t, "ArrayOf");
    }
    else
    { t = (char*)malloc(strlen(s)+12);
      strcpy(t, "ArrayOf");
    }
    strcat(t, s);
    d = get_Darraydims(typ);
    if (d)
      sprintf(t+strlen(t), "%dD", d);
    return t;
  }
  switch(typ->type){
  case Tchar :
    if (ns)
      return "xsd:byte";
    else
      return "byte";
  case Twchar :
    if (ns)
      return "xsd:wchar";
    else
      return "wchar";
  case Tshort :
    if (ns)
      return "xsd:short";
    else
      return "short";
  case Tint  :
    if (ns)
      return "xsd:int";
    else
      return "int";
  case Tlong  :
  case Tllong  :
    if (ns)
      return "xsd:long";
    else
      return "long";
  case Tfloat:
    if (ns)
      return "xsd:float";
    else
      return "float";
  case Tdouble:
    if (ns)
      return "xsd:double";
    else
      return "double";
  case Tuchar:
    if (ns)
      return "xsd:unsignedByte";
    else
      return "unsignedByte";
  case Tushort:
    if (ns)
      return "xsd:unsignedShort";
    else
      return "unsignedShort";
  case Tuint:
    if (ns)
      return "xsd:unsignedInt";
    else
      return "unsignedInt";
  case Tulong:
  case Tullong:
    if (ns)
      return "xsd:unsignedLong";
    else
      return "unsignedLong";
  case Ttime:
    if (ns)
      return "xsd:dateTime";
    else
      return "dateTime";
  case Tpointer:
  case Treference:
    return wsdl_type(typ->ref, ns);
  case Tarray:
    if (ns && *ns)
    { s = (char*)malloc((strlen(ns_convert(ns))+strlen(c_ident(typ))+2)*sizeof(char));
      strcpy(s, ns_convert(ns));
      strcat(s, ":");
      strcat(s, c_ident(typ));
      return s;
    }
    else
      return c_ident(typ);
  case Tenum:
    if (typ->ref == booltable)
      if (ns)
        return "xsd:boolean";
      else
        return "boolean";
  case Tstruct:
  case Tclass:
    if (!has_ns(typ) && ns && *ns)
    { s = (char*)malloc((strlen(ns_convert(ns))+strlen(typ->id->name)+2)*sizeof(char));
      strcpy(s, ns_convert(ns));
      strcat(s, ":");
      strcat(s, ns_convert(typ->id->name));
      return s;
    }
    else if (ns)
      return ns_convert(typ->id->name);
    else
      return ns_remove(typ->id->name);
  }
  return "";
}

char *
the_type(Tnode *typ)
{ if (!typ)
    return "NULL";
  if (typ->type == Tarray || is_dynamic_array(typ) && !has_ns(typ) && !is_untyped(typ))
    return "SOAP-ENC:Array";
  if (is_string(typ) || is_wstring(typ))
    return "string";
  switch(typ->type){
  case Tchar :
    return "byte";
  case Twchar :
    return "wchar";
  case Tshort :
    return "short";
  case Tint  :
    return "int";
  case Tlong  :
  case Tllong  :
    return "long";
  case Tfloat:
    return "float";
  case Tdouble:
    return "double";
  case Tuchar:
    return "unsignedByte";
  case Tushort:
    return "unsignedShort";
  case Tuint:
    return "unsignedInt";
  case Tulong:
  case Tullong:
    return "unsignedLong";
  case Ttime:
    return "dateTime";
  case Tpointer:
  case Treference:
    return the_type(typ->ref);
  case Tarray:
    return "SOAP-ENC:Array";
  case Tenum:
    if (typ->ref == booltable)
      return "boolean";
  case Tstruct:
  case Tclass:
    return ns_convert(typ->id->name);
  }
  return "";
}

/* c_type returns the type to be used in parameter declaration*/
char *
c_type(Tnode *typ)
{
  char *p, tempBuf[10];
  Tnode *temp;
  
  if (typ==0)
    return "NULL";
  
  switch(typ->type){
  case Tnone :
    return "";
  case Tvoid :
    return "void";
  case Tchar :
    return "char";
  case Twchar :
    return "wchar_t";
  case Tshort :
    return "short";
  case Tint   :
    return "int";
  case Tlong   :
    return "long";
  case Tllong   :
    return "LONG64";
  case Tfloat :
    return "float";
  case Tdouble:
    return "double";
  case Tuchar :
    return "unsigned char";
  case Tushort:
    return "unsigned short";
  case Tuint :
    return "unsigned int";
  case Tulong:
    return "unsigned long";
  case Tullong:
    return "ULONG64";
  case Ttime:
    return "time_t";
  case Tstruct:p=(char*) malloc((8+strlen(typ->id->name)) *sizeof(char));
    strcpy(p,"struct ");
    strcat(p,typ->id->name);
    break;
  case Tclass:
   p = typ->id->name;
   break;
  case Tunion: p=(char*) malloc((7+strlen(typ->id->name)) *sizeof(char));
    strcpy(p,"union ");
    strcat(p,typ->id->name);
    break;
  case Tenum:
    if (typ->ref == booltable)
      return "bool";
    p=(char*) malloc((6+strlen(typ->id->name)) *sizeof(char));
    strcpy(p,"enum ");
    strcat(p,typ->id->name);
    break;
  case Tpointer:
    p = c_type_id(typ->ref, "*");
    break;
  case Treference:
    p = c_type_id(typ->ref, "&");
    break;
  case Tarray :p=(char*) malloc((12+strlen(c_type(typ->ref))) *sizeof(char));

    temp = typ;
    while(((Tnode*) (typ->ref))->type==Tarray){
      typ = typ->ref;
    }

    if (((Tnode*)typ->ref)->type == Tpointer)
      sprintf(p,"%s",c_type(typ->ref));
    else
      strcpy(p, c_type(typ->ref));
     
    typ = temp;
    
    while(typ->type==Tarray)
    { if (((Tnode*) typ->ref)->width)
      { sprintf(tempBuf,"[%d]",(typ->width / ((Tnode*) typ->ref)->width));
        strcat(p,tempBuf);
      }
      typ = typ->ref;
    }
    break;

  default :
    return "UnknownType";   
  }
  return p;
}

char *
c_storage(Storage sto)
{ char *p;
  static char buf[256];
  if (sto & Sconst)
  { p = c_storage(sto & ~Sconst);
    strcat(p, "const ");
    return p;
  }
  if (sto & Sauto)
  { p = c_storage(sto & ~Sauto);
    strcat(p, "auto ");
    return p;
  }
  if (sto & Sregister)
  { p = c_storage(sto & ~Sregister);
    strcat(p, "register ");
    return p;
  }
  if (sto & Sstatic)
  { p = c_storage(sto & ~Sstatic);
    strcat(p, "static ");
    return p;
  }
  if (sto & Sextern)
  { p = c_storage(sto & ~Sextern);
    return p;
  }
  if (sto & Stypedef)
  { p = c_storage(sto & ~Stypedef);
    strcat(p, "typedef ");
    return p;
  }
  if (sto & Svirtual)
  { p = c_storage(sto & ~Svirtual);
    strcat(p, "virtual ");
    return p;
  }
  if (sto & Sfriend)
  { p = c_storage(sto & ~Sfriend);
    strcat(p, "friend ");
    return p;
  }
  if (sto & Sinline)
  { p = c_storage(sto & ~Sinline);
    strcat(p, "inline ");
    return p;
  }
  buf[0]= '\0';
  return buf;
}

char *
c_init(Entry *e)
{ static char buf[1024];
  buf[0] = '\0';
  if (e->info.hasval)
    switch (e->info.typ->type)
    { case Tchar:
      case Twchar:
      case Tuchar:
      case Tshort:
      case Tushort:
      case Tint:
      case Tuint:
      case Tlong:
      case Tllong:
      case Tulong:
      case Tullong:
      case Ttime:
        sprintf(buf, " = "SOAP_LONG_FORMAT, e->info.val.i);
	break;
      case Tfloat:
      case Tdouble:
        sprintf(buf, " = %f", e->info.val.r);
	break;
      case Tenum:
        sprintf(buf, " = (%s)"SOAP_LONG_FORMAT, c_type(e->info.typ), e->info.val.i);
	break;
      default:
	if (e->info.val.s && strlen(e->info.val.s) < sizeof(buf)-6)
          sprintf(buf, " = \"%s\"", e->info.val.s);
	break;
    }
  return buf;
}

/* c_type_id returns the arraytype to be used in parameter declaration
   Allows you to specify the identifier that acts acts as teh name of teh
   type of array */
char *
c_type_id(Tnode *typ, char *ident)
{
  char *p,*q,tempBuf[10];
  Tnode *temp;
  Entry *e;

  if (!typ)
    return "NULL";
  
  switch(typ->type)
  {
  case Tnone:
    p = ident;
    break;
  case Tvoid :
    p = (char*)malloc(6+strlen(ident));
    strcpy(p, "void ");
    strcat(p, ident);
    break;
  case Tchar :
    p = (char*)malloc(6+strlen(ident));
    strcpy(p, "char ");
    strcat(p, ident);
    break;
  case Twchar :
    p = (char*)malloc(9+strlen(ident));
    strcpy(p, "wchar_t ");
    strcat(p, ident);
    break;
  case Tshort :
    p = (char*)malloc(7+strlen(ident));
    strcpy(p, "short ");
    strcat(p, ident);
    break;
  case Tint   :
    p = (char*)malloc(5+strlen(ident));
    strcpy(p, "int ");
    strcat(p, ident);
    break;
  case Tlong   :
    p = (char*)malloc(6+strlen(ident));
    strcpy(p, "long ");
    strcat(p, ident);
    break;
  case Tllong   :
    p = (char*)malloc(8+strlen(ident));
    strcpy(p, "LONG64 ");
    strcat(p, ident);
    break;
  case Tfloat :
    p = (char*)malloc(7+strlen(ident));
    strcpy(p, "float ");
    strcat(p, ident);
    break;
  case Tdouble:
    p = (char*)malloc(8+strlen(ident));
    strcpy(p, "double ");
    strcat(p, ident);
    break;
  case Tuchar :
    p = (char*)malloc(15+strlen(ident));
    strcpy(p, "unsigned char ");
    strcat(p, ident);
    break;
  case Tushort:
    p = (char*)malloc(16+strlen(ident));
    strcpy(p, "unsigned short ");
    strcat(p, ident);
    break;
  case Tuint :
    p = (char*)malloc(14+strlen(ident));
    strcpy(p, "unsigned int ");
    strcat(p, ident);
    break;
  case Tulong:
    p = (char*)malloc(15+strlen(ident));
    strcpy(p, "unsigned long ");
    strcat(p, ident);
    break;
  case Tullong:
    p = (char*)malloc(9+strlen(ident));
    strcpy(p, "ULONG64 ");
    strcat(p, ident);
    break;
  case Ttime:
    p = (char*)malloc(8+strlen(ident));
    strcpy(p, "time_t ");
    strcat(p, ident);
    break;
  case Tstruct:
    p=(char*) malloc((9+strlen(typ->id->name)+strlen(ident)) *sizeof(char));
    strcpy(p,"struct ");
    strcat(p,typ->id->name);
    strcat(p, " ");
    strcat(p,ident);
    break;
  case Tclass:
    p=(char*) malloc((2+strlen(typ->id->name)+strlen(ident)) *sizeof(char));
    strcpy(p, typ->id->name);
    strcat(p, " ");
    strcat(p, ident);
    break;
  case Tunion:
    p=(char*) malloc((8+strlen(typ->id->name)+strlen(ident)) *sizeof(char));
    strcpy(p,"union ");
    strcat(p,typ->id->name);
    strcat(p, " ");
    strcat(p, ident);
    break;
  case Tenum:
    if (typ->ref == booltable)
    { p = (char*)malloc((strlen(ident)+6)*sizeof(char));
      strcpy(p, "bool ");
      strcat(p, ident);
      return p;
    }
    p=(char*) malloc((7+strlen(typ->id->name)+strlen(ident)) *sizeof(char));
    strcpy(p, "enum ");
    strcat(p, typ->id->name);
    strcat(p, " ");
    strcat(p, ident);
    break;
  case Tpointer:
    p = (char*)malloc(strlen(ident)+2);
    strcpy(p+1, ident);
    p[0] = '*';
    p = c_type_id(typ->ref, p);
    break;
  case Treference:
    p = (char*)malloc(strlen(ident)+2);
    strcpy(p+1, ident);
    p[0] = '&';
    p = c_type_id(typ->ref, p);
    break;
  case Tarray :

    temp = typ;
    while(((Tnode*) (typ->ref))->type==Tarray){
      typ = typ->ref;
    }

    p=(char*) malloc((12+strlen(c_type_id(typ->ref, ident))) *sizeof(char));
    if (((Tnode*)typ->ref)->type == Tpointer)
      sprintf(p,"%s",c_type_id(typ->ref, ident));
    else
      strcpy(p, c_type_id(typ->ref, ident));
     
    typ = temp;
    
    while(typ->type==Tarray){
      if (((Tnode*) typ->ref)->width)
      { sprintf(tempBuf,"[%d]",(typ->width / ((Tnode*) typ->ref)->width));
        strcat(p,tempBuf);
      }
      typ = typ->ref;
    }

    
    /*if(((Tnode*) (typ->ref))->type==Tarray){
      sprintf(p,"%s [%d]",c_type(typ->ref),(typ->width / ((Tnode*) typ->ref)->width));
    }else
    sprintf(p,"%s a[%d]",c_type(typ->ref),(typ->width /((Tnode*) typ->ref)->width));*/
    break;

  case Tfun:
    if (strncmp(ident, "operator ", 9))
      q = c_type_id(((FNinfo*)typ->ref)->ret, ident);
    else
      q = ident;
    p = (char*)malloc(256*sizeof(char));
    strcpy(p, q);
    strcat(p, "(");
    for (e = ((FNinfo*)typ->ref)->args->list; e; e = e->next)
    { strcat(p, c_storage(e->info.sto));
      strcat(p, c_type_id(e->info.typ, e->sym->name));
      if (e->next)
        strcat(p, ", ");
    }
    strcat(p, ")");
    break;
  default : return "UnknownType";   
  }
  return p;
}

char *
xsi_type_Tarray(Tnode *typ)
{ Tnode *t;
  int cardinality;
  char *p, *s;
  t = typ->ref;
  cardinality = 1;
  while (t->type == Tarray || (is_dynamic_array(t) && !has_ns(t) && !is_untyped(typ)))
  { if( t->type == Tarray)
      t = t->ref;
    else
      t = ((Table*)t->ref)->list->info.typ->ref;
    cardinality++;
  }
  s = xsi_type(t);
  if (!*s)
    s = wsdl_type(t, "");
  p = (char*)malloc(strlen(s)+cardinality+3);
  strcpy(p, s);
  if (cardinality > 1)
  { strcat(p, "[");
    for (; cardinality > 2; cardinality--)
      strcat(p, ",");
    strcat(p, "]");
  }
  /*
  for (; cardinality; cardinality--)
  { t = typ;
    for (i = 1; i < cardinality; i++)
      t = t->ref;
    sprintf(temp,"[%d]",get_dimension(t));
    strcat(p, temp);
  }
  */
  return p;
}

char *
xsi_type_Darray(Tnode *typ)
{ Tnode *t;
  int cardinality;
  char *p, *s;
  if (!typ->ref)
    return "";
  t = ((Table*)typ->ref)->list->info.typ->ref;
  cardinality = 1;
  while (t->type == Tarray || (is_dynamic_array(t) && !has_ns(t) && !is_untyped(typ)))
  { if( t->type == Tarray)
      t = t->ref;
    else
      t = ((Table*)t->ref)->list->info.typ->ref;
    cardinality++;
  }
  s = xsi_type(t);
  if (!*s)
    s = wsdl_type(t, "");
  p = (char*)malloc(strlen(s)+cardinality*2+1);
  strcpy(p, s);
  if (cardinality > 1)
  { strcat(p, "[");
    for (; cardinality > 2; cardinality--)
      strcat(p, ",");
    strcat(p, "]");
  }
  return p;
}

void
out_generate(typ)
Tnode *typ;
{
	if (is_transient(typ) || typ->type == Twchar || is_XML(typ))
	  return;
	if (is_primitive_or_string(typ))	
	{	      typeNO++;
	              fprintf(fhead,"\n\n#ifndef SOAP_TYPE_%s",c_ident(typ));	
		      fprintf(fhead,"\n#define SOAP_TYPE_%s (%d)",c_ident(typ),typeNO);	
		      fprintf(fhead,"\n#endif");	
			defaults(typ);
			serialize(typ);
			mark(typ);
			soap_put(typ);
			soap_out(typ);
			soap_get(typ);
			soap_in(typ);
	  return;
	}
        switch(typ->type)
        {
	case Tenum:
        case Tpointer:
        case Tarray :
        case Tstruct:
	 case Tclass: typeNO++;
  			if (is_header_or_fault(typ))
			  fprintf(fout,"\n\n#ifndef WITH_NOGLOBAL");
		      fprintf(fhead,"\n\n#ifndef SOAP_TYPE_%s",c_ident(typ));	
		      fprintf(fhead,"\n#define SOAP_TYPE_%s (%d)",c_ident(typ),typeNO);	
		      fprintf(fhead,"\n#endif");	
                      serialize(typ);  
                      mark(typ);
		      defaults(typ);	
		      soap_put(typ);
		      soap_out(typ);
		      soap_get(typ);	
		      soap_in(typ);
		      if (typ->type == Tclass)
		        soap_instantiate_class(typ);
  			if (is_header_or_fault(typ))
    			  fprintf(fout,"\n\n#endif");
                      break;
              default:break;
         }
}

void
def_table(table,typ)  
Table *table;  
Tnode *typ; 
{ 
  int i;  
  Tnode *p; 
  for(i=0;i<TYPES;i++)
    for(p=Tptr[i];p!=(Tnode*) 0;p=p->next)
	out_generate(p);
}

         
int no_of_var(typ)
Tnode * typ;
{
  Entry *p;
  Table *t;
  int i=0;
  if(typ->type==Tstruct || typ->type==Tclass)
    {
      t=typ->ref;
      for (p = t->list; p != (Entry*) 0; p = p->next) {
	if(p->info.typ->type==Tpointer)
	  i++;
      }
    }
  if((((Tnode *)(typ->ref))->type==Tstruct) ||
     (((Tnode *)(typ->ref))->type==Tclass) )
    {
      t=((Tnode*)(typ->ref))->ref;
      for (p = t->list; p != (Entry*) 0; p = p->next) {
	if(p->info.typ->type==Tpointer)
	  i++;
      }
    }
  return i;
}      

void
in_defs(Table *table)
{
  int i;  
  Tnode *p;
  for (i=0;i<TYPES;i++)
  { for(p=Tptr[i];p!=(Tnode*) 0;p=p->next)
    { if (!is_transient(p) && p->type != Twchar && p->type != Tfun && p->type != Treference && !is_XML(p) && !is_header_or_fault(p))
      { fprintf(fout,"\n\t\tcase SOAP_TYPE_%s:\n\t\t\tsoap_in_%s(soap, NULL, NULL, \"%s\");\n\t\t\tbreak;",c_ident(p), c_ident(p), xsi_type(p));
      }
    }
  }
}

void
in_defs2(Table *table)
{ int i;
  Tnode *p;
  char *s;
  int a=0;
  for(i=0;i<TYPES;i++)
  { for(p=Tptr[i];p!=(Tnode*) 0;p=p->next)
    { if (!is_transient(p) && p->type != Twchar && p->type != Tfun && p->type != Tpointer && p->type != Treference && !is_XML(p) && !is_header_or_fault(p) || is_string(p) && !is_XML(p))
      { if(a==0) 
	  { s = xsi_type(p);
	      if (!*s)
	        s = wsdl_type(p,"");
	      if (*s)
	      { fprintf(fout,"\n\t\t\tsoap_in_%s(soap, NULL, NULL, \"%s\");",c_ident(p),s);
	        a=1;
	      }
	  }
	else
	  { s = xsi_type(p);
	      if (!*s)
	        s = wsdl_type(p, "");
	      if (*s)
	        fprintf(fout,"\n\t\t\tif (soap->error == SOAP_TYPE_MISMATCH && soap->level == 2)\n\t\t\t\tsoap_in_%s(soap, NULL, NULL, \"%s\");", c_ident(p),s);
	  }
      }
    }
  }
  fprintf(fout,"\n\t\t\tif (soap->error)\n\t\t\t{\tsoap->peeked = 1;\n\t\t\t\tsoap_ignore_element(soap);\n\t\t\t}");
}

void
out_defs(Table *table)
{ int i;  
  char *s;
  Tnode *p;
  for (i = 0; i < TYPES; i++)
  { for (p = Tptr[i]; p; p = p->next)
    { if (is_transient(p) || is_primclass(p) || is_XML(p) || is_header_or_fault(p))
        continue;
      s = xsi_type(p);
      if (!*s)
        s = wsdl_type(p, "");
      if (p->type == Tarray)
        fprintf(fout,"\n\t\t\t\tcase SOAP_TYPE_%s:\n\t\t\t\t\tsoap_out_%s(soap, \"id\", pp->id, (%s)pp->ptr, \"%s\");\n\t\t\t\t\tbreak;",c_ident(p), c_ident(p),c_type_id(p->ref, "(*)"), s);
      else if(p->type == Tclass && !is_external(p))
        fprintf(fout,"\n\t\t\t\tcase SOAP_TYPE_%s:\n\t\t\t\t\t((%s)pp->ptr)->soap_out(soap, \"id\", pp->id, \"%s\");\n\t\t\t\t\tbreak;",c_ident(p), c_type_id(p, "*"), s);
      else if (is_string(p))
        fprintf(fout,"\n\t\t\t\tcase SOAP_TYPE_%s:\n\t\t\t\t\tsoap_out_string(soap, \"id\", pp->id, (char**)&pp->ptr, \"%s\");\n\t\t\t\t\tbreak;",c_ident(p), s);
      else if (is_wstring(p))
        fprintf(fout,"\n\t\t\t\tcase SOAP_TYPE_%s:\n\t\t\t\t\tsoap_out_wstring(soap, \"%s\", pp->id, (wchar_t**)&pp->ptr, \"%s\");\n\t\t\t\t\tbreak;",c_ident(p), xml_tag(p), s);
      else if (p->type == Tpointer)
        fprintf(fout,"\n\t\t\t\tcase SOAP_TYPE_%s:\n\t\t\t\t\tsoap_out_%s(soap, \"id\", pp->id, (%s)pp->ptr, \"%s\");\n\t\t\t\t\tbreak;",c_ident(p), c_ident(p),c_type_id(p, "const*"), s);
      else if(p->type != Tnone && p->type != Twchar && !is_void(p) && p->type != Tfun && p->type != Treference && p->type != Tunion)
        fprintf(fout,"\n\t\t\t\tcase SOAP_TYPE_%s:\n\t\t\t\t\tsoap_out_%s(soap, \"id\", pp->id, (const %s)pp->ptr, \"%s\");\n\t\t\t\t\tbreak;",c_ident(p), c_ident(p),c_type_id(p, "*"), s);
    }
  }
}

void
in_attach(Table *table)
{ int i;  
  Tnode *p;
  for (i = 0; i < TYPES; i++)
    for (p = Tptr[i]; p; p = p->next)
      if (is_attachment(p))
        if (p->type == Tclass)
	  fprintf(fout,"\n\t\tcase SOAP_TYPE_%s:\n\t\t{\t%s a;\n\t\t\ta = (%s)soap_class_id_enter(soap, soap->dime_id, NULL, SOAP_TYPE_%s, NULL, NULL);\n\t\t\tif (a)\n\t\t\t{\ta->__ptr = (unsigned char*)soap->dime_ptr;\n\t\t\t\ta->__size = soap->dime_size;\n\t\t\t\ta->id = soap->dime_id;\n\t\t\t\ta->type = soap->dime_type;\n\t\t\t\ta->options = soap->dime_options;\n\t\t\t}\n\t\t\tbreak;\n\t\t}", c_ident(p), c_type_id(p, "*"), c_type_id(p, "*"), c_ident(p));
	else
	  fprintf(fout,"\n\t\tcase SOAP_TYPE_%s:\n\t\t{\t%s a;\n\t\t\ta = (%s)soap_id_enter(soap, soap->dime_id, NULL, SOAP_TYPE_%s, sizeof(%s), 0);\n\t\t\tif (a)\n\t\t\t{\ta->__ptr = (unsigned char*)soap->dime_ptr;\n\t\t\t\ta->__size = soap->dime_size;\n\t\t\t\ta->id = soap->dime_id;\n\t\t\t\ta->type = soap->dime_type;\n\t\t\t\ta->options = soap->dime_options;\n\t\t\t}\n\t\t\tbreak;\n\t\t}", c_ident(p), c_type_id(p, "*"), c_type_id(p, "*"), c_ident(p), c_type(p));
      else if (is_binary(p) && !is_transient(p))
        if (p->type == Tclass)
	  fprintf(fout,"\n\t\tcase SOAP_TYPE_%s:\n\t\t{\t%s a;\n\t\t\ta = (%s)soap_class_id_enter(soap, soap->dime_id, NULL, SOAP_TYPE_%s, NULL, NULL);\n\t\t\tif (a)\n\t\t\t{\ta->__ptr = (unsigned char*)soap->dime_ptr;\n\t\t\t\ta->__size = soap->dime_size;\n\t\t\t}\n\t\t\tbreak;\n\t\t}", c_ident(p), c_type_id(p, "*"), c_type_id(p, "*"), c_ident(p));
	else
	  fprintf(fout,"\n\t\tcase SOAP_TYPE_%s:\n\t\t{\t%s a;\n\t\t\ta = (%s)soap_id_enter(soap, soap->dime_id, NULL, SOAP_TYPE_%s, sizeof(%s), 0);\n\t\t\tif (a)\n\t\t\t{\ta->__ptr = (unsigned char*)soap->dime_ptr;\n\t\t\t\ta->__size = soap->dime_size;\n\t\t\t}\n\t\t\tbreak;\n\t\t}", c_ident(p), c_type_id(p, "*"), c_type_id(p, "*"), c_ident(p), c_type(p));
      else if (is_string(p) && !is_XML(p) && !is_transient(p))
	fprintf(fout,"\n\t\tcase SOAP_TYPE_%s:\n\t\t{\t%s a;\n\t\t\ta = (%s)soap_id_enter(soap, soap->dime_id, NULL, SOAP_TYPE_%s, sizeof(%s), 0);\n\t\t\tif (a)\n\t\t\t\t*a = soap->dime_ptr;\n\t\t\tbreak;\n\t\t}", c_ident(p), c_type_id(p, "*"), c_type_id(p, "*"), c_ident(p), c_type(p));
}

void
out_attach(Table *table)
{ int i;  
  char *s;
  Tnode *p;
  for (i = 0; i < TYPES; i++)
    for (p = Tptr[i]; p; p = p->next)
      if (is_attachment(p))
      { s = c_type_id(p, "*");
        fprintf(fout,"\n\t\t\t\tcase SOAP_TYPE_%s:\n\t\t\t\t\tsoap_putdime(soap, pp->id, ((%s)pp->ptr)->id, ((%s)pp->ptr)->type, ((%s)pp->ptr)->options, ((%s)pp->ptr)->__ptr, ((%s)pp->ptr)->__size);\n\t\t\t\t\tbreak;", c_ident(p), s, s, s, s, s);
      }
}

void
soap_instantiate_class(Tnode *typ)
{ Table *Tptr;
  Entry *Eptr;
  int derclass = 0;
  char *s;
  
  fprintf(fhead,"\n\n#define soap_new_%s(soap, n) soap_instantiate_%s(soap, n, NULL, NULL)", c_ident(typ), c_ident(typ));
  fprintf(fhead,"\n\n#define soap_delete_%s(soap, p) soap_delete(soap, p)", c_ident(typ));
  fprintf(fhead,"\n\nSOAP_FMAC1 %s * SOAP_FMAC2 soap_instantiate_%s(struct soap*, int, const char*, const char*);", c_type(typ), c_ident(typ));
  fprintf(fout,"\n\nSOAP_FMAC1 %s * SOAP_FMAC2 soap_instantiate_%s(struct soap *soap, int n, const char *type, const char *arrayType)", c_type(typ), c_ident(typ));
  fprintf(fout,"\n{");
  fprintf(fout, "\nDBGLOG(TEST, SOAP_MESSAGE(fdebug, \"soap_instantiate_%s(%%d, %%s, %%s)\\n\", n, type?type:\"\", arrayType?arrayType:\"\"));", c_ident(typ));
  fprintf(fout,"\n\tsoap->alloced = 1;");

  fprintf(fout,"\n\tstruct soap_clist *cp = (struct soap_clist*)malloc(sizeof(struct soap_clist));");
  fprintf(fout,"\n\tif (!cp)\n\t\treturn NULL;");
  for (Eptr = classtable->list; Eptr; Eptr = Eptr->next)
  {
    Tptr = ((Table *) Eptr->info.typ->ref);
    if(Tptr == ((Table *) typ->ref)){
      continue;
    }
    
    derclass = 0;
    while(Tptr)
    {
      if(Tptr == typ->ref){
	derclass = 1;
      }

      Tptr = Tptr->prev;
    }

    if(derclass == 1 && !is_transient(Eptr->info.typ)){
      if (is_dynamic_array(Eptr->info.typ) && !is_binary(Eptr->info.typ) && !has_ns(Eptr->info.typ) && !is_untyped(Eptr->info.typ))
        fprintf(fout,"\n\tif (arrayType && !soap_match_tag(soap, arrayType, \"%s\"))", xsi_type(Eptr->info.typ));
      else
        fprintf(fout,"\n\tif (type && !soap_match_tag(soap, type, \"%s\"))", the_type(Eptr->info.typ));
      fprintf(fout,"\n\t{\tcp->next = soap->clist;");
      fprintf(fout,"\n\t\tcp->type = SOAP_TYPE_%s;", c_ident(Eptr->info.typ));
      fprintf(fout,"\n\t\tcp->size = n;");
      fprintf(fout,"\n\t\tsoap->clist = cp;");
      fprintf(fout,"\n\t\tif (n < 0)");
      fprintf(fout,"\n\t\t{\tcp->ptr = (void*)new %s;", c_type(Eptr->info.typ));
      if ((s = has_soapref(Eptr->info.typ)))
        fprintf(fout,"\n\t\t\t((%s*)cp->ptr)->%s = soap;", c_type(Eptr->info.typ), s);
      fprintf(fout,"\n\t\t}\n\t\telse");
      fprintf(fout,"\n\t\t{\tcp->ptr = (void*)new %s[n];", c_type(Eptr->info.typ));
      if (s)
        fprintf(fout,"\n\t\t\tfor (int i = 0; i < n; i++)\n\t\t\t\t((%s*)cp->ptr)[i].%s = soap;", c_type(Eptr->info.typ), s);
      fprintf(fout,"\n\t\t}\n\t\treturn (%s*)cp->ptr;", c_type(Eptr->info.typ));
      fprintf(fout,"\n\t}");

      derclass = 0;
    }
  }

      fprintf(fout,"\n\tcp->next = soap->clist;");
      fprintf(fout,"\n\tcp->type = SOAP_TYPE_%s;", c_ident(typ));
      fprintf(fout,"\n\tcp->size = n; ");
      fprintf(fout,"\n\tsoap->clist = cp;");
      fprintf(fout,"\n\tif (n < 0)");
      fprintf(fout,"\n\t{\tcp->ptr = (void*)new %s;", c_type(typ));
      if ((s = has_soapref(typ)))
        fprintf(fout,"\n\t\t((%s*)cp->ptr)->%s = soap;", c_type(typ), s);
      fprintf(fout,"\n\t}\n\telse");
      fprintf(fout,"\n\t{\tcp->ptr = (void*)new %s[n];", c_type(typ));
      if (s)
        fprintf(fout,"\n\t\tfor (int i = 0; i < n; i++)\n\t\t\t((%s*)cp->ptr)[i].%s = soap;", c_type(typ), s);
      fprintf(fout,"\n\t}\n\treturn (%s*)cp->ptr;", c_type(typ));
  
  fprintf(fout,"\n}");
}

void
serialize(Tnode *typ) 
{ if(typ->type==Tarray){
    /* ARRAY */
    fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_serialize_%s(struct soap*, %s);", c_ident(typ), c_type_id(typ, "const"));
    fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_serialize_%s(struct soap *soap, %s)", c_ident(typ), c_type_id(typ, "const a"));
    fprintf(fout,"\n{\tif (!soap_reference(soap, a, SOAP_TYPE_%s))",c_ident(typ));
    fprintf(fout,"\n\t\tsoap_mark_%s(soap, a);\n}",c_ident(typ));
      fflush(fout);
    return;   
  }
  if (is_dynamic_array(typ))
    if (typ->type == Tclass)
    { fprintf(fout,"\n\nvoid %s::soap_serialize(struct soap *soap) const\n{",c_ident(typ));
      fprintf(fout,"\n\tthis->soap_mark(soap);\n}");
      fflush(fout);
      return;
    }
    else
    { fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_serialize_%s(struct soap*, %s);",c_ident(typ),c_type_id(typ, "const*")); 
      fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_serialize_%s(struct soap *soap, %s)\n{",c_ident(typ),c_type_id(typ, "const*a")); 
      fprintf(fout,"\n\tsoap_mark_%s(soap, a);\n}", c_ident(typ));
      fflush(fout);
      return;
    }
  if(typ->type==Tclass && !is_external(typ)){
    fprintf(fout,"\n\nvoid %s::soap_serialize(struct soap *soap) const\n{", typ->id->name);
    if (!is_primclass(typ))
      fprintf(fout,"\n\tif (!soap_reference(soap, this, SOAP_TYPE_%s))",c_ident(typ));
    fprintf(fout,"\n\t\tthis->soap_mark(soap);\n}",c_ident(typ));
      fflush(fout);
    return;
  }

  fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_serialize_%s(struct soap*, %s);",c_ident(typ),c_type_id(typ, "const*"));
  fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_serialize_%s(struct soap *soap, %s)\n{", c_ident(typ),c_type_id(typ, "const*a"));
  if (is_primitive(typ))
    fprintf(fout,"\n\tsoap_reference(soap, a, SOAP_TYPE_%s);\n}",c_ident(typ));
  else if (is_string(typ) || is_wstring(typ))
  { fprintf(fout,"\n\tsoap_reference(soap, *a, SOAP_TYPE_%s);\n}",c_ident(typ));
  }
  else
  { if (!is_primclass(typ))
      fprintf(fout,"\n\tif (!soap_reference(soap, a, SOAP_TYPE_%s))",c_ident(typ));
    fprintf(fout,"\n\t\tsoap_mark_%s(soap, a);\n}",c_ident(typ));
  }
}

int
get_dimension(Tnode *typ)
{ if (((Tnode*)typ->ref)->width)
    return typ->width / ((Tnode*) typ->ref)->width;
  return 0;
}


void
mark(Tnode *typ)
{ int d;
  Table *table,*t;
  Entry *p;
  Tnode* temp;
  int cardinality;

    if ((p = is_dynamic_array(typ)))
    { if (is_external(typ))
        return;
      if (typ->type == Tclass)
      { fprintf(fout,"\n\nvoid %s::soap_mark(struct soap *soap) const\n{",c_ident(typ));
        if (is_binary(typ))
	{ if (is_attachment(typ))
            fprintf(fout,"\n\tif (this->__ptr)\n\t{\tsoap_array_reference(soap, this, (struct soap_array*)&this->%s, this->__size, SOAP_TYPE_%s);\n\t\tif (this->id || this->type)\n\t\t\tsoap->mode |= SOAP_ENC_DIME;\n\t}\n}", p->sym->name, c_ident(typ));
          else
            fprintf(fout,"\n\tif (this->__ptr)\n\t\tsoap_array_reference(soap, this, (struct soap_array*)&this->%s, this->__size, SOAP_TYPE_%s);\n}", p->sym->name, c_ident(typ));
      fflush(fout);
      return;
	}
	else
	{
      fprintf(fout,"\n\tint i;");
      d = get_Darraydims(typ);
      if (d)
      { fprintf(fout,"\n\tif (this->%s && !soap_array_reference(soap, this, (struct soap_array*)&this->%s, soap_size(this->__size, %d), SOAP_TYPE_%s))", p->sym->name, p->sym->name, d, c_ident(typ));
        fprintf(fout,"\n\t\tfor (i = 0; i < soap_size(this->__size, %d); i++)", d);
      }
      else
      { fprintf(fout,"\n\tif (this->%s && !soap_array_reference(soap, this, (struct soap_array*)&this->%s, this->__size, SOAP_TYPE_%s))", p->sym->name, p->sym->name, c_ident(typ));
        fprintf(fout,"\n\t\tfor (i = 0; i < this->__size; i++)");
      }
      fprintf(fout,"\n\t\t{\tsoap_embedded(soap, this->%s + i, SOAP_TYPE_%s);", p->sym->name, c_ident(p->info.typ->ref));
      if (((Tnode*)p->info.typ->ref)->type == Tclass && !is_external(p->info.typ->ref))
        fprintf(fout,"\n\t\t\tthis->%s[i].soap_mark(soap);", p->sym->name);
      else if (!is_primitive(p->info.typ->ref))
        fprintf(fout,"\n\t\t\tsoap_mark_%s(soap, this->%s + i);", c_ident(p->info.typ->ref), p->sym->name);
      fprintf(fout,"\n\t\t}\n}");
      return;
      }
      }
      else
      { fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap*, %s);",c_ident(typ),c_type_id(typ, "const*")); 
        fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap *soap, %s)\n{",c_ident(typ),c_type_id(typ, "const*a")); 
        if (is_binary(typ))
	{ if (is_attachment(typ))
            fprintf(fout,"\n\tif (a->__ptr)\n\t{\tsoap_array_reference(soap, a, (struct soap_array*)&a->%s, a->__size, SOAP_TYPE_%s);\n\t\tif (a->id || a->type)\n\t\t\tsoap->mode |= SOAP_ENC_DIME;\n\t}\n}", p->sym->name, c_ident(typ));
          else
            fprintf(fout,"\n\tif (a->__ptr)\n\t\tsoap_array_reference(soap, a, (struct soap_array*)&a->%s, a->__size, SOAP_TYPE_%s);\n}", p->sym->name, c_ident(typ));
      fflush(fout);
      return;
	}
	else
	{
      fprintf(fout,"\n\tint i;");
      d = get_Darraydims(typ);
      if (d)
      { fprintf(fout,"\n\tif (a->%s && !soap_array_reference(soap, a, (struct soap_array*)&a->%s, soap_size(a->__size, %d), SOAP_TYPE_%s))", p->sym->name, p->sym->name, d, c_ident(typ));
        fprintf(fout,"\n\t\tfor (i = 0; i < soap_size(a->__size, %d); i++)", d);
      }
      else
      { fprintf(fout,"\n\tif (a->%s && !soap_array_reference(soap, a, (struct soap_array*)&a->%s, a->__size, SOAP_TYPE_%s))", p->sym->name, p->sym->name, c_ident(typ));
        fprintf(fout,"\n\t\tfor (i = 0; i < a->__size; i++)");
      }
      fprintf(fout,"\n\t\t{\tsoap_embedded(soap, a->%s + i, SOAP_TYPE_%s);", p->sym->name, c_ident(p->info.typ->ref));
      if (((Tnode*)p->info.typ->ref)->type == Tclass && !is_external(p->info.typ->ref))
        fprintf(fout,"\n\t\t\ta->%s[i].soap_mark(soap);", p->sym->name);
      else if (!is_primitive(p->info.typ->ref))
        fprintf(fout,"\n\t\t\tsoap_mark_%s(soap, a->%s + i);", c_ident(p->info.typ->ref), p->sym->name);
      fprintf(fout,"\n\t\t}\n}");
      fflush(fout);
      return;
      }
      }
      }
  switch(typ->type)
    {	
    case Tstruct:

      if (!typ->ref)
        return;
      fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap*, const %s);",c_ident(typ),c_type_id(typ, "*")); 
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap *soap, const %s)\n{",c_ident(typ),c_type_id(typ, "*a")); 
      /* DYNAMIC ARRAY */
      
      table=(Table*)typ->ref;
      for (t = table; t != (Table *) 0; t = t->prev) { 
	for (p = t->list; p != (Entry*) 0; p = p->next) {
	  if (p->info.sto & Sconst)
	    fprintf(fout, "\n\t/* const %s skipped */", p->sym->name);
	  else if (is_transient(p->info.typ))
	    fprintf(fout, "\n\t/* transient %s skipped */", p->sym->name);
	  else if (p->info.sto & Sattribute)
	    ;
	  else if (is_repetition(p))
	  { 
      fprintf(fout,"\n\tif (a->%s)", p->next->sym->name);
      fprintf(fout,"\n\t{\tint i;\n\t\tfor (i = 0; i < a->%s; i++)", p->sym->name);
      fprintf(fout,"\n\t\t{\tsoap_embedded(soap, a->%s + i, SOAP_TYPE_%s);", p->next->sym->name, c_ident(p->next->info.typ->ref));
      if (((Tnode*)p->next->info.typ->ref)->type == Tclass && !is_external(p->next->info.typ->ref))
        fprintf(fout,"\n\t\t\ta->%s[i].soap_mark(soap);", p->next->sym->name);
      else if (!is_primitive(p->next->info.typ->ref))
        fprintf(fout,"\n\t\t\tsoap_mark_%s(soap, a->%s + i);", c_ident(p->next->info.typ->ref), p->next->sym->name);
      fprintf(fout,"\n\t\t}\n\t}");
          p = p->next;
	  }
	  else if(p->info.typ->type==Tarray)
	    {
	      fprintf(fout,"\n\tsoap_embedded(soap, a->%s, SOAP_TYPE_%s);", p->sym->name,c_ident(p->info.typ));
	      fprintf(fout,"\n\tsoap_mark_%s(soap, a->%s);", c_ident(p->info.typ),p->sym->name);
	    }
	  else if(p->info.typ->type == Tclass && !is_external(p->info.typ))
	    {
	      /* CLASS ( within STRUCT ) */
	      fprintf(fout,"\n\tsoap_embedded(soap, &a->%s, SOAP_TYPE_%s);", p->sym->name,c_ident(p->info.typ));
	      fprintf(fout,"\n\ta->%s.soap_mark(soap);",p->sym->name);
	    }
	  else if (p->info.typ->type != Tfun && !is_void(p->info.typ) && !is_XML(p->info.typ))
	    {
	      fprintf(fout,"\n\tsoap_embedded(soap, &a->%s, SOAP_TYPE_%s);", p->sym->name,c_ident(p->info.typ));
	      if (!is_primitive(p->info.typ))
	        fprintf(fout,"\n\tsoap_mark_%s(soap, &a->%s);", c_ident(p->info.typ),p->sym->name);
	    }
	}
      }
      fprintf(fout,"\n}");	 
      break;
      
    case Tclass:
      /* CLASS */
      if (is_external(typ))
      { fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap*, const %s);",c_ident(typ),c_type_id(typ, "*")); 
        return;
      }
      table=(Table*)typ->ref;
      /*      fprintf(fhead,"\nvirtual void %s::soap_mark();",
	      ((Table *) typ->ref)->sym->name ,
	      c_ident(typ),c_type(typ) ); */
      fprintf(fout,"\n\nvoid %s::soap_mark(struct soap *soap) const\n{", typ->id->name); 
      for (t = table; t != (Table *) 0; t = t->prev) { 
	for (p = t->list; p != (Entry*) 0; p = p->next) {
	  if (p->info.sto & Sconst)
	    fprintf(fout, "\n\t/* const %s skipped */", p->sym->name);
	  else if (is_transient(p->info.typ))
	    fprintf(fout, "\n\t/* transient %s skipped */", p->sym->name);
	  else if (p->info.sto & Sattribute)
	    ;
	  else if (is_repetition(p))
	  { 
      fprintf(fout,"\n\tif (((%s*)this)->%s)", t->sym->name, p->next->sym->name);
      fprintf(fout,"\n\t{\tint i;\n\t\tfor (i = 0; i < ((%s*)this)->%s; i++)", t->sym->name, p->sym->name);
      fprintf(fout,"\n\t\t{\tsoap_embedded(soap, ((%s*)this)->%s + i, SOAP_TYPE_%s);", t->sym->name, p->next->sym->name, c_ident(p->next->info.typ->ref));
      if (((Tnode*)p->next->info.typ->ref)->type == Tclass && !is_external(p->next->info.typ->ref))
        fprintf(fout,"\n\t\t\t((%s*)this)->%s[i].soap_mark(soap);", t->sym->name, p->next->sym->name);
      else if (!is_primitive(p->next->info.typ->ref))
        fprintf(fout,"\n\t\t\tsoap_mark_%s(soap, ((%s*)this)->%s + i);", c_ident(p->next->info.typ->ref), t->sym->name, p->next->sym->name);
      fprintf(fout,"\n\t\t}\n\t}");
          p = p->next;
	  }
	  else if(p->info.typ->type==Tarray)
	    {
	      /* ARRAY */
	      fprintf(fout,"\n\tsoap_embedded(soap, ((%s*)this)->%s, SOAP_TYPE_%s);", t->sym->name, p->sym->name,c_ident(p->info.typ));
	      fprintf(fout,"\n\tsoap_mark_%s(soap, ((%s*)this)->%s);", c_ident(p->info.typ),t->sym->name, p->sym->name);
	    }
	  else if(p->info.typ->type==Tclass && !is_external(p->info.typ))
	    {
	      /* CLASS ( within CLASS ) */
	      fprintf(fout,"\n\tsoap_embedded(soap, &((%s*)this)->%s, SOAP_TYPE_%s);", t->sym->name, p->sym->name, c_ident(p->info.typ));
	      fprintf(fout,"\n\t((%s*)this)->%s.soap_mark(soap);", t->sym->name, p->sym->name );
	    }
	  else if (p->info.typ->type != Tfun && !is_void(p->info.typ) && !is_XML(p->info.typ)) {
	    fprintf(fout,"\n\tsoap_embedded(soap, &((%s*)this)->%s, SOAP_TYPE_%s);", t->sym->name,p->sym->name,c_ident(p->info.typ));
	    if (!is_primitive(p->info.typ))
	      fprintf(fout,"\n\tsoap_mark_%s(soap, &((%s*)this)->%s);", c_ident(p->info.typ),t->sym->name,p->sym->name);
	  }
	}
      }
      fprintf(fout,"\n}");	 
      break;
    case Tpointer:
      if(is_string(typ) || is_wstring(typ)) 
	{ fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap*, %s);", c_ident(typ),c_type_id(typ, "const*"));
          if (is_external(typ))
            return;
	  fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap *soap, %s)\n{", c_ident(typ),c_type_id(typ, "const*a"));
	  fprintf(fout,"\n\tsoap_reference(soap, *a, SOAP_TYPE_%s);\n}", c_ident(typ));
	  return;
	}

      if (((Tnode*)typ->ref)->type == Tclass && !is_external(typ->ref))
      { fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap*, %s);", c_ident(typ),c_type_id(typ, "const*"));
        if (is_external(typ))
          return;
	fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap *soap, %s)\n{", c_ident(typ),c_type_id(typ, "const*a"));
	if (p = is_dynamic_array(typ->ref))
        { d = get_Darraydims(typ->ref);
          if (d)
	    fprintf(fout,"\n\tif (*a && !soap_array_reference(soap, *a, (struct soap_array*)&(*a)->%s, soap_size((*a)->__size, d), SOAP_TYPE_%s))", p->sym->name, d, c_ident(typ->ref));
	  else
	    fprintf(fout,"\n\tif (*a && !soap_array_reference(soap, *a, (struct soap_array*)&(*a)->%s, (*a)->__size, SOAP_TYPE_%s))", p->sym->name, c_ident(typ->ref));
	}
	else
	  fprintf(fout,"\n\tif (!soap_reference(soap, *a, SOAP_TYPE_%s))", c_ident(typ->ref));
	fprintf(fout,"\n\t\t(*a)->soap_mark(soap);\n}");
	break;	
      }
      else
      {
	fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap*, %s);", c_ident(typ),c_type_id(typ, "const*"));
      if (is_external(typ))
        return;
	
	fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap *soap, %s)\n{", c_ident(typ),c_type_id(typ, "const*a"));
	if (is_primitive(typ->ref))
	  fprintf(fout,"\n\tsoap_reference(soap, *a, SOAP_TYPE_%s);\n}", c_ident(typ->ref));
	else if (p = is_dynamic_array(typ->ref))
        { d = get_Darraydims(typ->ref);
          if (d)
	    fprintf(fout,"\n\tif (*a && !soap_array_reference(soap, *a, (struct soap_array*)&(*a)->%s, soap_size((*a)->__size, %d), SOAP_TYPE_%s))", p->sym->name, d, c_ident(typ->ref));
	  else
	    fprintf(fout,"\n\tif (*a && !soap_array_reference(soap, *a, (struct soap_array*)&(*a)->%s, (*a)->__size, SOAP_TYPE_%s))", p->sym->name, c_ident(typ->ref));
	  fprintf(fout,"\n\t\tsoap_mark_%s(soap, *a);\n}", c_ident(typ->ref));
	}
	else
	{ fprintf(fout,"\n\tif (!soap_reference(soap, *a, SOAP_TYPE_%s))", c_ident(typ->ref));
	  fprintf(fout,"\n\t\tsoap_mark_%s(soap, *a);\n}", c_ident(typ->ref));
	}
	break;
      }
	
    case Tarray :
      fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap*, %s);", c_ident(typ),c_type_id(typ, "const"));
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_mark_%s(struct soap *soap, %s)", c_ident(typ),c_type_id(typ, "const a"));
      if (is_primitive(typ->ref))
        fprintf(fout, "\n{");
      else
      { fprintf(fout,"\n{\tint i;");
        fprintf(fout,"\n\tfor(i = 0; i < %d; i++)", get_dimension(typ));
	
        temp=typ->ref;;
        cardinality = 1;
        while(temp->type==Tarray)
	{
	  temp=temp->ref;
	  cardinality++;
	}
        fprintf(fout,"\n\t{\tsoap_embedded(soap, a", c_ident(typ->ref));
        if(cardinality > 1){
	  fprintf(fout,"[i]");
        }else {
	  fprintf(fout,"+i");
        }
      
        fprintf(fout,", SOAP_TYPE_%s);",c_ident(typ->ref));

	if (((Tnode *)typ->ref)->type == Tclass && !is_external(typ->ref))
      	{	fprintf(fout,"\n\ta[i].soap_mark(soap)");
	}
	else if (!is_primitive(typ->ref))
      	{	fprintf(fout,"\n\tsoap_mark_%s(soap, a",c_ident(typ->ref));
      		if(cardinality > 1){
		fprintf(fout,"[i])");
      		}else {
	  	fprintf(fout,"+i)");
      		}
	}
        fprintf(fout,";\n\t}");
      }
      fprintf(fout,"\n}");
      break;	
    default:     break;
    }
}

void
defaults(typ)
Tnode* typ;
{ int i, d;
  Table *table,*t;
  Entry *p;
  Tnode *temp;
  char *s;
  int cardinality;
  if (p = is_dynamic_array(typ))
  {   if (is_external(typ))
        return;
    if (typ->type == Tclass)
      { fprintf(fout,"\n\nvoid %s::soap_default(struct soap *soap)\n{", c_ident(typ)); 
        fprintf(fout,"\n\tstatic %s a;\n\tmemcpy(this, &a, sizeof(%s));", c_ident(typ), c_ident(typ));
        if ((s = has_soapref(typ)))
          fprintf(fout,"\n\tthis->%s = soap;", s);
	d = get_Darraydims(typ);
        if (d)
	{ fprintf(fout,"\n\tthis->%s = NULL;", p->sym->name);
	  for (i = 0; i < d; i++)
	  { fprintf(fout,"\n\tthis->__size[%d] = 0;", i);
            if (has_offset(typ) && (((Table*)typ->ref)->list->next->next->info.sto & Sconst) == 0)
              fprintf(fout, "\n\tthis->__offset[%d] = 0;", i);
	  }
	}
	else
	{ fprintf(fout,"\n\tthis->__size = 0;\n\tthis->%s = NULL;", p->sym->name);
          if (has_offset(typ) && (((Table*)typ->ref)->list->next->next->info.sto & Sconst) == 0)
            fprintf(fout, "\n\tthis->__offset = 0;");
	}
	if (is_attachment(typ))
          fprintf(fout,"\n\tthis->id = NULL;\n\tthis->type = NULL;\n\tthis->options = NULL;");
        fprintf(fout,"\n}");
      }
      else
      { fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap*, %s);",c_ident(typ),c_type_id(typ, "*")); 
        fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap *soap, %s)\n{", c_ident(typ),c_type_id(typ, "*a")); 
	d = get_Darraydims(typ);
        if (d)
	{ fprintf(fout,"\n\ta->%s = NULL;", p->sym->name);
	  for (i = 0; i < d; i++)
	  { fprintf(fout,"\n\ta->__size[%d] = 0;", i);
            if (has_offset(typ) && (((Table*)typ->ref)->list->next->next->info.sto & Sconst) == 0)
              fprintf(fout, "\n\ta->__offset[%d] = 0;", i);
	  }
	}
	else
	{ fprintf(fout,"\n\ta->__size = 0;\n\ta->%s = NULL;", p->sym->name);
          if (has_offset(typ) && (((Table*)typ->ref)->list->next->next->info.sto & Sconst) == 0)
            fprintf(fout, "\n\ta->__offset = 0;");
	}
	if (is_attachment(typ))
          fprintf(fout,"\n\ta->id = NULL;\n\ta->type = NULL;\n\ta->options = NULL;");
        fprintf(fout,"\n}");
      }
      fflush(fout);
      return;
  }
  if (is_primitive(typ) || is_string(typ) && !is_XML(typ))
  {   fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap*, %s);",c_ident(typ),c_type_id(typ, "*"));
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap *soap, %s)\n{\n#ifdef SOAP_DEFAULT_%s\n\t*a = SOAP_DEFAULT_%s;\n#else\n\t*a = (%s)0;\n#endif\n}",c_ident(typ),c_type_id(typ, "*a"), c_ident(typ), c_ident(typ), c_type(typ));
      return;
  }
  switch(typ->type)
    {
    case Tclass:
      /* CLASS */
      if (is_external(typ))
      { fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap*, %s);",c_ident(typ),c_type_id(typ, "*")); 
        return;
      }
      table=(Table*)typ->ref;
      fprintf(fout,"\n\nvoid %s::soap_default(struct soap *soap)\n{", typ->id->name ); 
      fprintf(fout,"\n\tstatic %s a;\n\tmemcpy(this, &a, sizeof(%s));", typ->id->name, typ->id->name);
      if ((s = has_soapref(typ)))
        fprintf(fout,"\n\tthis->%s = soap;", s);
	
      /*      fprintf(fhead,"\nvirtual void %s::soap_default();",
	     ((Table *) typ->ref)->sym->name ); */
      fflush(fout);
      for (t = table; t != (Table *) 0; t = t->prev) { 
	if(t==(Table *) 0 ) return;
	for (p = t->list; p != (Entry*) 0; p = p->next)
	  if (p->info.sto & Sconst)
	    fprintf(fout, "\n\t/* const %s skipped */", p->sym->name);
	  else if (is_transient(p->info.typ))
	    fprintf(fout, "\n\t/* transient %s skipped */", p->sym->name);
	  else if (is_repetition(p))
	  { fprintf(fout, "\n\t((%s*)this)->%s = 0;\n\t((%s*)this)->%s = NULL;", t->sym->name, p->sym->name, t->sym->name, p->next->sym->name);
	    p = p->next;
	  }
	  else
	  {
	  if(p->info.typ->type==Tarray){
	    fprintf(fout,"\n\tsoap_default_%s(soap, ((%s*)this)->%s);", c_ident(p->info.typ),t->sym->name,p->sym->name);
	  }
	  else if(p->info.typ->type==Tclass && !is_external(p->info.typ))
	    /* CLASS (within CLASS) */
	    fprintf(fout,"\n\t((%s*)this)->%s.%s::soap_default(soap);",t->sym->name, p->sym->name, c_ident(p->info.typ));
	  else if (p->info.hasval)
	    fprintf(fout,"\n\t((%s*)this)->%s%s;", t->sym->name,p->sym->name,c_init(p));
	  else if (p->info.typ->type != Tfun && !is_void(p->info.typ) && !is_XML(p->info.typ)) {
	    fprintf(fout,"\n\tsoap_default_%s(soap, &((%s*)this)->%s);", c_ident(p->info.typ),t->sym->name,p->sym->name);
	  }
	}
      }
      fprintf(fout,"\n}");	 
      fflush(fout);
      break;
      
    case Tstruct:
      table=(Table*)typ->ref;

      fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap*, %s);",c_ident(typ),c_type_id(typ, "*")); 
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap *soap, %s)\n{", c_ident(typ),c_type_id(typ, "*a")); 
      fflush(fout);
      /* DYNAMIC ARRAY */

      for (t = table; t != (Table *) 0; t = t->prev) { 
	if(t==(Table *) 0 ) return;
	for (p = t->list; p != (Entry*) 0; p = p->next)
	  if (p->info.sto & Sconst)
	    fprintf(fout, "\n\t/* const %s skipped */", p->sym->name);
	  else if (is_transient(p->info.typ))
	    fprintf(fout, "\n\t/* transient %s skipped */", p->sym->name);
	  else if (is_repetition(p))
	  { fprintf(fout, "\n\ta->%s = 0;\n\ta->%s = NULL;", p->sym->name, p->next->sym->name);
	    p = p->next;
	  }
	  else
	  {
	  if(p->info.typ->type==Tarray)
	    fprintf(fout,"\n\tsoap_default_%s(soap, a->%s);",
		    c_ident(p->info.typ),p->sym->name);
	  else if(p->info.typ->type==Tclass && !is_external(p->info.typ)){
	    /* CLASS (within STRUCT) */
	    fprintf(fout,"\n\ta->%s.%s::soap_default(soap);",p->sym->name, c_ident(p->info.typ));
	  }
	  else if (p->info.hasval)
	    fprintf(fout,"\n\ta->%s%s;", p->sym->name,c_init(p));
	  else if (p->info.typ->type != Tfun && !is_void(p->info.typ) && !is_XML(p->info.typ))
	    fprintf(fout,"\n\tsoap_default_%s(soap, &a->%s);", c_ident(p->info.typ),p->sym->name);
	}
      }
      fprintf(fout,"\n}");	 
      fflush(fout);
      break;
      
    case Tarray:
      fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap*, %s);",c_ident(typ),c_type(typ));
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap *soap, %s)\n{", c_ident(typ),c_type_id(typ, "a"));
      fprintf(fout,"\n\tint i;");
      fprintf(fout,"\n\tfor (i = 0; i < %d; i++)",get_dimension(typ));
      temp = typ->ref;
      cardinality = 1;
      while(temp->type==Tarray)
	{
	  temp=temp->ref;
	  cardinality++;
	}
	if (((Tnode *)typ->ref)->type == Tclass && !is_external(typ->ref))
	{
      	if(cardinality>1){
		fprintf(fout,"a[i].%s::soap_default(soap)", c_ident(typ->ref));
     	 }else {
		fprintf(fout,"(a+i)->soap_default(soap)");
      		}
	}
      else
      {
      	fprintf(fout,"\n\tsoap_default_%s(soap, a",c_ident(typ->ref));
      	if(cardinality>1){
		fprintf(fout,"[i])");
     	 }else {
		fprintf(fout,"+i)");
      	}
      }
      fprintf(fout,";\n}");
      break;	
      
    case Tpointer:
      
      fprintf(fhead,"\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap*, %s);",c_ident(typ),c_type_id(typ, "*"));
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 void SOAP_FMAC2 soap_default_%s(struct soap *soap, %s)\n{\n\t*a = NULL;\n}",c_ident(typ),c_type_id(typ, "*a"));
      break;
      
    default    :break;
    }
  
}

void
soap_put(Tnode *typ)
{ int d;
  Entry *p;
  char *ci = c_ident(typ);
  char *ct = c_type(typ);
  char *cta = c_type_id(typ, "a");
  char *ctp = c_type_id(typ, "*");
  char *ctpa = c_type_id(typ, "*a");
  char *xt = xsi_type(typ);
  if(typ->type==Tarray)
    {
      fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_put_%s(struct soap*, %s, const char*, const char*);", ci,ct);
      fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_put_%s(struct soap *soap, %s, const char *tag, const char *type)\n{", ci,cta);
    }
  else if(typ->type==Tclass && !is_external(typ))
    {
      fprintf(fout,"\n\nint %s::soap_put(struct soap *soap, const char *tag, const  char *type) const\n{", ct);
    }
  else
    {
      fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_put_%s(struct soap*, %s, const char*, const char*);", ci,ctp);
      fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_put_%s(struct soap *soap, %s, const char *tag, const char *type)\n{", ci,ctpa);
    }
  
  fflush(fout);
  fprintf(fout,"\n\tint i;");
  fprintf(fout,"\n\tstruct soap_plist *pp;");
  if (p = is_dynamic_array(typ))
  { d = get_Darraydims(typ);
    if (typ->type == Tclass)
      if (d)
        fprintf(fout,"\n\tif ((i = soap_array_pointer_lookup(soap, this, (struct soap_array*)&this->%s, soap_size(this->__size, %d), SOAP_TYPE_%s, &pp)))", p->sym->name, d, ci);
      else
        fprintf(fout,"\n\tif ((i = soap_array_pointer_lookup(soap, this, (struct soap_array*)&this->%s, this->__size, SOAP_TYPE_%s, &pp)))", p->sym->name, ci);
    else if (d)
      fprintf(fout,"\n\tif ((i = soap_array_pointer_lookup(soap, a, (struct soap_array*)&a->%s, soap_size(a->__size, %d), SOAP_TYPE_%s, &pp)))", p->sym->name, d, ci);
    else
      fprintf(fout,"\n\tif ((i = soap_array_pointer_lookup(soap, a, (struct soap_array*)&a->%s, a->__size, SOAP_TYPE_%s, &pp)))", p->sym->name, ci);
  }
  else if (typ->type == Tclass && !is_external(typ))
    fprintf(fout,"\n\tif ((i = soap_pointer_lookup(soap, this, SOAP_TYPE_%s, &pp)))", ci);
  else
    fprintf(fout,"\n\tif ((i = soap_pointer_lookup(soap, a, SOAP_TYPE_%s, &pp)))", ci);
  fprintf(fout,"\n\t\tif (soap_is_embedded(soap, pp))");
  fprintf(fout,"\n\t\t\tsoap_element_ref(soap, tag, 0, i);");
  fprintf(fout,"\n\t\telse if (soap_is_single(soap, pp))");
  fflush(fout);
 
  if(typ->type == Tarray){
    /* ARRAY */
    fprintf(fout,"\n\t\t\tsoap_out_%s(soap, tag, 0, a, type);", ci);
  }
  else if(typ->type == Tclass && !is_external(typ)){
    /* CLASS */
    fprintf(fout,"\n\t\t\tthis->soap_out(soap, tag, 0, type);");
  }
  else{
    fprintf(fout,"\n\t\t\tsoap_out_%s(soap, tag, 0, a, type);", ci);
  }
  
  fprintf(fout,"\n\t\telse\n\t\t{");
  fprintf(fout,"\n\t\t\tsoap_set_embedded(soap, pp);");   
  fflush(fout);
  if(typ->type  == Tarray){
    /* ARRAY */
    fprintf(fout,"\n\t\t\tsoap_out_%s(soap, tag, i, a, type);\n\t\t}", ci);
  }
  else if(typ->type == Tclass && !is_external(typ)){
    /* CLASS */
    fprintf(fout,"\n\t\t\tthis->soap_out(soap, tag, i, type);\n\t\t}");
  }
  else{
    fprintf(fout,"\n\t\t\tsoap_out_%s(soap, tag, i, a, type);\n\t\t}", ci);
  }
  fflush(fout);
  
  if(typ->type == Tarray){
    /* ARRAY */
    fprintf(fout,"\n\telse\n\t\tsoap_out_%s(soap, tag, 0, a, type);", ci);
   
  }
  else if(typ->type == Tclass && !is_external(typ)){
    /* CLASS */
    fprintf(fout,"\n\telse\n\t\tthis->soap_out(soap, tag, 0, type);");
  }
  else{
    fprintf(fout,"\n\telse\n\t\tsoap_out_%s(soap, tag, 0, a, type);", ci);
  }
  fprintf(fout,"\n\treturn soap_putindependent(soap);\n}");
  fflush(fout);
}

Entry *
is_dynamic_array(Tnode *typ)
{ Entry *p;
  Table *t;
  if ((typ->type == Tstruct || typ->type == Tclass) && typ->ref)
  { for (t = (Table*)typ->ref; t; t = t->prev)
    { p = t->list;
      if (p && p->info.typ->type == Tpointer && !strncmp(p->sym->name, "__ptr", 5))
        if (p->next && (p->next->info.typ->type == Tint || p->next->info.typ->type == Tuint || p->next->info.typ->type == Tarray && (((Tnode*)p->next->info.typ->ref)->type == Tint || ((Tnode*)p->next->info.typ->ref)->type == Tuint)) && !strcmp(p->next->sym->name, "__size"))
	  return p;
    }
  }
  return 0;
}

int
get_Darraydims(Tnode *typ)
{ Entry *p;
  Table *t;
  if ((typ->type == Tstruct || typ->type == Tclass) && typ->ref)
  { for (t = (Table*)typ->ref; t; t = t->prev)
    { p = t->list;
      if (p && p->info.typ->type == Tpointer && !strncmp(p->sym->name, "__ptr", 5))
        if (p->next && p->next->info.typ->type == Tarray && (((Tnode*)p->next->info.typ->ref)->type == Tint || ((Tnode*)p->next->info.typ->ref)->type == Tuint) && !strcmp(p->next->sym->name, "__size"))
          return get_dimension(p->next->info.typ);
    }
  }
  return 0;
}

int
has_offset(Tnode *typ)
{ Entry *p;
  Table *t;
  if (typ->type == Tstruct || typ->type == Tclass)
  { for (t = (Table*)typ->ref; t; t = t->prev)
    { for (p = t->list; p; p = p->next)
      { if ((p->info.typ->type == Tint || p->info.typ->type == Tarray && ((Tnode*)p->info.typ->ref)->type == Tint) && !strcmp(p->sym->name, "__offset"))
          return 1;
      }
    }
  }
  return 0;
}

int
is_hexBinary(Tnode *typ)
{ Entry *p;
  Table *t;
  if ((typ->type == Tstruct || typ->type == Tclass) && !strcmp(typ->id->name, "xsd__hexBinary"))
  { for (t = (Table*)typ->ref; t; t = t->prev)
    { p = t->list;
      if (p && p->info.typ->type == Tpointer && ((Tnode*)p->info.typ->ref)->type == Tuchar && !strcmp(p->sym->name, "__ptr"))
      { p = p->next;
        return p && (p->info.typ->type == Tint || p->info.typ->type == Tuint) && !strcmp(p->sym->name, "__size");
      }
    }
  }
  return 0;
}

int
is_binary(Tnode *typ)
{ Entry *p;
  Table *t;
  if (typ->type == Tstruct || typ->type == Tclass) 
  { for (t = (Table*)typ->ref; t; t = t->prev)
    { p = t->list;
      if (p && p->info.typ->type == Tpointer && ((Tnode*)p->info.typ->ref)->type == Tuchar && !strcmp(p->sym->name, "__ptr"))
      { p = p->next;
        return p && (p->info.typ->type == Tint || p->info.typ->type == Tuint) && !strcmp(p->sym->name, "__size");
      }
    }
  }
  return 0;
}

is_attachment(Tnode *typ)
{ Entry *p;
  Table *t;
  if (!is_binary(typ) || is_transient(typ))
    return 0;
  for (t = (Table*)typ->ref; t; t = t->prev)
  { for (p = t->list; p; p = p->next)
    { if (is_string(p->info.typ) && !strcmp(p->sym->name, "id"))
      { p = p->next;
        if (!p || !is_string(p->info.typ) || strcmp(p->sym->name, "type"))
          break;
        p = p->next;
        if (!p || !is_string(p->info.typ) || strcmp(p->sym->name, "options"))
          break;
        return 1;
      }
    }
  }
  return 0;
}

int
is_header_or_fault(Tnode *typ)
{ if (typ->type == Tpointer || typ->type == Treference)
    return is_header_or_fault(typ->ref);
  return (typ->type == Tstruct || typ->type == Tclass) && (!strcmp(typ->id->name, "SOAP_ENV__Header") || !strcmp(typ->id->name, "SOAP_ENV__Fault") || !strcmp(typ->id->name, "SOAP_ENV__Code"));
}

char *
has_soapref(Tnode *typ)
{ Entry *p;
  Table *t;
  if (typ->type == Tstruct || typ->type == Tclass) 
  { for (t = (Table*)typ->ref; t; t = t->prev)
    { for (p = t->list; p; p = p->next)
        if (p->info.typ->type == Tpointer && ((Tnode*)p->info.typ->ref)->type == Tstruct && ((Tnode*)p->info.typ->ref)->id == lookup("soap"))
          return p->sym->name;
    }
  }
  return 0;
}

int
is_primitive_or_string(Tnode *typ)
{ return is_primitive(typ) || is_string(typ) || is_wstring(typ); 
}

int
is_primitive(Tnode *typ)
{ return typ->type <= Ttime; 
}

int
is_string(Tnode *typ)
{ return typ->type == Tpointer && ((Tnode*)typ->ref)->type == Tchar;
}

int
is_wstring(Tnode *typ)
{ return typ->type == Tpointer && ((Tnode*)typ->ref)->type == Twchar;
}

int
reflevel(Tnode *typ)
{ int level;
  for (level = 0; typ->type == Tpointer; level++)
    typ = (Tnode*)typ->ref;
  return level;
}

void
soap_out(Tnode *typ)
{ Table *table,*t;
  Entry *p;
  int cardinality,i,j,d;
  Tnode *n;
  if (is_dynamic_array(typ))
  { soap_out_Darray(typ);
    return;
  }
  if (is_primitive(typ) && typ->type != Tenum)
  {
    fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char*, int, const %s, const char*);", c_ident(typ),c_type_id(typ, "*")); 
      if (is_external(typ))
        return;
    fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, const %s, const char *type)\n{", c_ident(typ),c_type_id(typ, "*a")); 
    if (typ->type == Tllong || typ->type == Tullong)
      fprintf(fout,"\n\treturn soap_out%s(soap, tag, id, a, type, SOAP_TYPE_%s);\n}", c_type(typ),c_ident(typ)); 
    else
      fprintf(fout,"\n\treturn soap_out%s(soap, tag, id, a, type, SOAP_TYPE_%s);\n}", the_type(typ),c_ident(typ)); 
    return;
  } else if (is_string(typ))
  {
    fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char*, int, char*const*, const char*);", c_ident(typ));
      if (is_external(typ))
        return;
    fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, char *const*a, const char *type)\n{", c_ident(typ));
    fprintf(fout,"\n\treturn soap_outstring(soap, tag, id, a, type, SOAP_TYPE_%s);\n}", c_ident(typ)); 
    return;
  } else if (is_wstring(typ))
  {
    fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char*, int, wchar_t*const*, const char*);", c_ident(typ));
      if (is_external(typ))
        return;
    fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, wchar_t *const*a, const char *type)\n{", c_ident(typ));
    fprintf(fout,"\n\treturn soap_outwstring(soap, tag, id, a, type, SOAP_TYPE_%s);\n}", c_ident(typ)); 
    return;
  }
  switch(typ->type)
  { case Tstruct:
      table=(Table*)typ->ref;
      fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char*, int, const %s, const char*);", c_ident(typ),c_type_id(typ, "*")); 
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, const %s, const char *type)\n{", c_ident(typ),c_type_id(typ, "*a")); 
      for (t = table; t; t = t->prev)
      {	for (p = t->list; p; p = p->next)
	{ if (p->info.sto & Sattribute)
	    if (is_string(p->info.typ))
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", a->%s);", ns_convert(p->sym->name), p->sym->name);
            else if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, a->%s));", ns_convert(p->sym->name), c_type(p->info.typ), p->sym->name);
            else if (p->info.typ->type == Tenum)
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, a->%s));", ns_convert(p->sym->name), c_ident(p->info.typ), p->sym->name);
            else
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, a->%s));", ns_convert(p->sym->name), the_type(p->info.typ), p->sym->name);
        }
      }
       if (is_primclass(typ))
       {
	for (table = (Table*)typ->ref; table; table = table->prev)
	{ p = table->list;
	  if (p && !strcmp(p->sym->name, "__item"))
	    break;
        }
	  if ((p->info.sto & SmustUnderstand) && !(p->info.sto & Sconst) && !(p->info.sto & Sattribute) && !is_transient(p->info.typ) && !is_void(p->info.typ) && p->info.typ->type != Tfun)
	    fprintf(fout, "\n\tsoap->mustUnderstand = 1;");
	  else if(p->info.typ->type==Tarray){
	    fprintf(fout,"\n\tsoap_out_%s(soap, tag, id, a->%s, \"%s\");", c_ident(p->info.typ), p->sym->name, xsi_type(typ));
	  }
	  else if(p->info.typ->type==Tclass && !is_external(p->info.typ)){
	    /* CLASS ( within CLASS ) */
	    fprintf(fout,"\n\t(a->%s.soap_out(soap, tag, id, \"%s\");", p->sym->name,xsi_type(typ));
	  }
	  else if (is_XML(p->info.typ) && is_string(p->info.typ))
	    fprintf(fout,"\n\tsoap_outliteral(soap, \"%s\", &a->%s);", ns_convert(p->sym->name),p->sym->name);
	  else if (is_XML(p->info.typ) && is_wstring(p->info.typ))
	    fprintf(fout,"\n\tsoap_outwliteral(soap, \"%s\", &a->%s);", ns_convert(p->sym->name),p->sym->name);
	  else if (p->info.typ->type != Tfun && !is_void(p->info.typ)) {
	    fprintf(fout,"\n\tsoap_out_%s(soap, tag, id, &a->%s, \"%s\");", c_ident(p->info.typ), p->sym->name,xsi_type(typ));
	  }
   }
   else
   {  fprintf(fout,"\n\tsoap_element_begin_out(soap, tag, soap_embedded_id(soap, id, a, SOAP_TYPE_%s), type);",c_ident(typ));
      fflush(fout);
      for (t = table; t; t = t->prev)
      {	for (p = t->list; p; p = p->next)
	{ if ((p->info.sto & SmustUnderstand) && !(p->info.sto & Sconst) && !is_transient(p->info.typ) && !is_void(p->info.typ) && p->info.typ->type != Tfun)
	    fprintf(fout, "\n\tsoap->mustUnderstand = 1;");
	  if (p->info.sto & Sconst)
	    fprintf(fout, "\n\t/* const %s skipped */", p->sym->name);
	  else if (is_transient(p->info.typ))
	    fprintf(fout, "\n\t/* transient %s skipped */", p->sym->name);
	  else if (p->info.sto & Sattribute)
	    ;
	  else if (is_repetition(p))
	  { fprintf(fout,"\n\tif (a->%s)", p->next->sym->name);
            fprintf(fout,"\n\t{\tint i;\n\t\tfor (i = 0; i < a->%s; i++)", p->sym->name);
            if (((Tnode*)p->next->info.typ->ref)->type == Tclass && !is_external(p->next->info.typ->ref))
              fprintf(fout,"\n\t\t\ta->%s[i].soap_out(soap, \"%s\", -1, \"%s\");", p->next->sym->name, ns_convert(p->next->sym->name),xsi_type(p->next->info.typ->ref));
            else
              fprintf(fout,"\n\t\t\tsoap_out_%s(soap, \"%s\", -1, a->%s + i, \"%s\");", c_ident(p->next->info.typ->ref), ns_convert(p->next->sym->name), p->next->sym->name, xsi_type(p->next->info.typ->ref));
            fprintf(fout,"\n\t}");
            p = p->next;
	  }
	  else if (p->info.typ->type==Tarray)
	    fprintf(fout,"\n\tsoap_out_%s(soap, \"%s\", -1, a->%s, \"%s\");",
	    c_ident(p->info.typ),ns_convert(p->sym->name),p->sym->name,xsi_type(p->info.typ));
	  else if (p->info.typ->type==Tclass && !is_external(p->info.typ))
	    fprintf(fout,"\n\ta->%s.soap_out(soap, \"%s\", -1, \"%s\");", p->sym->name,ns_convert(p->sym->name),xsi_type(p->info.typ));
	  else if (is_XML(p->info.typ) && is_string(p->info.typ))
	    fprintf(fout,"\n\tsoap_outliteral(soap, \"%s\", &a->%s);", ns_convert(p->sym->name),p->sym->name);
	  else if (is_XML(p->info.typ) && is_wstring(p->info.typ))
	    fprintf(fout,"\n\tsoap_outwliteral(soap, \"%s\", &a->%s);", ns_convert(p->sym->name),p->sym->name);
	  else if (p->info.typ->type != Tfun && !is_void(p->info.typ))
	    fprintf(fout,"\n\tsoap_out_%s(soap, \"%s\", -1, &a->%s, \"%s\");",
	    c_ident(p->info.typ),ns_convert(p->sym->name),p->sym->name, xsi_type(p->info.typ));
	}
      }	
      fprintf(fout,"\n\tsoap_element_end_out(soap, tag);");
    }
    fprintf(fout,"\n\treturn SOAP_OK;\n}");	 
    fflush(fout);
    break;
      
    case Tclass:
      if (is_external(typ))
      { fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char*, int, const %s, const char*);", c_ident(typ),c_type_id(typ, "*")); 
        return;
      }
      table=(Table*)typ->ref;
      fprintf(fout,"\n\nint %s::soap_out(struct soap *soap, const char *tag, int id, const char *type) const", typ->id->name); 
      fprintf(fout,"\n{\n\treturn soap_out_%s(soap, tag, id, this, type);\n}", typ->id->name); 
      fprintf(fhead,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char*, int, const %s, const char*);", typ->id->name, c_type_id(typ, "*")); 
      fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, const %s, const char *type)\n{", typ->id->name, c_type_id(typ, "*a")); 
      fflush(fout);
      for (t = table; t; t = t->prev)
      {	for (p = t->list; p; p = p->next)
	{ if (p->info.sto & Sattribute)
	    if (is_string(p->info.typ))
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", ((%s*)a)->%s);", ns_convert(p->sym->name), t->sym->name, p->sym->name);
            else if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, ((%s*)a)->%s));", ns_convert(p->sym->name), c_type(p->info.typ), t->sym->name, p->sym->name);
            else if (p->info.typ->type == Tenum)
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, ((%s*)a)->%s));", ns_convert(p->sym->name), c_ident(p->info.typ), t->sym->name, p->sym->name);
            else
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, ((%s*)a)->%s));", ns_convert(p->sym->name), the_type(p->info.typ), t->sym->name, p->sym->name);
        }
      }
       if (is_primclass(typ))
       {
	for (table = (Table*)typ->ref; table; table = table->prev)
	{ p = table->list;
	  if (p && !strcmp(p->sym->name, "__item"))
	    break;
        }
	  if ((p->info.sto & SmustUnderstand) && !(p->info.sto & Sconst) && !(p->info.sto & Sattribute) && !is_transient(p->info.typ) && !is_void(p->info.typ) && p->info.typ->type != Tfun)
	    fprintf(fout, "\n\tsoap->mustUnderstand = 1;");
	  if (p->info.sto & Sconst)
	    fprintf(fout, "\n\t/* const %s skipped */", p->sym->name);
	  else if (is_transient(p->info.typ))
	    fprintf(fout, "\n\t/* transient %s skipped */", p->sym->name);
	  else if (p->info.sto & Sattribute)
	    ;
	  else if(p->info.typ->type==Tarray){
	    fprintf(fout,"\n\tsoap_out_%s(soap, tag, id, ((%s*)a)->%s, \"%s\");", c_ident(p->info.typ), table->sym->name,p->sym->name, xsi_type(typ));
	  }
	  else if(p->info.typ->type==Tclass && !is_external(p->info.typ)){
	    /* CLASS ( within CLASS ) */
	    fprintf(fout,"\n\t(((%s*)a)->%s).soap_out(soap, tag, id, \"%s\");", table->sym->name, p->sym->name,xsi_type(typ));
	  }
	  else if (is_XML(p->info.typ) && is_string(p->info.typ))
	    fprintf(fout,"\n\tsoap_outliteral(soap, \"%s\", &(((%s*)a)->%s);", ns_convert(p->sym->name),table->sym->name,p->sym->name);
	  else if (is_XML(p->info.typ) && is_wstring(p->info.typ))
	    fprintf(fout,"\n\tsoap_outwliteral(soap, \"%s\", &(((%s*)a)->%s);", ns_convert(p->sym->name),table->sym->name,p->sym->name);
	  else if (p->info.typ->type != Tfun && !is_void(p->info.typ)) {
	    fprintf(fout,"\n\tsoap_out_%s(soap, tag, id, &(((%s*)a)->%s), \"%s\");", c_ident(p->info.typ), table->sym->name,p->sym->name,xsi_type(typ));
	  }
       }
       else
       { fprintf(fout,"\n\tsoap_element_begin_out(soap, tag, soap_embedded_id(soap, id, a, SOAP_TYPE_%s), \"%s\");", c_ident(typ), xsi_type(typ));
  	fflush(fout);
      
      i=0;
      /* Get the depth of the inheritance hierarchy */
      for (t = table; t != (Table *) 0; t = t->prev) { 
	/*if(t==(Table *) 0 ) return;*/
	i++;
      }

      /* Call routines to output the member data of the class */
      /* Data members of the Base Classes are outputed first
	 followed by the data members of the Derived classes.
	 Over written data members are outputed twice once for the base class
	 they are defined in and once for the derived class that overwrites
	 them */
      
      for(;i>0;i--){
	t = table;
	for(j=0;j<i-1;j++){
	  t = t->prev;
	}
	
	for (p = t->list; p != (Entry*) 0; p = p->next) {
	  if ((p->info.sto & SmustUnderstand) && !(p->info.sto & Sconst) && !(p->info.sto & Sattribute) && !is_transient(p->info.typ) && !is_void(p->info.typ) && p->info.typ->type != Tfun)
	    fprintf(fout, "\n\tsoap->mustUnderstand = 1;");
	  if (p->info.sto & Sconst)
	    fprintf(fout, "\n\t/* const %s skipped */", p->sym->name);
	  else if (is_transient(p->info.typ))
	    fprintf(fout, "\n\t/* transient %s skipped */", p->sym->name);
	  else if (p->info.sto & Sattribute)
	    ;
	  else if (is_repetition(p))
	  { 
      fprintf(fout,"\n\tif (((%s*)a)->%s)", table->sym->name, p->next->sym->name);
      fprintf(fout,"\n\t{\tint i;\n\t\tfor (i = 0; i < ((%s*)a)->%s; i++)", table->sym->name, p->sym->name);
      if (((Tnode*)p->next->info.typ->ref)->type == Tclass && !is_external(p->next->info.typ->ref))
        fprintf(fout,"\n\t\t\t((%s*)a)->%s[i].soap_out(soap, \"%s\", -1, \"%s\");", table->sym->name, p->next->sym->name, ns_overridden(t, p->next),xsi_type(p->next->info.typ->ref));
      else
        fprintf(fout,"\n\t\t\tsoap_out_%s(soap, \"%s\", -1, ((%s*)a)->%s + i, \"%s\");", c_ident(p->next->info.typ->ref), ns_overridden(t, p->next), table->sym->name, p->next->sym->name, xsi_type(p->next->info.typ->ref));
      fprintf(fout,"\n\t}");
          p = p->next;
	  }
	  else if(p->info.typ->type==Tarray){
	    fprintf(fout,"\n\tsoap_out_%s(soap, \"%s\", -1, ((%s*)a)->%s, \"%s\");", c_ident(p->info.typ),ns_overridden(t, p), t->sym->name,p->sym->name, xsi_type(p->info.typ));
	  }
	  else if(p->info.typ->type==Tclass && !is_external(p->info.typ)){
	    /* CLASS ( within CLASS ) */
	    fprintf(fout,"\n\t(((%s*)a)->%s).soap_out(soap, \"%s\", -1, \"%s\");", t->sym->name, p->sym->name,ns_overridden(t, p),xsi_type(p->info.typ));
	  }
	  else if (is_XML(p->info.typ) && is_string(p->info.typ))
	    fprintf(fout,"\n\tsoap_outliteral(soap, \"%s\", &(((%s*)a)->%s);", ns_overridden(t, p),t->sym->name,p->sym->name);
	  else if (is_XML(p->info.typ) && is_wstring(p->info.typ))
	    fprintf(fout,"\n\tsoap_outwliteral(soap, \"%s\", &(((%s*)a)->%s);", ns_overridden(t, p),t->sym->name,p->sym->name);
	  else if (p->info.typ->type != Tfun && !is_void(p->info.typ)) {
	    fprintf(fout,"\n\tsoap_out_%s(soap, \"%s\", -1, &(((%s*)a)->%s), \"%s\");", c_ident(p->info.typ),ns_overridden(t, p), t->sym->name,p->sym->name,xsi_type(p->info.typ));
	  }
  	fflush(fout);
	}
      }
      
         fprintf(fout,"\n\tsoap_element_end_out(soap, tag);");
      }
      fprintf(fout,"\n\treturn SOAP_OK;\n}");	 
  	fflush(fout);
      break;
      
    case Tpointer:
      fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char *, int, %s, const char *);", c_ident(typ),c_type_id(typ, "const*")); 
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, %s, const char *type)\n{", c_ident(typ),c_type_id(typ, "const*a")); 
      fprintf(fout,"\n\tid = soap_embedded_id(soap, id, a, SOAP_TYPE_%s);",c_ident(typ));
      fprintf(fout,"\n\tif (!*a)");
      fprintf(fout,"\n\t\tsoap_element_null(soap, tag, id, type);");
      fprintf(fout,"\n\telse");
      fprintf(fout,"\n\t{\tstruct soap_plist *pp;");   
      if (p = is_dynamic_array(typ->ref))
      { d = get_Darraydims(typ->ref);
        if (d)
          fprintf(fout,"\n\t\tint i = soap_array_pointer_lookup(soap, *a, (struct soap_array*)&(*a)->%s, soap_size((*a)->__size, %d), SOAP_TYPE_%s, &pp);", p->sym->name, d, c_ident(typ->ref));
        else
          fprintf(fout,"\n\t\tint i = soap_array_pointer_lookup(soap, *a, (struct soap_array*)&(*a)->%s, (*a)->__size, SOAP_TYPE_%s, &pp);", p->sym->name, c_ident(typ->ref));
      }
      else
        fprintf(fout,"\n\t\tint i = soap_pointer_lookup(soap, *a, SOAP_TYPE_%s, &pp);",c_ident(typ->ref));
      /*
      fprintf(fout,"\n\t         if (id > 0)");
      fprintf(fout,"\n\t		{"); 
      fprintf(fout,"\n\t	        if (i)");
      fprintf(fout,"\n\t	              if (soap_is_embedded(soap, pp))");
      fprintf(fout,"\n\t			   soap_element_ref(soap, tag, id, i);");
      fprintf(fout,"\n\t                   else if (soap_is_single(soap, pp))");

      */
      if(((Tnode *) typ->ref)->type == Tclass && !is_external(typ->ref)){
	/* Pointer points to a CLASS */
/*	fprintf(fout,"\n\t 		           (*a)->soap_out(soap, tag, 0, type);");
	fprintf(fout,"\n\t                   else\n\t{");
	fprintf(fout,"\n\t			   soap_set_embedded(soap, pp);");
	fprintf(fout,"\n\t                       (*a)->soap_out(soap, tag, i, type);");
	fprintf(fout,"\n\t                   }");
	fprintf(fout,"\n\t                   else   (*a)->soap_out(soap, tag, soap_pointer_enter(soap, *a, SOAP_TYPE_%s, &pp), type);",c_ident(typ->ref));	
	fprintf(fout,"\n\t              }\n\t              else");
	*/
	fprintf(fout,"\n\t\tif (i)");
	fprintf(fout,"\n\t\t\tif (soap_is_embedded(soap, pp))");
	fprintf(fout,"\n\t\t\t\tsoap_element_ref(soap, tag, id, i);");
	fprintf(fout,"\n\t\t\telse if (soap_is_single(soap, pp))");
	fprintf(fout,"\n\t\t\t\t(*a)->soap_out(soap, tag, 0, type);");
	fprintf(fout,"\n\t\t\telse");
	fprintf(fout,"\n\t\t\t{\tsoap_set_embedded(soap, pp);");
	fprintf(fout,"\n\t\t\t\t(*a)->soap_out(soap, tag, i, type);");
	fprintf(fout,"\n\t\t\t}");
	fprintf(fout,"\n\t\telse");
	fprintf(fout,"\n\t\t\t(*a)->soap_out(soap, tag, soap_pointer_enter(soap, *a, SOAP_TYPE_%s, &pp), type);", c_ident(typ->ref));
      }
      else
      {
      /*
	fprintf(fout,"\n\t 		           soap_out_%s(soap, tag, 0, *a, type);",c_ident(typ->ref));
	fprintf(fout,"\n\t                   else\n\t{");
	fprintf(fout,"\n\t			   soap_set_embedded(soap, pp);");
	fprintf(fout,"\n\t                       soap_out_%s(soap, tag, i, *a, type);",c_ident(typ->ref));
	fprintf(fout,"\n\t                   }");
	fprintf(fout,"\n\t                   else   soap_out_%s(soap, tag, soap_pointer_enter(soap, *a, SOAP_TYPE_%s, &pp), *a, type);",c_ident(typ->ref),c_ident(typ->ref));	
	fprintf(fout,"\n\t              }\n\t              else");
	*/
	fprintf(fout,"\n\t\tif (i)");
	fprintf(fout,"\n\t\t\tif (soap_is_embedded(soap, pp))");
	fprintf(fout,"\n\t\t\t\tsoap_element_ref(soap, tag, id, i);");
	fprintf(fout,"\n\t\t\telse if (soap_is_single(soap, pp))");
	fprintf(fout,"\n\t\t\t\tsoap_out_%s(soap, tag, 0, *a, type);",c_ident(typ->ref));
	fprintf(fout,"\n\t\t\telse");
	fprintf(fout,"\n\t\t\t{\tsoap_set_embedded(soap, pp);");
	fprintf(fout,"\n\t\t\t\tsoap_out_%s(soap, tag, i, *a, type);",c_ident(typ->ref));
	fprintf(fout,"\n\t\t\t}");
	fprintf(fout,"\n\t\telse");
        if (p = is_dynamic_array(typ->ref))
	  fprintf(fout,"\n\t\t\tsoap_out_%s(soap, tag, soap_array_pointer_enter(soap, *a, (struct soap_array*)&(*a)->%s, SOAP_TYPE_%s, &pp), *a, type);",c_ident(typ->ref), p->sym->name, c_ident(typ->ref));
	else
	  fprintf(fout,"\n\t\t\tsoap_out_%s(soap, tag, soap_pointer_enter(soap, *a, SOAP_TYPE_%s, &pp), *a, type);",c_ident(typ->ref), c_ident(typ->ref));
      }
      fprintf(fout,"\n\t}\n\treturn SOAP_OK;\n}");
      break;

    case Tarray:
      fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char*, int, %s, const char*);", c_ident(typ),c_type_id(typ, "const")); 
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, %s, const char *type)\n{", c_ident(typ),c_type_id(typ, "const a")); 
      fprintf(fout,"\n\tint i;");
      if (is_untyped(typ->ref))
        fprintf(fout,"\n\tsoap_array_begin_out(soap, tag, soap_embedded_id(soap, id, a, SOAP_TYPE_%s), \"%s[%d]\", 0);",c_ident(typ), wsdl_type(typ->ref, "xsd"), get_dimension(typ));
      else
        fprintf(fout,"\n\tsoap_array_begin_out(soap, tag, soap_embedded_id(soap, id, a, SOAP_TYPE_%s), \"%s[%d]\", 0);",c_ident(typ), xsi_type_Tarray(typ), get_dimension(typ));
      n=typ->ref;
      cardinality = 1;
      while(n->type==Tarray)
	{
	  n=n->ref;
	  cardinality++;
	}

      fprintf(fout,"\n\tfor (i = 0; i < %d; i++)\n\t{",get_dimension(typ));
     if (((Tnode *)typ->ref)->type == Tclass && !is_external(typ->ref))
     { if(cardinality>1)
         fprintf(fout,"\n\t\ta[i].soap_out(soap, \"item\", -1, \"%s\")", xsi_type(typ->ref));
       else fprintf(fout,"\n\t\t(a+i)->soap_out(soap, \"item\", -1, \"%s\")", xsi_type(typ->ref));
     }
     else
     { if(((Tnode *)typ->ref)->type != Tarray)
       { if(((Tnode *)typ->ref)->type == Tpointer)
	  fprintf(fout,"\n\t\tsoap->position = 1;\n\t\tsoap->positions[0] = i;\n\t\tsoap_out_%s(soap, \"item\", -1, a", c_ident(typ->ref));
	 else
	  fprintf(fout,"\n\t\tsoap_out_%s(soap, \"item\", -1, a",c_ident(typ->ref));
       }
       else
         fprintf(fout,"\n\t\tsoap_out_%s(soap, \"item\", -1, a",c_ident(typ->ref));
       if(cardinality>1)
         fprintf(fout,"[i], \"%s\")", xsi_type(typ->ref));
       else
         fprintf(fout,"+i, \"%s\")", xsi_type(typ->ref));
      }
      if(((Tnode *)typ->ref)->type == Tpointer)
        fprintf(fout,";\n\t}\n\tsoap->position = 0;\n\tsoap_element_end_out(soap, tag);");
      else
        fprintf(fout,";\n\t}\n\tsoap_element_end_out(soap, tag);");		
      fprintf(fout,"\n\treturn SOAP_OK;\n}");
      break;

    case Tenum:
      fprintf(fhead, "\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char*, int, const %s, const char*);", c_ident(typ), c_type_id(typ, "*"));
      if (is_external(typ))
        return;
      if (!is_mask(typ))
      { fprintf(fhead, "\n\nSOAP_FMAC1 const char* SOAP_FMAC2 soap_%s2s(struct soap*, %s);", c_ident(typ), c_type(typ));
        fprintf(fout, "\n\nSOAP_FMAC1 const char* SOAP_FMAC2 soap_%s2s(struct soap *soap, %s)", c_ident(typ), c_type_id(typ, "n"));
        fprintf(fout, "\n{\tswitch (n)\n\t{");
        for (t = (Table*)typ->ref; t; t = t->prev)
        { for (p = t->list; p; p = p->next)
	    fprintf(fout, "\n\t\tcase %s: return \"%s\";", p->sym->name, ns_convert(p->sym->name));
        }	
        fprintf(fout, "\n\t}\n");
        fprintf(fout, "\n\tsprintf(soap->tmpbuf, SOAP_LONG_FORMAT, (LONG64)n);");
        fprintf(fout, "\n\treturn soap->tmpbuf;");
        fprintf(fout, "\n}");
      }
      if (is_mask(typ))
      { fprintf(fout, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, const %s, const char *type)", c_ident(typ), c_type_id(typ, "*a"));
        fprintf(fout, "\n{\tLONG64 i;\n\tsoap_element_begin_out(soap, tag, soap_embedded_id(soap, id, a, SOAP_TYPE_%s), type);", c_ident(typ));
        fprintf(fout, "\n\tfor (i = 1; i; i <<= 1)");
        fprintf(fout, "\n\t\tswitch ((LONG64)*a & i)\n\t\t{");
        for (t = (Table*)typ->ref; t; t = t->prev)
        { for (p = t->list; p; p = p->next)
	    fprintf(fout, "\n\t\t\tcase "SOAP_LONG_FORMAT": soap_send(soap, \"%s \"); break;", p->info.val.i, ns_convert(p->sym->name));
        }	
        fprintf(fout, "\n\t\t}");
      }
      else
      { fprintf(fout, "\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, const %s, const char *type)", c_ident(typ), c_type_id(typ, "*a"));
        fprintf(fout, "\n{\tsoap_element_begin_out(soap, tag, soap_embedded_id(soap, id, a, SOAP_TYPE_%s), type);", c_ident(typ));
        fprintf(fout, "\n\tsoap_send(soap, soap_%s2s(soap, *a));", c_ident(typ));
      }
      fprintf(fout, "\n\treturn soap_element_end_out(soap, tag);\n}");
    default: break;
    }
}	  

void
soap_out_Darray(Tnode *typ)
{ int i, j, d;
  Table *t, *table;
  Entry *p, *q;

  table=(Table*)typ->ref;

  fprintf(fhead,"\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap*, const char*, int, const %s, const char*);", c_ident(typ),c_type_id(typ, "*")); 
      if (is_external(typ))
        return;
  if (typ->type == Tclass)
  { fprintf(fout,"\n\nint %s::soap_out(struct soap *soap, const char *tag, int id, const char *type) const", c_type(typ)); 
    fprintf(fout,"\n{\treturn soap_out_%s(soap, tag, id, this, type);\n}", c_ident(typ)); 
  }
  fflush(fout);
  fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_out_%s(struct soap *soap, const char *tag, int id, const %s, const char *type)\n{", c_ident(typ),c_type_id(typ, "*a")); 
  fprintf(fout,"\n\tint i;\n\tstruct soap_plist *pp;");
  d = get_Darraydims(typ);
  if (d)
    fprintf(fout,"\n\tint n = soap_size(a->__size, %d);", d);
  else
    fprintf(fout,"\n\tint n = a->__size;");
  if (typ->type == Tclass)
  { for (t = table; t; t = t->prev)
    {	for (p = t->list; p; p = p->next)
	{ if (p->info.sto & Sattribute)
	    if (is_string(p->info.typ))
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", ((%s*)a)->%s);", ns_convert(p->sym->name), t->sym->name, p->sym->name);
            else if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, ((%s*)a)->%s));", ns_convert(p->sym->name), c_type(p->info.typ), t->sym->name, p->sym->name);
            else if (p->info.typ->type == Tenum)
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, ((%s*)a)->%s));", ns_convert(p->sym->name), c_ident(p->info.typ), t->sym->name, p->sym->name);
            else
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, ((%s*)a)->%s));", ns_convert(p->sym->name), the_type(p->info.typ), t->sym->name, p->sym->name);
        }
    }
  }
  else
  { for (t = table; t; t = t->prev)
    {	for (p = t->list; p; p = p->next)
	{ if (p->info.sto & Sattribute)
	    if (is_string(p->info.typ))
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", a->%s);", ns_convert(p->sym->name), p->sym->name);
            else if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, a->%s));", ns_convert(p->sym->name), c_type(p->info.typ), p->sym->name);
            else if (p->info.typ->type == Tenum)
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, a->%s));", ns_convert(p->sym->name), c_ident(p->info.typ), p->sym->name);
            else
	      fprintf(fout, "\n\tsoap_set_attr(soap, \"%s\", soap_%s2s(soap, a->%s));", ns_convert(p->sym->name), the_type(p->info.typ), p->sym->name);
        }
    }
  }
  p = is_dynamic_array(typ);
  q = table->list;
  if (!has_ns(typ) && !is_untyped(typ) && !is_binary(typ))
  { if (is_untyped(p->info.typ))
      if (typ->type == Tclass)
      { if (has_offset(typ))
          if (d)
	    fprintf(fout,"\n\tchar *t = soap_putsizesoffsets(soap, \"%s\", a->__size, a->__offset, %d);", wsdl_type(p->info.typ, "xsd"), d); 
	  else
	    fprintf(fout,"\n\tchar *t = soap_putsize(soap, \"%s\", n + a->__offset);", wsdl_type(p->info.typ, "xsd"));
        else if (d)
	  fprintf(fout,"\n\tchar *t = soap_putsizes(soap, \"%s\", a->__size, %d);", wsdl_type(p->info.typ, "xsd"), d);
        else
	  fprintf(fout,"\n\tchar *t = soap_putsize(soap, \"%s\", a->__size);", wsdl_type(p->info.typ, "xsd"));
      }
      else
      { if (has_offset(typ))
          if (d)
	    fprintf(fout,"\n\tchar *t = soap_putsizesoffsets(soap, type, a->__size, a->__offset, %d);", d); 
	  else
	    fprintf(fout,"\n\tchar *t = soap_putsize(soap, type, n + a->__offset);");
        else if (d)
	  fprintf(fout,"\n\tchar *t = soap_putsizes(soap, type, a->__size, %d);", d);
        else
	  fprintf(fout,"\n\tchar *t = soap_putsize(soap, type, a->__size);");
      }
    else if (typ->type == Tclass)
    { if (has_offset(typ))
        if (d)
          fprintf(fout,"\n\tchar *t = soap_putsizesoffsets(soap, \"%s\", a->__size, a->__offset, %d);", xsi_type(typ), d);
        else
          fprintf(fout,"\n\tchar *t = soap_putsize(soap, \"%s\", n + a->__offset);",xsi_type(typ));
      else if (d)
        fprintf(fout,"\n\tchar *t = soap_putsizes(soap, \"%s\", a->__size, %d);", xsi_type(typ),d);
      else
        fprintf(fout,"\n\tchar *t = soap_putsize(soap, \"%s\", a->__size);" ,xsi_type(typ));
    }
    else if (has_offset(typ))
      if (d)
        fprintf(fout,"\n\tchar *t = soap_putsizesoffsets(soap, type, a->__size, a->__offset, %d);", d); 
      else
        fprintf(fout,"\n\tchar *t = soap_putsize(soap, type, n + a->__offset);"); 
    else if (d)
      fprintf(fout,"\n\tchar *t = soap_putsizes(soap, type, a->__size, %d);", d); 
    else
      fprintf(fout,"\n\tchar *t = soap_putsize(soap, type, a->__size);"); 
  }
  fprintf(fout,"\n\ti = soap_array_pointer_lookup(soap, a, (struct soap_array*)&a->%s, n, SOAP_TYPE_%s, &pp);",p->sym->name,c_ident(typ));
  if (!has_ns(typ) && !is_untyped(typ) && !is_binary(typ))
    fprintf(fout,"\n\tif (!a->%s)\n\t{\tsoap_element_null(soap, tag, i, t);\n\t\treturn SOAP_OK;\n\t}", p->sym->name);
  else
    fprintf(fout,"\n\tif (!a->%s)\n\t{\tsoap_element_null(soap, tag, i, type);\n\t\treturn SOAP_OK;\n\t}", p->sym->name);
  if (is_attachment(typ))
  { fprintf(fout,"\n\tif (a->id || a->type)\n\t{\tif (i && !a->id)\n\t\t{\tsprintf(soap->tmpbuf, soap->dime_id_format, i);\n\t\t\tsoap_element_href(soap, tag, id, soap->tmpbuf);\n\t\t\tsoap_set_attached(soap, pp, soap->tmpbuf, a->type, a->options, n);\n\t\t}\n\t\telse\n\t\t{\tsoap_element_href(soap, tag, id, a->id);\n\t\t\tif (i)\n\t\t\t\tsoap_set_attached(soap, pp, a->id, a->type, a->options, n);\n\t\t}\n\t\treturn SOAP_OK;\n\t}");
  }
  fprintf(fout,"\n\tif (id <= 0 && i)\n\t{\tif (soap_is_embedded(soap, pp))\n\t\t{\tsoap_element_ref(soap, tag, 0, i);\n\t\t\treturn SOAP_OK;\n\t\t}\n\t\tif (soap_is_single(soap, pp))\n\t\t\ti = 0;\n\t}\n\telse if (id > 0)\n\t\ti = id;");
  if (has_ns(typ) || is_untyped(typ) || is_binary(typ))
    fprintf(fout,"\n\tsoap_element_begin_out(soap, tag, i, \"%s\");", xsi_type(typ));
  else if (has_offset(typ))
    if (d)
      fprintf(fout,"\n\tsoap_array_begin_out(soap, tag, i, t, soap_putoffsets(soap, a->__offset, %d));", d);
    else
      fprintf(fout,"\n\tsoap_array_begin_out(soap, tag, i, t, soap_putoffset(soap, a->__offset));");
  else
    fprintf(fout,"\n\tsoap_array_begin_out(soap, tag, i, t, NULL);");
  fprintf(fout,"\n\tif (i)\n\t\tsoap_set_embedded(soap, pp);");
  if (is_binary(typ) && !is_hexBinary(typ))
    fprintf(fout, "\n\tsoap_putbase64(soap, a->__ptr, a->__size);");
  else
  { fprintf(fout,"\n\tfor (i = 0; i < n; i++)\n\t{");
    if (is_hexBinary(typ))
      fprintf(fout, "\n\t\tsoap_puthex(soap, (a->__ptr)[i]);");
    else if (((Tnode *)p->info.typ->ref)->type == Tclass && !is_external(p->info.typ->ref))
      fprintf(fout,"\n\t\ta->%s[i].soap_out(soap, \"item\", -1, \"%s\");", p->sym->name, xsi_type(((Tnode *)p->info.typ->ref)));
    else if (!has_ns(typ) && !is_untyped(typ) && ((Tnode *)p->info.typ->ref)->type == Tpointer)
    { if (d)
      { fprintf(fout,"\n\t\tsoap->position = %d;", d);
        for (i = 0; i < d; i++)
	{ fprintf(fout, "\n\t\tsoap->positions[%d] = i", i);
          for (j = i+1; j < d; j++)
	    fprintf(fout, "/a->__size[%d]", j);
	  fprintf(fout, "%%a->__size[%d];", i);
        }
	fprintf(fout, "\n\t\tsoap_out_%s(soap, \"%s\", 0, &a->%s[i], \"%s\");",c_ident(((Tnode *)p->info.typ->ref)), 
	p->sym->name[5]?ns_convert(p->sym->name+5):"item", p->sym->name,
	xsi_type(((Tnode *)p->info.typ->ref)));
      }
      else
        fprintf(fout,"\n\t\tsoap->position = 1;\n\t\tsoap->positions[0] = i;\n\t\tsoap_out_%s(soap, \"%s\", 0, &a->%s[i], \"%s\");",c_ident(((Tnode *)p->info.typ->ref)), 
	p->sym->name[5]?ns_convert(p->sym->name+5):"item", p->sym->name,
	xsi_type(((Tnode *)p->info.typ->ref)));
    }
    else
      fprintf(fout,"\n\t\tsoap_out_%s(soap, \"%s\", 0, &a->%s[i], \"%s\");",c_ident(((Tnode *)p->info.typ->ref)), 
	p->sym->name[5]?ns_convert(p->sym->name+5):"item", p->sym->name,
      xsi_type(((Tnode *)p->info.typ->ref)));
  }
  if (!has_ns(typ) && !is_untyped(typ) && ((Tnode *)p->info.typ->ref)->type == Tpointer)
    fprintf(fout,"\n\t}\n\tsoap->position = 0;\n\tsoap_element_end_out(soap, tag);");
  else if (is_binary(typ) && !is_hexBinary(typ))
    fprintf(fout,"\n\tsoap_element_end_out(soap, tag);");
  else
    fprintf(fout,"\n\t}\n\tsoap_element_end_out(soap, tag);");
  fprintf(fout,"\n\treturn SOAP_OK;\n}");	 
}

void
soap_get(Tnode *typ)
{
  Tnode *temp;
  
  if(typ->type==Tarray)
    {
      /* ARRAY */
      temp = typ;
      while(temp->type == Tarray){
	temp = temp->ref;
      }
      fprintf(fhead,"\nSOAP_FMAC1 %s * SOAP_FMAC2 soap_get_%s(struct soap*, %s, const char*, const char*);", c_type(temp),c_ident(typ),c_type(typ));
      fprintf(fout,"\nSOAP_FMAC1 %s * SOAP_FMAC2 soap_get_%s(struct soap *soap, %s, const char *tag, const char *type)", c_type(temp),c_ident(typ),c_type_id(typ, "a"));
      fprintf(fout,"\n{\t%s;",c_type_id(temp, "(*p)"));
      fprintf(fout,"\n\tif ((p = soap_in_%s(soap, tag, a, type)))", c_ident(typ));
    }
  else if(typ->type==Tclass && !is_external(typ))
    {
      /* CLASS  */
      fprintf(fout,"\n\nvoid *%s::soap_get(struct soap *soap, const char *tag, const char *type)", c_type(typ));
      fprintf(fout,"\n{\n\treturn soap_get_%s(soap, this, tag, type);\n}", c_ident(typ));
      fprintf(fhead,"\nSOAP_FMAC1 %s SOAP_FMAC2 soap_get_%s(struct soap*, %s, const char*, const char*);", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*"));
      fprintf(fout,"\n\nSOAP_FMAC1 %s SOAP_FMAC2 soap_get_%s(struct soap *soap, %s, const char *tag, const char *type)\n{", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*p"));
      fprintf(fout,"\n\tif ((p = soap_in_%s(soap, tag, p, type)))", c_ident(typ));
    }
  else 
    {
      fprintf(fhead,"\nSOAP_FMAC1 %s SOAP_FMAC2 soap_get_%s(struct soap*, %s, const char*, const char*);", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*"));
      fprintf(fout,"\n\nSOAP_FMAC1 %s SOAP_FMAC2 soap_get_%s(struct soap *soap, %s, const char *tag, const char *type)\n{", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*p"));
      fprintf(fout,"\n\tif ((p = soap_in_%s(soap, tag, p, type)))", c_ident(typ));
    }
  fprintf(fout,"\n\t\tsoap_getindependent(soap);");
  fprintf(fout,"\n\treturn p;\n}");
  fflush(fout);
}

void
soap_in(Tnode *typ)
{ Entry *p;
  Table *table,*t;
  int total,a, cardinality,i,j;
  Tnode *n, *temp;
  if (is_dynamic_array(typ))
  { soap_in_Darray(typ);
    return;
  }
  if (is_primitive_or_string(typ) && typ->type != Tenum)
  {
    fprintf(fhead,"\nSOAP_FMAC1 %s * SOAP_FMAC2 soap_in_%s(struct soap*, const char*, %s, const char*);", c_type(typ), c_ident(typ),c_type_id(typ, "*")); 
      if (is_external(typ))
        return;
    fprintf(fout,"\n\nSOAP_FMAC1 %s * SOAP_FMAC2 soap_in_%s(struct soap *soap, const char *tag, %s, const char *type)\n{", c_type(typ), c_ident(typ),c_type_id(typ, "*a")); 
    if (typ->type == Tllong || typ->type == Tullong)
      fprintf(fout,"\n\treturn soap_in%s(soap, tag, a, type, SOAP_TYPE_%s);\n}", c_type(typ), c_ident(typ));
    else if (is_wstring(typ))
      fprintf(fout,"\n\treturn soap_inwstring(soap, tag, a, type, SOAP_TYPE_%s);\n}", c_ident(typ));
    else if (is_string(typ))
      fprintf(fout,"\n\treturn soap_in%s(soap, tag, a, type, SOAP_TYPE_%s, %d);\n}", the_type(typ), c_ident(typ), is_qname(typ)+1);
    else
      fprintf(fout,"\n\treturn soap_in%s(soap, tag, a, type, SOAP_TYPE_%s);\n}", the_type(typ), c_ident(typ));
    return;
  }
  switch(typ->type)
  { case Tstruct:
      fprintf(fhead,"\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap*, const char*, %s, const char*);", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*"));
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap *soap, const char *tag, %s, const char *type)\n{", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*a"));
      table = (Table *)typ->ref;
      if (is_primclass(typ))
      { fprintf(fout, "\n\tif (soap_peek_element(soap))\n\t\treturn NULL;");
        fprintf(fout,"\n\tif (!(a = (%s)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0)))\n\t{\tsoap->error = SOAP_TAG_MISMATCH;\n\t\treturn NULL;\n\t}", c_type_id(typ, "*"), c_ident(typ), c_type(typ));
        fprintf(fout,"\n\t*soap->id = '\\0';");
        fprintf(fout,"\n\tif (soap->alloced)");
        fprintf(fout,"\n\t\tsoap_default_%s(soap, a);",c_ident(typ));
          for (t = (Table*)typ->ref; t; t = t->prev)
          { for (p = t->list; p; p = p->next) 
	      if (p->info.sto & Sattribute)
                if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", c_type(p->info.typ), ns_convert(p->sym->name), p->sym->name);
                else if (p->info.typ->type == Tenum)
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", c_ident(p->info.typ), ns_convert(p->sym->name), p->sym->name);
                else if (is_qname(p->info.typ))
	          fprintf(fout, "\n\t\tif (soap_s2QName(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", ns_convert(p->sym->name), p->sym->name);
                else
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", the_type(p->info.typ), ns_convert(p->sym->name), p->sym->name);
	  }
      fflush(fout);
	for (table = (Table*)typ->ref; table; table = table->prev)
	{ p = table->list;
	  if (p && !strcmp(p->sym->name, "__item"))
	    break;
        }
	    if (is_XML(p->info.typ) && is_string(p->info.typ))
	    { fprintf(fout,"\n\tif (!soap_inliteral(soap, tag, &a->%s, \"%s\"))", p->sym->name,xsi_type(typ));
	    }
	    else if (is_XML(p->info.typ) && is_wstring(p->info.typ))
	    { fprintf(fout,"\n\tif (!soap_inwliteral(soap, tag, &a->%s, \"%s\"))", p->sym->name,xsi_type(typ));
	    }
	    else if(p->info.typ->type==Tarray) {
	      fprintf(fout,"\n\tif (!soap_in_%s(soap, tag, a->%s, \"%s\"))",
		      c_ident(p->info.typ),p->sym->name,xsi_type(typ));
	    }
	    else if(p->info.typ->type==Tclass && !is_external(p->info.typ)) {
	      /*CLASS IN CLASS */
	      fprintf(fout,"\n\tif (!a->%s.soap_in(soap, tag, \"%s\"))",
		     p->sym->name,xsi_type(p->info.typ));
	    }
	    else if (p->info.typ->type != Tfun && !is_void(p->info.typ)) {
	      fprintf(fout,"\n\tif (!soap_in_%s(soap, tag, &a->%s, \"%s\"))",
		      c_ident(p->info.typ),p->sym->name,xsi_type(typ));
	    }
            /*fprintf(fout,"\n\t\tif (soap->error)\n\t\t\t{\tif (f)\n\t\t\t\t\tdelete a;\n\t\t\t\tsoap_dealloc(soap, * p);\n\t\t\t\treturn NULL;\n\t\t\t}");*/
           fprintf(fout,"\n\t\treturn NULL;");
      }
      else
      { a=0;
        for (t = table; t; t = t->prev)
	  for (p = table->list; p; p = p->next)
	  { if (!(p->info.sto & Sconst) && !(p->info.sto & Sattribute) && p->info.typ->type != Tfun && !is_void(p->info.typ) && !is_transient(p->info.typ) && !is_repetition(p))
	      if (a==0)
	      { fprintf(fout,"\n\tshort soap_flag_%s = 1", p->sym->name);
	        a=1;
              }
	      else
	        fprintf(fout,", soap_flag_%s = 1", p->sym->name);
	  }
	  fprintf(fout,";");
          fprintf(fout,"\n\tif (soap_element_begin_in(soap, tag))\n\t\treturn NULL;");
          fprintf(fout,"\n\tif (*soap->type && soap_match_tag(soap, soap->type, type))");
          fprintf(fout,"\n\t{\tsoap->error = SOAP_TYPE_MISMATCH;\n\t\tsoap_revert(soap);");
          fprintf(fout,"\n\t\treturn NULL;\n\t}");
          fprintf(fout,"\n\tif (soap->null)\n\t{\tif (soap->mode & SOAP_XML_NIL)");
          fprintf(fout,"\n\t\t{\tsoap->error = SOAP_NULL;");
          fprintf(fout,"\n\t\t\treturn NULL;");
          fprintf(fout,"\n\t\t}\n\t\telse");
          fprintf(fout,"\n\t\t\treturn a;");
          fprintf(fout,"\n\t}");
         fprintf(fout,"\n\tif (!*soap->href)");
          fprintf(fout,"\n\t{\ta = (%s)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0);",c_type_id(typ, "*"), c_ident(typ), c_type(typ));
        fprintf(fout,"\n\t\tif (!a)\n\t\t\treturn NULL;");
        fprintf(fout,"\n\t\tif (soap->alloced)");
        fprintf(fout,"\n\t\t\tsoap_default_%s(soap, a);",c_ident(typ));
          for (t = table; t; t = t->prev)
          { for (p = t->list; p; p = p->next) 
	      if (p->info.sto & Sattribute)
                if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", c_type(p->info.typ), ns_convert(p->sym->name), p->sym->name);
                else if (p->info.typ->type == Tenum)
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", c_ident(p->info.typ), ns_convert(p->sym->name), p->sym->name);
                else if (is_qname(p->info.typ))
	          fprintf(fout, "\n\t\tif (soap_s2QName(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", ns_convert(p->sym->name), p->sym->name);
                else
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", the_type(p->info.typ), ns_convert(p->sym->name), p->sym->name);
	  }
        fprintf(fout,"\n\t\tif (soap->body)\n\t\t{\tfor (;;)\n\t\t\t{");
        if (!is_primclass(typ))
          fprintf(fout,"\tsoap->error = SOAP_TAG_MISMATCH;");

        a=0;
        for (t = table; t; t = t->prev)
	{ for (p = t->list; p; p = p->next) 
	    if (p->info.sto & Sconst)
	      fprintf(fout, "\n\t\t/* const %s skipped */", p->sym->name);
	    else if (is_transient(p->info.typ))
	      fprintf(fout, "\n\t\t/* transient %s skipped */", p->sym->name);
	    else if (p->info.sto & Sattribute)
	      ;
	    else if (is_repetition(p))
	    { 
    fprintf(fout,"\n\t\t\tif (soap_flag_%s && soap->error == SOAP_TAG_MISMATCH)",p->next->sym->name);
    fprintf(fout,"\n\t\t\t{\t%s;\n\t\t\t\tsoap_new_block(soap);", c_type_id(p->next->info.typ, "p"));
    fprintf(fout,"\n\t\t\t\tfor (a->%s = 0; ; a->%s++)", p->sym->name, p->sym->name);
    fprintf(fout,"\n\t\t\t\t{\tp = (%s)soap_push_block(soap, sizeof(%s));", c_type(p->next->info.typ), c_type(p->next->info.typ->ref));
    if (((Tnode*)p->next->info.typ->ref)->type == Tclass && !is_external(p->next->info.typ->ref))
      fprintf(fout,"\n\t\t\t\t\tp->%s::soap_default(soap);", c_type(p->next->info.typ->ref));
    else
      fprintf(fout,"\n\t\t\t\t\tsoap_default_%s(soap, p);", c_ident(p->next->info.typ->ref));
    fprintf(fout,"\n\t\t\t\t\tif (!soap_in_%s(soap, \"%s\", p, \"%s\"))", c_ident(p->next->info.typ->ref), ns_convert(p->next->sym->name), xsi_type(p->next->info.typ->ref));
    fprintf(fout,"\n\t\t\t\t\t\tbreak;");
    fprintf(fout,"\n\t\t\t\t}");
    fprintf(fout,"\n\t\t\t\tsoap_pop_block(soap);");
    if (((Tnode*)p->next->info.typ->ref)->type == Tclass)
      fprintf(fout,"\n\t\t\t\tif (soap->blist->size)\n\t\t\t\t\ta->%s = soap_instantiate_%s(soap, soap->blist->size/sizeof(%s), NULL, NULL);\n\t\t\t\telse\n\t\t\t\t\ta->%s = NULL;", p->next->sym->name, c_ident(p->next->info.typ->ref), c_type(p->next->info.typ->ref), p->next->sym->name);
    else
      fprintf(fout,"\n\t\t\t\ta->%s = (%s)soap_malloc(soap, soap->blist->size);", p->next->sym->name, c_type(p->next->info.typ));
    fprintf(fout,"\n\t\t\t\tsoap_store_block(soap, (char*)a->%s);", p->next->sym->name);
    fprintf(fout,"\n\t\t\t\tsoap_flag_%s = 0;", p->next->sym->name);
    fprintf(fout,"\n\t\t\t\tif (soap->error == SOAP_TAG_MISMATCH)\n\t\t\t\t\tcontinue;\n\t\t\t}");
          p = p->next;
	  }
	  else
	  { 
	    if (!is_primclass(typ) && p->info.typ->type != Tfun && !is_void(p->info.typ))
	      fprintf(fout,"\n\t\t\tif (soap_flag_%s && soap->error == SOAP_TAG_MISMATCH)",p->sym->name);
	   if (is_unmatched(p->sym))
	   {
	    if (is_XML(p->info.typ) && is_string(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_inliteral(soap, NULL, &a->%s))", p->sym->name);
	    } else if (is_XML(p->info.typ) && is_wstring(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_inwliteral(soap, NULL, &a->%s))", p->sym->name);
	    /*} else if (is_string(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_string(soap, NULL, &a->%s, \"%s\"))", p->sym->name,xsi_type(p->info.typ));
	    } else if (is_wstring(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_wstring(soap, NULL, &a->%s, \"%s\"))", p->sym->name,xsi_type(p->info.typ));
	    */} else if(p->info.typ->type==Tarray) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_%s(soap, NULL, a->%s, \"%s\"))", c_ident(p->info.typ),p->sym->name,xsi_type(p->info.typ));
	    } else if(p->info.typ->type==Tclass && !is_external(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (a->%s.soap_in(soap, NULL, \"%s\"))", p->sym->name,xsi_type(p->info.typ));
	    } else if (p->info.typ->type != Tfun && !is_void(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_%s(soap, NULL, &a->%s, \"%s\"))", c_ident(p->info.typ),p->sym->name,xsi_type(p->info.typ));
	    }
	   }
	   else
	   {
	    if (is_XML(p->info.typ) && is_string(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_inliteral(soap, \"%s\", &a->%s))", ns_convert(p->sym->name), p->sym->name);
	    } else if (is_XML(p->info.typ) && is_wstring(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_inwliteral(soap, \"%s\", &a->%s))", ns_convert(p->sym->name), p->sym->name);
	    /*} else if (is_string(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_string(soap, \"%s\", &a->%s, \"%s\"))", ns_convert(p->sym->name),p->sym->name,xsi_type(p->info.typ));
	    } else if (is_wstring(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_wstring(soap, \"%s\", &a->%s, \"%s\"))", ns_convert(p->sym->name),p->sym->name,xsi_type(p->info.typ));
	    */} else if(p->info.typ->type==Tarray) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_%s(soap, \"%s\", a->%s, \"%s\"))", c_ident(p->info.typ),ns_convert(p->sym->name),p->sym->name,xsi_type(p->info.typ));
	    } else if(p->info.typ->type==Tclass && !is_external(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (a->%s.soap_in(soap, \"%s\", \"%s\"))", p->sym->name,ns_convert(p->sym->name),xsi_type(p->info.typ));
	    } else if (p->info.typ->type != Tfun && !is_void(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_%s(soap, \"%s\", &a->%s, \"%s\"))", c_ident(p->info.typ),ns_convert(p->sym->name),p->sym->name,xsi_type(p->info.typ));
	    }
	   }
	    if (!is_primclass(typ) && p->info.typ->type != Tfun && !is_void(p->info.typ))
	    { fprintf(fout,"\n\t\t\t\t{\tsoap_flag_%s = 0;", p->sym->name);
	      fprintf(fout,"\n\t\t\t\t\tcontinue;");
	      fprintf(fout,"\n\t\t\t\t}");
	    }
	fflush(fout);
	}
      }
      fprintf(fout,"\n\t\t\tif (soap->error == SOAP_TAG_MISMATCH)");
      fprintf(fout,"\n\t\t\t\tsoap->error = soap_ignore_element(soap);");
      fprintf(fout,"\n\t\t\tif (soap->error == SOAP_NO_TAG)");
      fprintf(fout,"\n\t\t\t\tbreak;");
      fprintf(fout,"\n\t\t\tif (soap->error)\n\t\t\t{\treturn NULL;\n\t\t\t}");
      fprintf(fout,"\n\t\t}");
      fprintf(fout,"\n\t\tif (soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
      fprintf(fout,"\n\t\t}");
      fprintf(fout,"\n\t}");
      fprintf(fout,"\n\telse\n\t{\ta = (%s)soap_id_forward(soap, soap->href, (void**)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0), SOAP_TYPE_%s, sizeof(%s));",c_type_id(typ, "*"), c_ident(typ), c_type(typ), c_ident(typ), c_type(typ));
      fprintf(fout,"\n\t\tif (soap->alloced)\n\t\t\tsoap_default_%s(soap, a);",c_ident(typ));
      fprintf(fout,"\n\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
      fprintf(fout,"\n\t}");
      }
      fprintf(fout, "\n\treturn a;\n}");
      break;
    
     case Tclass:

       /* CLASS ( Dynamic binding ) */
       /*       fprintf(fhead,"\nvirtual %s * %s::soap_in(char *);",
	      ((Table *) typ->ref)->sym->name ,c_type(typ),c_ident(typ));*/

      /* CLASS ( Static binding ) */
      /* Used on the receiving side if we are expecting a pointer to a Class
	 and memory has not been allocated for the Class ( that the pointer
	 points to ) before the get routine was called */ 
      /* Different from the dynamic binding method only until the object
	 pointed to by the pointer is allocated. */
       
      fprintf(fhead,"\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap*, const char*, %s, const char*);", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*"));
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nvoid *%s::soap_in(struct soap *soap, const char *tag, const char *type)", c_type(typ));
      fprintf(fout,"\n{\treturn soap_in_%s(soap, tag, this, type);\n}",c_ident(typ));
      fflush(fout);
      fprintf(fout,"\n\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap *soap, const char *tag, %s, const char *type)\n{", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*a"));
      /*fprintf(fout,"\n\tvoid *p = soap->alist;");
      fprintf(fout,"\n\tshort f;");
      */
      if (is_primclass(typ))
      {
        fprintf(fout, "\n\tif (soap_peek_element(soap))\n\t\treturn NULL;");
        fprintf(fout,"\n\tif (!(a = (%s)soap_class_id_enter(soap, soap->id, a, SOAP_TYPE_%s, soap->type, soap->arrayType)))\n\t{\tsoap->error = SOAP_TAG_MISMATCH;\n\t\treturn NULL;\n\t}", c_type_id(typ, "*"), c_ident(typ));
        fprintf(fout,"\n\t*soap->id = '\\0';");
        fprintf(fout,"\n\tif (soap->alloced)");
        fprintf(fout,"\n\t{\ta->soap_default(soap);",c_ident(typ));
        fprintf(fout,"\n\t\tif (soap->clist->type != SOAP_TYPE_%s)", c_ident(typ));
        fprintf(fout,"\n\t\t\treturn (%s)a->soap_in(soap, tag, type);", c_type_id(typ, "*"));
        fprintf(fout,"\n\t}");

          for (t = (Table*)typ->ref; t; t = t->prev)
          { for (p = t->list; p; p = p->next) 
	      if (p->info.sto & Sattribute)
                if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	          fprintf(fout, "\n\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\treturn NULL;", c_type(p->info.typ), ns_convert(p->sym->name), t->sym->name, p->sym->name);
                else if (p->info.typ->type == Tenum)
	          fprintf(fout, "\n\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\treturn NULL;", c_ident(p->info.typ), ns_convert(p->sym->name), t->sym->name, p->sym->name);
                else if (is_qname(p->info.typ))
	          fprintf(fout, "\n\tif (soap_s2QName(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\treturn NULL;", ns_convert(p->sym->name), t->sym->name, p->sym->name);
                else
	          fprintf(fout, "\n\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\treturn NULL;", the_type(p->info.typ), ns_convert(p->sym->name), t->sym->name, p->sym->name);
	  }
      fflush(fout);
	for (table = (Table*)typ->ref; table; table = table->prev)
	{ p = table->list;
	  if (p && !strcmp(p->sym->name, "__item"))
	    break;
        }
	    if (is_XML(p->info.typ) && is_string(p->info.typ))
	    { fprintf(fout,"\n\tif (!soap_inliteral(soap, tag, &(((%s*)a)->%s), \"%s\"))", table->sym->name,p->sym->name,xsi_type(typ));
	    }
	    else if (is_XML(p->info.typ) && is_wstring(p->info.typ))
	    { fprintf(fout,"\n\tif (!soap_inwliteral(soap, tag, &(((%s*)a)->%s), \"%s\"))", table->sym->name,p->sym->name,xsi_type(typ));
	    }
	    else if(p->info.typ->type==Tarray) {
	      fprintf(fout,"\n\tif (!soap_in_%s(soap, tag, ((%s*)a)->%s, \"%s\"))",
		      c_ident(p->info.typ),table->sym->name,p->sym->name,xsi_type(typ));
	    }
	    else if(p->info.typ->type==Tclass && !is_external(p->info.typ)) {
	      /*CLASS IN CLASS */
	      fprintf(fout,"\n\tif (!(((%s*)a)->%s).soap_in(soap, tag, \"%s\"))",
		     table->sym->name,p->sym->name,xsi_type(p->info.typ));
	    }
	    else if (p->info.typ->type != Tfun && !is_void(p->info.typ)) {
	      fprintf(fout,"\n\tif (!soap_in_%s(soap, tag, &(((%s*)a)->%s), \"%s\"))",
		      c_ident(p->info.typ),table->sym->name,p->sym->name,xsi_type(typ));
	    }
            /*fprintf(fout,"\n\t\tif (soap->error)\n\t\t\t{\tif (f)\n\t\t\t\t\tdelete a;\n\t\t\t\tsoap_dealloc(soap, * p);\n\t\t\t\treturn NULL;\n\t\t\t}");*/
           fprintf(fout,"\n\t\treturn NULL;");
      }
      else
      { fprintf(fout,"\n\tif (soap_element_begin_in(soap, tag))\n\t\treturn NULL;");
        fprintf(fout,"\n\tif (soap->null)\n\t\tif (soap->mode & SOAP_XML_NIL)");
        fprintf(fout,"\n\t\t{\tsoap->error = SOAP_NULL;");
        fprintf(fout,"\n\t\t\treturn NULL;");
        fprintf(fout,"\n\t\t}\telse");
        fprintf(fout,"\n\t\t\treturn a;");
        fprintf(fout,"\n\tif (!*soap->href)");
        fprintf(fout,"\n\t{\tif (!(a = (%s)soap_class_id_enter(soap, soap->id, a, SOAP_TYPE_%s, soap->type, soap->arrayType)))\n\t\t{\tsoap->error = SOAP_TAG_MISMATCH;\n\t\t\treturn NULL;\n\t\t}", c_type_id(typ, "*"), c_ident(typ));
        fprintf(fout,"\n\t\tif (soap->alloced)");
        fprintf(fout,"\n\t\t{\ta->soap_default(soap);",c_ident(typ));
        fprintf(fout,"\n\t\t\tif (soap->clist->type != SOAP_TYPE_%s)", c_ident(typ));
        fprintf(fout,"\n\t\t\t{\tsoap_revert(soap);");
        fprintf(fout,"\n\t\t\t\t*soap->id = '\\0';");
        fprintf(fout,"\n\t\t\t\treturn (%s)a->soap_in(soap, tag, type);", c_type_id(typ, "*"));
        fprintf(fout,"\n\t\t\t}\n\t\t}");
        table=(Table *)typ->ref;
        for (t = table; t; t = t->prev)
        { for (p = t->list; p; p = p->next) 
	    if (p->info.sto & Sattribute)
              if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	        fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\t\treturn NULL;", c_type(p->info.typ), ns_convert(p->sym->name), t->sym->name, p->sym->name);
              else if (p->info.typ->type == Tenum)
	        fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\t\treturn NULL;", c_ident(p->info.typ), ns_convert(p->sym->name), t->sym->name, p->sym->name);
              else if (is_qname(p->info.typ))
	          fprintf(fout, "\n\t\tif (soap_s2QName(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\t\treturn NULL;", ns_convert(p->sym->name), t->sym->name, p->sym->name);
              else
	        fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\t\treturn NULL;", the_type(p->info.typ), ns_convert(p->sym->name), t->sym->name, p->sym->name);
	}
        fflush(fout);
       
      i=0;
      /* Get the depth of the inheritance hierarchy */
      for (t = table; t; t = t->prev)
	i++;
      
      /* Declaring the flags to be used later */
      a=0;
      for(;i>0;i--){
	t = table;
	for(j=0;j<i-1;j++){
	  t = t->prev;
	}
	{for (p = t->list; p != (Entry*) 0; p = p->next)
	  { if (!(p->info.sto & Sconst) && !(p->info.sto & Sattribute) && p->info.typ->type != Tfun && !is_void(p->info.typ) && !is_transient(p->info.typ) && !is_repetition(p))
	    if(a==0)
	    { fprintf(fout,"\n\t\tshort soap_flag_%s%d = 1", p->sym->name, i );
	      a = 1;
            }
	    else
	      fprintf(fout,", soap_flag_%s%d = 1", p->sym->name, i );
	  }
        }
      }
      fprintf(fout,";"); 
      fflush(fout);
      fprintf(fout,"\n\t\tif (soap->body)\n\t\t{\tfor (;;)\n\t\t\t{");
        fprintf(fout,"\n\t\t\tsoap->error = SOAP_TAG_MISMATCH;\n"); 
	fflush(fout);
      table=(Table *)typ->ref;
      a=0;
      
      i=0;
      /* Get the depth of the inheritance hierarchy */
      for (t = table; t != (Table *) 0; t = t->prev) { 
	/*if(t==(Table *) 0 ) return;*/
	i++;
      }
      
      /* Call routines to output the member data of the class */
      /* Data members of the Base Classes are outputed first
	 followed by the data members of the Derived classes.
	 Over written data members are outputed twice once for the base class
	 they are defined in and once for the derived class that overwrites
	 them */
      
       
      for(;i>0;i--){
	t = table;
	for(j=0;j<i-1;j++){
	  t = t->prev;
	}
	
	for (p = t->list; p != (Entry*) 0; p = p->next)
	  if (p->info.sto & Sconst)
	    fprintf(fout, "\n\t\t\t/* const %s skipped */", p->sym->name);
	  else if (is_transient(p->info.typ))
	    fprintf(fout, "\n\t\t\t/* transient %s skipped */", p->sym->name);
	  else if (p->info.sto & Sattribute)
	    ;
	  else if (is_repetition(p))
	  { 
    fprintf(fout,"\n\t\t\tif (soap_flag_%s%d && soap->error == SOAP_TAG_MISMATCH)",p->next->sym->name,i);
    fprintf(fout,"\n\t\t\t{\t%s;\n\t\t\t\tsoap_new_block(soap);", c_type_id(p->next->info.typ, "p"));
    fprintf(fout,"\n\t\t\t\tfor (((%s*)a)->%s = 0; ; ((%s*)a)->%s++)", t->sym->name, p->sym->name, t->sym->name, p->sym->name);
    fprintf(fout,"\n\t\t\t\t{\tp = (%s)soap_push_block(soap, sizeof(%s));\n\t\t\t\t\tif (!p)\n\t\t\t\t\t\treturn NULL;", c_type(p->next->info.typ), c_type(p->next->info.typ->ref));
    if (((Tnode*)p->next->info.typ->ref)->type == Tclass && !is_external(p->next->info.typ->ref))
      fprintf(fout,"\n\t\t\t\t\tp->%s::soap_default(soap);", c_type(p->next->info.typ->ref));
    else
      fprintf(fout,"\n\t\t\t\t\tsoap_default_%s(soap, p);", c_ident(p->next->info.typ->ref));
    fprintf(fout,"\n\t\t\t\t\tif (!soap_in_%s(soap, \"%s\", p, \"%s\"))", c_ident(p->next->info.typ->ref), ns_overridden(t, p->next), xsi_type(p->next->info.typ->ref));
    fprintf(fout,"\n\t\t\t\t\t\tbreak;");
    fprintf(fout,"\n\t\t\t\t}");
    fprintf(fout,"\n\t\t\t\tsoap_pop_block(soap);");
    if (((Tnode*)p->next->info.typ->ref)->type == Tclass)
      fprintf(fout,"\n\t\t\t\tif (soap->blist->size)\n\t\t\t\t\t((%s*)a)->%s = soap_instantiate_%s(soap, soap->blist->size/sizeof(%s), NULL, NULL);\n\t\t\t\telse\n\t\t\t\t\t((%s*)a)->%s = NULL;", t->sym->name, p->next->sym->name, c_ident(p->next->info.typ->ref), c_type(p->next->info.typ->ref), t->sym->name, p->next->sym->name);
    else
      fprintf(fout,"\n\t\t\t\t((%s*)a)->%s = (%s)soap_malloc(soap, soap->blist->size);", t->sym->name, p->next->sym->name, c_type(p->next->info.typ));
    fprintf(fout,"\n\t\t\t\tsoap_store_block(soap, (char*)((%s*)a)->%s);", t->sym->name, p->next->sym->name);
    fprintf(fout,"\n\t\t\t\tsoap_flag_%s%d = 0;", p->next->sym->name,i);
    fprintf(fout,"\n\t\t\t\tif (soap->error == SOAP_TAG_MISMATCH)\n\t\t\t\t\tcontinue;\n\t\t\t}");
          p = p->next;
	  }
	  else
	  {
	    if (!is_primclass(typ) && p->info.typ->type != Tfun && !is_void(p->info.typ))
	      fprintf(fout,"\n\t\t\tif (soap_flag_%s%d && soap->error == SOAP_TAG_MISMATCH)",p->sym->name,i);
	   if (is_unmatched(p->sym))
	   { 
	    if (is_XML(p->info.typ) && is_string(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_inliteral(soap, NULL, &(((%s*)a)->%s))", t->sym->name, p->sym->name);
	    } else if (is_XML(p->info.typ) && is_wstring(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_inwliteral(soap, NULL, &(((%s*)a)->%s))", t->sym->name, p->sym->name);
	    }
	    /*else if (is_string(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_string(soap, NULL, &(((%s*)a)->%s), \"%s\"))", t->sym->name,p->sym->name,xsi_type(p->info.typ));
	    } else if (is_wstring(p->info.typ))
	    { fprintf(fout,"\n\t\t\t\tif (soap_in_wstring(soap, NULL, &(((%s*)a)->%s), \"%s\"))", t->sym->name,p->sym->name,xsi_type(p->info.typ));
	    }*/
	    else if(p->info.typ->type==Tarray) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_%s(soap, NULL, ((%s*)a)->%s, \"%s\"))",
		      c_ident(p->info.typ),t->sym->name,p->sym->name,xsi_type(p->info.typ));
	    } else if(p->info.typ->type==Tclass && !is_external(p->info.typ)) {
	      /*CLASS IN CLASS */
	      fprintf(fout,"\n\t\t\t\tif ((((%s*)a)->%s).soap_in(soap, NULL, \"%s\"))",
		     t->sym->name,p->sym->name,xsi_type(p->info.typ));
	    } else if (p->info.typ->type != Tfun && !is_void(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_%s(soap, NULL, &(((%s*)a)->%s), \"%s\"))",
		      c_ident(p->info.typ),t->sym->name,p->sym->name,xsi_type(p->info.typ));
	    }
           }
	   else
	   { 
	    if (is_XML(p->info.typ) && is_string(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_inliteral(soap, \"%s\", &(((%s*)a)->%s))", ns_overridden(t, p), t->sym->name,p->sym->name);
	    } else if (is_XML(p->info.typ) && is_wstring(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_inwliteral(soap, \"%s\", &(((%s*)a)->%s))", ns_overridden(t, p), t->sym->name,p->sym->name);
	    }/*
	    else if (is_string(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_string(soap, \"%s\", &(((%s*)a)->%s), \"%s\"))", ns_overridden(t, p),t->sym->name,p->sym->name,xsi_type(p->info.typ));
	    } else if (is_wstring(p->info.typ))
	    { fprintf(fout,"\n\t\t\t\tif (soap_in_wstring(soap, \"%s\", &(((%s*)a)->%s), \"%s\"))", ns_overridden(t, p),t->sym->name,p->sym->name,xsi_type(p->info.typ));
	    }
	    */
	    else if(p->info.typ->type==Tarray) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_%s(soap, \"%s\", ((%s*)a)->%s, \"%s\"))", c_ident(p->info.typ),ns_overridden(t, p),t->sym->name,p->sym->name,xsi_type(p->info.typ));
	    } else if(p->info.typ->type==Tclass && !is_external(p->info.typ)) {
	      /*CLASS IN CLASS */
	      fprintf(fout,"\n\t\t\t\tif ((((%s*)a)->%s).soap_in(soap, \"%s\", \"%s\"))", t->sym->name,p->sym->name,ns_overridden(t, p),xsi_type(p->info.typ));
	    } else if (p->info.typ->type != Tfun && !is_void(p->info.typ)) {
	      fprintf(fout,"\n\t\t\t\tif (soap_in_%s(soap, \"%s\", &(((%s*)a)->%s), \"%s\"))", c_ident(p->info.typ),ns_overridden(t, p),t->sym->name,p->sym->name,xsi_type(p->info.typ));
	    }
           }
	    a=1;
	    	    
	    if (!is_primclass(typ) && p->info.typ->type != Tfun && !is_void(p->info.typ))
	    { fprintf(fout,"\n\t\t\t\t{\tsoap_flag_%s%d = 0;", p->sym->name, i);
	      fprintf(fout,"\n\t\t\t\t\tcontinue;");
	      fprintf(fout,"\n\t\t\t\t}\n");
	    }
	fflush(fout);
	  }
	
      }
        fprintf(fout,"\n\t\t\tif (soap->error == SOAP_TAG_MISMATCH)");
        fprintf(fout,"\n\t\t\t\tsoap->error = soap_ignore_element(soap);");
        fprintf(fout,"\n\t\t\tif (soap->error == SOAP_NO_TAG)");
        fprintf(fout,"\n\t\t\t\tbreak;");
      fprintf(fout,"\n\t\t\tif (soap->error)\n\t\t\treturn NULL;");
      fprintf(fout,"\n\t\t}");
      fprintf(fout, "\n\t\tif (soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
      
      fprintf(fout,"\n\t\t}");
      fprintf(fout,"\n\t}\n\telse\n\t{\ta = (%s)soap_id_forward(soap, soap->href, (void**)soap_class_id_enter(soap, soap->id, a, SOAP_TYPE_%s, soap->type, soap->arrayType), SOAP_TYPE_%s, sizeof(%s));",c_type_id(typ, "*"), c_ident(typ), c_ident(typ), c_type(typ));
      fprintf(fout,"\n\t\tif (soap->alloced)\n\t\t\ta->soap_default(soap);",c_ident(typ));
      fprintf(fout, "\n\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
      fprintf(fout, "\n\t}");
      }
      fprintf(fout,"\n\treturn a;\n}");

      break;   
           
    case Tpointer:
      
      fprintf(fhead,"\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap*, const char*, %s, const char*);", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*"));
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap *soap, const char *tag, %s, const char *type)\n{", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*a"));
      if (is_primclass(typ))
      { if(((Tnode *) typ->ref)->type == Tclass && !is_external(typ->ref))
	{ fprintf(fout, "\n\ta = (%s)soap_id_enter(soap, \"\", a, SOAP_TYPE_%s, sizeof(%s), 0);", c_type_id(typ, "*"), c_ident(typ), c_type(typ));
	  fprintf(fout, "\n\tif (a)");
	  fprintf(fout, "\n\t{\tif (soap->alloced || !*a)");
	  fprintf(fout, "\n\t\t{\tif (soap_peek_element(soap))\n\t\t\t\treturn NULL;");
	  fprintf(fout, "\n\t\t\t*a = (%s)soap_instantiate_%s(soap, -1, soap->type, soap->arrayType);\n\t\t}", c_type(typ), c_ident(typ->ref));
	  fprintf(fout, "\n\t\tif (!*a)");
	  fprintf(fout, "\n\t\t\ta = NULL;");
	  fprintf(fout, "\n\t\telse");
	  fprintf(fout, "\n\t\t{\t(*a)->soap_default(soap);");
	  fprintf(fout, "\n\t\t\tif (!(*a)->soap_in(soap, tag, NULL))"); 
	  fprintf(fout, "\n\t\t\t\ta = NULL;");
	  fprintf(fout, "\n\t\t}");
	  fprintf(fout, "\n\t}");
	  fprintf(fout, "\n\treturn a;\n}");
	}
	else
        { fprintf(fout, "\n\ta = (%s)soap_id_enter(soap, \"\", a, SOAP_TYPE_%s, sizeof(%s), 1);", c_type_id(typ, "*"), c_ident(typ), c_type(typ));
	  fprintf(fout, "\n\tif (a)");
	  fprintf(fout, "\n\t\tif (!soap_in_%s(soap, tag, *a, NULL))\n\t\t\ta = NULL;", c_ident(typ->ref));
	  fprintf(fout, "\n\treturn a;\n}");
	}
        break;
      }
      if(((Tnode *) typ->ref)->type != Tclass || is_external(typ->ref))
        fprintf(fout,"\n\t%s;",c_type_id(typ, "p"));
      fprintf(fout,"\n\tif (soap_element_begin_in(soap, tag))");
      fprintf(fout,"\n\t\treturn NULL;");

      if(((Tnode *) typ->ref)->type == Tclass && !is_external(typ->ref)){
	fprintf(fout,"\n\tif (soap->null)");
	fprintf(fout,"\n\t{\ta = (%s)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), %d);", c_type_id(typ, "*"), c_ident(typ), c_type(typ), reflevel(typ));
        fprintf(fout,"\n\t\tif (a)");
	fprintf(fout,"\n\t\t\t*a = NULL;");
	fprintf(fout,"\n\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
	fprintf(fout,"\n\t}\n\telse if (!*soap->href)");
	/*fprintf(fout,"\n\t{    if(soap_match_tag(soap_type, \"%s\")==0)",soap_param(typ));
	fprintf(fout,"\n\t     {     a=(%s *)soap_id_enter(soap_id,a,sizeof(%s));",
		c_type(typ),c_type(typ));
	fprintf(fout,"\n\t           *a=NULL;");
	

	fprintf(fout,"\n\t     soap_peeked = 1;\n\t   soap_level--;");
	fprintf(fout,"\n\t           if(p->soap_in(tag,*a))",
		c_ident(typ->ref));
	fprintf(fout,"\n\t                *a=p;");
	fprintf(fout,"\n\t           else    a=NULL;");
	fprintf(fout,"\n\t           soap_element_end_in(tag);");

	fprintf(fout,"\n\t}\n\telse\n\t{");

	fprintf(fout,"\n\t     if((p = (%s *) soap_class_id_enter(soap_id, NULL,\"%s\",soap_type))==NULL)",
		c_type(typ->ref), c_type(typ->ref));
	fprintf(fout,"\n\t      return NULL;");
	*/
	fprintf(fout,"\n\t{\tsoap_revert(soap);");
	fprintf(fout, "\n\t\ta = (%s)soap_id_enter(soap, \"\", a, SOAP_TYPE_%s, sizeof(%s), 0);", c_type_id(typ, "*"), c_ident(typ), c_type(typ));
	fprintf(fout, "\n\t\tif (a)");
	fprintf(fout, "\n\t\t{\tif (soap->alloced || !*a)");
	fprintf(fout, "\n\t\t\t\t*a = (%s)soap_instantiate_%s(soap, -1, soap->type, soap->arrayType);", c_type(typ), c_ident(typ->ref));
	fprintf(fout, "\n\t\t\tif (!*a)");
	fprintf(fout, "\n\t\t\t\ta = NULL;");
	fprintf(fout, "\n\t\t\telse");
	fprintf(fout, "\n\t\t\t{\t(*a)->soap_default(soap);");
	fprintf(fout, "\n\t\t\t\tif (!(*a)->soap_in(soap, tag, NULL))"); 
	fprintf(fout, "\n\t\t\t\t\ta = NULL;");
	fprintf(fout, "\n\t\t\t}");
	fprintf(fout, "\n\t\t}");
	/*
	fprintf(fout,
		"\n\t{\tsoap_peeked = 1;\
		\n\t\tsoap_level--;\
		\n\t\ta = (%s)soap_id_enter(\"\", a, SOAP_TYPE_%s, sizeof(%s), 0);\
		\n\t\tif (a)\
		\n\t\t{\tif (soap_alloced || !*a)\
		\n\t\t\t\t*a = (%s)soap_instantiate_%s(-1, soap_type);\
		\n\t\t\tif (!*a)\
		\n\t\t\t\ta = NULL;\
		\n\t\t\telse\
		\n\t\t\t{\t(*a)->soap_default();\
		\n\t\t\t\tif (!(*a)->soap_in(tag, \"%s\"))\
		\n\t\t\t\t\ta = NULL;\
		\n\t\t\t}\
		\n\t\t}", c_type_id(typ, "*"), c_ident(typ), c_type(typ), c_type(typ), c_ident(typ->ref), xsi_type(typ->ref));
		*/
	/*
        fprintf(fout,"\n\t      if(soap_alloced)");
	fprintf(fout,"\n\t p->soap_default();",c_ident(typ->ref));
	fprintf(fout,"\n\t     if(p->soap_in(tag)==NULL)");
	  fprintf(fout,"\n\t      return NULL;");
	 
	fprintf(fout,"\n\t     if(a){");
	fprintf(fout,"\n\t           *a = p;\n\t }");
	fprintf(fout,"\n\t     else ");
	fprintf(fout,"\n\t       {a=(%s*)soap_id_enter(\"\",NULL,sizeof(%s));",
		c_type(typ),c_type(typ));
	fprintf(fout,"\n\t             *a=p;");
	fprintf(fout,"\n\t       }");

	
	fprintf(fout,"\n\t     if(a)\n\t       {if(p=soap_in_%s(tag,*a))",
		c_ident(typ->ref));
	fprintf(fout,"\n\t               *a = p;\n\t        else a=NULL;\n\t}");
	fprintf(fout,"\n\t       else   if(p=soap_in_%s(tag,NULL))",
		c_ident(typ->ref));
	fprintf(fout,"\n\t       {     a=(%s*)soap_id_enter(\"\",NULL,sizeof(%s));",
		c_type(typ),c_type(typ));
	fprintf(fout,"\n\t             *a=p;");
	fprintf(fout,"\n\t       }");*/
	
	fprintf(fout,"\n\t}\n\telse\n\t{\ta = (%s)soap_id_lookup(soap, soap->href, (void**)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), %d), SOAP_TYPE_%s, sizeof(%s), %d);", c_type_id(typ, "*"), c_ident(typ), c_type(typ), reflevel(typ), c_ident(typ->ref), c_type(typ->ref), reflevel(typ->ref) );
	fprintf(fout,"\n\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
	fprintf(fout,"\n\t}\n\treturn a;\n}");
      }
      else {
	fprintf(fout,"\n\tif (soap->null)");
	fprintf(fout,"\n\t{\ta = (%s)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), %d);", c_type_id(typ, "*"), c_ident(typ), c_type(typ),reflevel(typ));
        fprintf(fout,"\n\t\tif (a)");
	fprintf(fout,"\n\t\t\t*a = NULL;");
	fprintf(fout,"\n\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
	fprintf(fout,"\n\t}\n\telse if (!*soap->href)");
	/*fprintf(fout,"\n\t{    if(soap_match_tag(soap_type, \"%s\")==0) {",soap_param(typ));
	fprintf(fout,"\n\t          a=(%s *)soap_id_enter(soap_id,a,sizeof(%s));",
		c_type(typ),c_type(typ));
	fprintf(fout,"\n\t           *a=NULL;");
	
	fprintf(fout,"\n\t     soap_peeked = 1;\n\t   soap_level--;");
	fprintf(fout,"\n\t           if(p=soap_in_%s(tag,*a))",
		c_ident(typ->ref));
	fprintf(fout,"\n\t                *a=p;");
	fprintf(fout,"\n\t           else    a=NULL;");
	fprintf(fout,"\n\t           soap_element_end_in(tag);");
	fprintf(fout,"\n\t}\n\telse\n\t{");
	*/
	fprintf(fout,"\n\t{\tsoap_revert(soap);");
	fprintf(fout,"\n\t\tif (a)\n\t\t{\tif ((p = soap_in_%s(soap, tag, *a, type)))", c_ident(typ->ref));
	fprintf(fout,"\n\t\t\t\t*a = p;\n\t\t\telse\n\t\t\t\ta = NULL;\n\t\t}");

	fprintf(fout,"\n\t\telse if ((p = soap_in_%s(soap, tag, NULL, type)))",c_ident(typ->ref));
	fprintf(fout,"\n\t\t{\ta = (%s)soap_id_enter(soap, \"\", NULL, SOAP_TYPE_%s, sizeof(%s), 0);", c_type_id(typ, "*"), c_ident(typ), c_type(typ));
        fprintf(fout,"\n\t\t\tif (a)\n\t\t\t\t*a = p;");
	fprintf(fout,"\n\t\t}");
	fprintf(fout,"\n\t}\n\telse\n\t{\ta = (%s)soap_id_lookup(soap, soap->href, (void**)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), %d), SOAP_TYPE_%s, sizeof(%s), %d);", c_type_id(typ, "*"), c_ident(typ), c_type(typ), reflevel(typ), c_ident(typ->ref), c_type(typ->ref), reflevel(typ->ref) );
	fprintf(fout,"\n\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
	fprintf(fout,"\n\t}\n\treturn a;\n}");
      }
    
      break;
  
    case Tarray:
      temp = typ;
      while(temp->type == Tarray){
	temp = temp->ref;
      }
      fprintf(fhead,"\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap*, const char*, %s, const char*);",c_type_id(temp, "*"),c_ident(typ),c_type(typ));  
      if (is_external(typ))
        return;
      fprintf(fout,"\n\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap *soap, const char *tag, %s, const char *type)\n{",c_type_id(temp, "*"),c_ident(typ),c_type_id(typ, "a"));  
      fprintf(fout,"\n\tint i;");
      fprintf(fout,"\n\tif (soap_element_begin_in(soap, tag))");	
      fprintf(fout,"\n\t   return NULL;");
      fprintf(fout,"\n\tif (soap_match_array(soap, type))");
      fprintf(fout,"\n\t{     soap->error = SOAP_TYPE_MISMATCH;\n\tsoap_revert(soap);\n\t    return NULL;\n\t}");
      fprintf(fout,"\n\tif (soap->null)\n\t\tif (soap->mode & SOAP_XML_NIL)");
      fprintf(fout,"\n\t\t{\tsoap->error = SOAP_NULL;\n\t\t\t\treturn NULL;\n\t\t}\n\t\telse\n\t\t\treturn (%s)a;", c_type_id(temp, "*"));
      fprintf(fout,"\n\tif (soap->body && !*soap->href)");
      fprintf(fout,"\n\t{\ta = (%s)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0);", c_type_id(typ->ref, "(*)"), c_ident(typ), c_type(typ));
      fprintf(fout,"\n\t      if (!a)\n\t\treturn NULL;");
      fprintf(fout,"\n\t    if (soap->alloced)");
      fprintf(fout,"\n\t         soap_default_%s(soap, a);",c_ident(typ));
      total=get_dimension(typ);  
      n=typ->ref;
      cardinality = 1;
      while(n->type==Tarray)
	{
	  total=total*get_dimension(n);
	  n = n->ref;
	  cardinality++;
	}
      fprintf(fout,"\n\t\tfor (i = 0; i < %d; i++)",get_dimension(typ));
  fprintf(fout,"\n\t\t{\tsoap_peek_element(soap);\n\t\t\tif (soap->position)\n\t\t\t{\ti = soap->positions[0];\n\t\t\t\tif (i < 0 || i >= %d)\n\t\t\t\t{\tsoap->error = SOAP_IOB;\n\t\t\t\treturn NULL;\n\t\t\t\t}\n\t\t\t}", get_dimension(typ));
	fprintf(fout,"\n\t\t\tif (!soap_in_%s(soap, NULL, a", c_ident(typ->ref));

      if(cardinality > 1){
	fprintf(fout,"[i]");
      }else {
	fprintf(fout,"+i");
      }
      fprintf(fout,", \"%s\"))", xsi_type(typ->ref));
      fprintf(fout,"\n\t\t\t\tif (soap->error == SOAP_NO_TAG)");
      fprintf(fout,"\n\t\t\t\t{\tsoap->error = SOAP_OK;");
      fprintf(fout,"\n\t\t\t\t\tbreak;\n\t\t\t\t}");
      fprintf(fout,"\n\t\t\t\telse\n\t\t\t\t\treturn NULL;");
      fprintf(fout,"\n\t\t}");
      fprintf(fout,"\n\t\tif (soap->mode & SOAP_C_NOIOB)\n\t\t\twhile (soap_element_end_in(soap, tag) == SOAP_SYNTAX_ERROR)\n\t\t\t{\tsoap->peeked = 1;\n\t\t\t\tsoap_ignore_element(soap);\n\t\t\t}");
      fprintf(fout,"\n\t\telse if (soap_element_end_in(soap, tag))\n\t\t{\tif (soap->error == SOAP_SYNTAX_ERROR)\n\t\t\t\tsoap->error = SOAP_IOB;\n\t\t\treturn NULL;\n\t\t}");
      fprintf(fout,"\n\t}\n\telse\n\t{");
      fprintf(fout,"\n\t\ta = (%s)soap_id_forward(soap, soap->href, (void**)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0), SOAP_TYPE_%s, sizeof(%s));", c_type_id(typ->ref, "(*)"), c_ident(typ), c_type(typ), c_ident(typ), c_type(typ));
      fprintf(fout,"\n\t\tif (soap->alloced)");
      fprintf(fout,"\n\t\t\tsoap_default_%s(soap, a);",c_ident(typ));
      fprintf(fout,"\n\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
      fprintf(fout,"\n\t}\n\treturn (%s)a", c_type_id(temp, "*"));
      /*
      for(i=1;i<=cardinality;i++){
          fprintf(fout,"[0]");
      }	      
      */
      fprintf(fout,";\n}");
      break;

    case Tenum:
      fprintf(fhead,"\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap*, const char*, %s, const char*);",c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*"));  
      if (is_external(typ))
        return;
      if (!is_mask(typ))
      { fprintf(fhead,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_s2%s(struct soap*, const char*, %s);",c_ident(typ),c_type_id(typ, "*"));  
        fprintf(fout,"\n\nSOAP_FMAC1 int SOAP_FMAC2 soap_s2%s(struct soap *soap, const char *s, %s)\n{\n\t",c_ident(typ),c_type_id(typ, "*a"));  
        for (t = (Table*)typ->ref; t; t = t->prev)
        { for (p = t->list; p; p = p->next)
	    fprintf(fout, "if (!strcmp(s, \"%s\"))\n\t\t*a = %s;\n\telse ", ns_convert(p->sym->name), p->sym->name);
        }	
        fprintf(fout, "\n\t{\tchar *r;\n\t\t*a = (%s)strtol(s, &r, 10);\n\t\tif (*r)\n\t\t\treturn soap->error = SOAP_TYPE_MISMATCH;\n\t}\n\treturn SOAP_OK;\n}", c_type(typ));
      }
      fprintf(fout,"\n\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap *soap, const char *tag, %s, const char *type)\n{",c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*a"));  
      if (is_mask(typ))
        fprintf(fout,"\n\tchar *s;\n\tLONG64 i;");
      fprintf(fout,"\n\tif (soap_element_begin_in(soap, tag))");	
      fprintf(fout,"\n\t\treturn NULL;");
      fprintf(fout,"\n\tif (*soap->type && soap_match_tag(soap, soap->type, type))");
      fprintf(fout,"\n\t{\tsoap->error = SOAP_TYPE_MISMATCH;\n\tsoap_revert(soap);\n\t\treturn NULL;\n\t}");
      fprintf(fout,"\n\tif (soap->null)\n\t{\tif (soap->mode & SOAP_XML_NIL)");
      fprintf(fout,"\n\t\t{\tsoap->error = SOAP_NULL;\n\t\t\treturn NULL;\n\t\t}\n\t\telse\n\t\t\treturn a;\n\t}");
      fprintf(fout,"\n\tif (soap->body && !*soap->href)");
      fprintf(fout,"\n\t{\ta = (%s)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0);", c_type_id(typ, "*"), c_ident(typ), c_type(typ));
      if (is_mask(typ))
      { fprintf(fout,"\n\t\tif (!a)\n\t\t\treturn NULL;");
        fprintf(fout,"\n\t\ti = 0;\n\t\twhile (*(s = soap_value(soap)))\n\t\t\t");
        for (t = (Table*)typ->ref; t; t = t->prev)
        { for (p = t->list; p; p = p->next)
	    fprintf(fout, "if (!strcmp(s, \"%s\"))\n\t\t\t\ti |= (LONG64)%s;\n\t\t\telse ", ns_convert(p->sym->name), p->sym->name);
        }	
        fprintf(fout, "\n\t\t\t{\tsoap->error = SOAP_TYPE_MISMATCH;\n\t\t\t\treturn NULL;\n\t\t\t}");
        fprintf(fout, "\n\t\t*a = (%s)i;", c_type(typ));
      }
      else
        fprintf(fout,"\n\t\tif (!a || soap_s2%s(soap, soap_value(soap), a))\n\t\t\treturn NULL;", c_ident(typ));
      fprintf(fout, "\n\t\tif (soap_element_end_in(soap, tag))\n\t\t\treturn NULL;", c_type(typ));
      fprintf(fout, "\n\t}\n\telse\n\t{\ta = (%s)soap_id_forward(soap, soap->href, (void**)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0), SOAP_TYPE_%s, sizeof(%s));", c_type_id(typ, "*"), c_ident(typ), c_type(typ), c_ident(typ), c_type(typ));
      fprintf(fout,"\n\t\tif (soap->alloced)\n\t\t\tsoap_default_%s(soap, a);",c_ident(typ));
      fprintf(fout, "\n\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
      fprintf(fout,"\n\t}\n\treturn a;\n}");
      break;

    default: break;
    }
  fflush(fout);
}


void
soap_in_Darray(Tnode *typ)
{ int i, j, d;
  Entry *p, *q;
  Table *t, *table;

  table=(Table *)typ->ref;
  q = table->list;
  p = is_dynamic_array(typ);
  d = get_Darraydims(typ);
  
  fprintf(fhead,"\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap*, const char*, %s, const char*);", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*"));
      if (is_external(typ))
        return;
  if (typ->type == Tclass)
  { fprintf(fout,"\n\nvoid *%s::soap_in(struct soap *soap, const char *tag, const char *type)", c_type(typ));
    fprintf(fout,"\n{\treturn soap_in_%s(soap, tag, this, type);\n}", c_ident(typ));
  }
  fflush(fout);
  fprintf(fout,"\n\nSOAP_FMAC1 %s SOAP_FMAC2 soap_in_%s(struct soap *soap, const char *tag, %s, const char *type)", c_type_id(typ, "*"),c_ident(typ),c_type_id(typ, "*a"));
  if (is_hexBinary(typ))
    fprintf(fout,"\n{\tint i, n;\n\tchar *s;");
  else if (is_binary(typ))
    fprintf(fout,"\n{");
  else if (d)
    fprintf(fout,"\n{\tint i, j, n;\n\t%s;", c_type_id(p->info.typ, "p"));
  else
    fprintf(fout,"\n{\tint i, j;\n\t%s;", c_type_id(p->info.typ, "p"));
  fprintf(fout,"\n\tif (soap_element_begin_in(soap, tag))\n\t\treturn NULL;");
  if (has_ns(typ) || is_untyped(typ))
    if (is_hexBinary(typ))
      fprintf(fout,"\n\tif (*soap->type && soap_match_tag(soap, soap->type, type))");
    else if (is_binary(typ))
      fprintf(fout,"\n\tif (*soap->type && soap_match_tag(soap, soap->type, type) && soap_match_tag(soap, soap->type, \"SOAP-ENC:base64\") && soap_match_tag(soap, soap->type, \"SOAP-ENC:base64Binary\"))");
    else
      fprintf(fout,"\n\tif (*soap->type && soap_match_tag(soap, soap->type, type))");
  else
    fprintf(fout,"\n\tif (soap_match_array(soap, type))");
  fprintf(fout,"\n\t{\tsoap->error = SOAP_TYPE_MISMATCH;\n\t\tsoap_revert(soap);\n\t\treturn NULL;\n\t}");
  fprintf(fout,"\n\tif (soap->null)");
  if (typ->type == Tclass)
  { fprintf(fout,"\n\t{\tif ((a = (%s)soap_class_id_enter(soap, soap->id, a, SOAP_TYPE_%s, soap->type, soap->arrayType)))",c_type_id(typ, "*"), c_ident(typ)); 
    fprintf(fout,"\n\t\t\ta->soap_default(soap);");
    fprintf(fout,"\n\t\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\t\treturn NULL;");
    fprintf(fout,"\n\t}\n\telse if (!*soap->href)");
    fprintf(fout,"\n\t{\tif (!(a = (%s)soap_class_id_enter(soap, soap->id, a, SOAP_TYPE_%s, soap->type, soap->arrayType)))",c_type_id(typ, "*"), c_ident(typ)); 
  }
  else
  { fprintf(fout,"\n\t{\tif ((a = (%s)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0)))",c_type_id(typ, "*"), c_ident(typ), c_type(typ));
    fprintf(fout,"\n\t\t\tsoap_default_%s(soap, a);", c_ident(typ));
    fprintf(fout,"\n\t\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\t\t\treturn NULL;");
    fprintf(fout,"\n\t}\n\telse if (!*soap->href)");
    fprintf(fout,"\n\t{\tif (!(a = (%s)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0)))",c_type_id(typ, "*"), c_ident(typ), c_type(typ));
  }
  fprintf(fout,"\n\t\t\treturn NULL;");
  fprintf(fout,"\n\t\tif (soap->alloced)");
  if (typ->type == Tclass)
  { fprintf(fout,"\n\t\t\ta->soap_default(soap);");
    for (t = (Table*)typ->ref; t; t = t->prev)
    { for (p = t->list; p; p = p->next) 
	      if (p->info.sto & Sattribute)
                if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\t\treturn NULL;", c_type(p->info.typ), ns_convert(p->sym->name), t->sym->name, p->sym->name);
                else if (p->info.typ->type == Tenum)
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\t\treturn NULL;", c_ident(p->info.typ), ns_convert(p->sym->name), t->sym->name, p->sym->name);
                else if (is_qname(p->info.typ))
	          fprintf(fout, "\n\t\tif (soap_s2QName(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\t\treturn NULL;", ns_convert(p->sym->name), t->sym->name, p->sym->name);
                else
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &(((%s*)a)->%s)))\n\t\t\treturn NULL;", the_type(p->info.typ), ns_convert(p->sym->name), t->sym->name, p->sym->name);
    }
  }
  else
  { fprintf(fout,"\n\t\t\tsoap_default_%s(soap, a);",c_ident(typ));
    for (t = (Table*)typ->ref; t; t = t->prev)
    { for (p = t->list; p; p = p->next) 
	      if (p->info.sto & Sattribute)
                if (p->info.typ->type == Tllong || p->info.typ->type == Tullong)
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", c_type(p->info.typ), ns_convert(p->sym->name), p->sym->name);
                else if (p->info.typ->type == Tenum)
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", c_ident(p->info.typ), ns_convert(p->sym->name), p->sym->name);
                else if (is_qname(p->info.typ))
	          fprintf(fout, "\n\t\tif (soap_s2QName(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", ns_convert(p->sym->name), p->sym->name);
                else
	          fprintf(fout, "\n\t\tif (soap_s2%s(soap, soap_attr_value(soap, \"%s\"), &a->%s))\n\t\t\treturn NULL;", the_type(p->info.typ), ns_convert(p->sym->name), p->sym->name);
    }
  }
  fprintf(fout,"\n\t\tif (soap->body)\n\t\t{");
  p = is_dynamic_array(typ);
  if (is_hexBinary(typ))
  { fprintf(fout,"\n\t\tsoap_new_block(soap);");
    fprintf(fout,"\n\t\tdo\n\t\t{\tif (!(s = (char*)soap_push_block(soap, SOAP_BLKLEN)))\n\t\t\t\treturn NULL;");
    fprintf(fout,"\n\t\t\tfor (i = 0; i < SOAP_BLKLEN; i++)\n\t\t\t{\tif ((n = soap_gethex(soap)) < 0)\n\t\t\t\t\tbreak;\n\t\t\t\ts[i] = (char)n;\n\t\t\t}");
    fprintf(fout,"\n\t\t}\n\t\twhile (i >= SOAP_BLKLEN);");
    fprintf(fout,"\n\t\tsoap_size_block(soap, i);");
    fprintf(fout,"\n\t\ta->__size = soap->blist->size;");
    if (mflag && typ->type == Tclass)
      fprintf(fout,"\n\t\tif (soap->blist->size)\n\t\t\ta->__ptr = (%s)malloc(soap->blist->size);\n\t\telse\n\t\t\ta->__ptr = NULL;", c_type(p->info.typ));
    else
      fprintf(fout,"\n\t\ta->__ptr = (unsigned char*)soap_malloc(soap, soap->blist->size);");
    fprintf(fout,"\n\t\tsoap_store_block(soap, (char*)a->__ptr);");
    fprintf(fout,"\n\t\tif (soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
  }
  else if (is_binary(typ))
  { if (mflag && typ->type == Tclass)
      fprintf(fout,"\n\t\ta->__ptr = soap_getbase64(soap, (size_t*)&a->__size, 1);");
    else
      fprintf(fout,"\n\t\ta->__ptr = soap_getbase64(soap, (size_t*)&a->__size, 0);");
    fprintf(fout,"\n\t\tif (soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
  }
  else
  { if (d)
    { fprintf(fout,"\n\t\tn = soap_getsizes(soap->arraySize, a->__size, %d);", d);
      if (has_offset(typ))
        fprintf(fout,"\n\t\tn -= j = soap_getoffsets(soap->arrayOffset, a->__size, a->__offset, %d);", d);
      else
        fprintf(fout,"\n\t\tn -= j = soap_getoffsets(soap->arrayOffset, a->__size, NULL, %d);", d);
      fprintf(fout,"\n\t\tif (j >= 0 && n >= 0)");
      if (((Tnode*)p->info.typ->ref)->type == Tclass)
      { fprintf(fout,"\n\t\t{\ta->%s = soap_instantiate_%s(soap, n, NULL, NULL);", p->sym->name, c_ident(p->info.typ->ref));
        if (!is_external(p->info.typ->ref))
          fprintf(fout, "\n\t\t\tfor (i = 0; i < n; i++)\n\t\t\t\t(a->%s+i)->%s::soap_default(soap);", p->sym->name, c_type(p->info.typ->ref));
        else
          fprintf(fout, "\n\t\t\tfor (i = 0; i < n; i++)\n\t\t\t\tsoap_default_%s(soap, a->%s+i);", c_ident(p->info.typ->ref), p->sym->name);
      }
      else
        { fprintf(fout,"\n\t\t{\ta->%s = (%s)soap_malloc(soap, n*sizeof(%s));", p->sym->name, c_type_id(p->info.typ->ref, "*"),  c_type(p->info.typ->ref));
        fprintf(fout, "\n\t\t\tfor (i = 0; i < n; i++)\n\t\t\t\tsoap_default_%s(soap, a->%s+i);", c_ident(p->info.typ->ref), p->sym->name);
      }
      if (has_offset(typ))
        fprintf(fout,"\n\t\t\tfor (i = 0; i < n; i++)");
      else
        fprintf(fout,"\n\t\t\tfor (i = j; i < j+n; i++)");
      fprintf(fout,"\n\t\t\t{\tsoap_peek_element(soap);\n\t\t\t\tif (soap->position == %d)", d);
      fprintf(fout,"\n\t\t\t\t{\ti = ");
	for (i = 0; i < d; i++)
	{ fprintf(fout,"soap->positions[%d]", i);
	  for (j = 1; j < d-i; j++)
	    fprintf(fout,"*a->__size[%d]", j);
	  if (i < d-1)
	    fprintf(fout,"+");
	}
	fprintf(fout,"-j;");
	fprintf(fout,"\n\t\t\t\t\tif (i < 0 || i >= n)\n\t\t\t\t\t{\tsoap->error = SOAP_IOB;\n\t\t\t\t\t\treturn NULL;\n\t\t\t\t\t}\n\t\t\t\t}");
        fprintf(fout,"\n\t\t\t\tif (!soap_in_%s(soap, NULL, a->%s+i, \"%s\"))", c_ident(p->info.typ->ref), p->sym->name, xsi_type(p->info.typ->ref));
      fprintf(fout,"\n\t\t\t\t{\tif (soap->error == SOAP_NO_TAG)");
      fprintf(fout,"\n\t\t\t\t\t{\tsoap->error = SOAP_OK;");
      fprintf(fout,"\n\t\t\t\t\t\tbreak;");
      fprintf(fout,"\n\t\t\t\t\t}\n\t\t\t\t\telse\n\t\t\t\t\t\treturn NULL;\n\t\t\t\t}");
    }
    else
    { fprintf(fout,"\n\t\ta->__size = soap_getsize(soap->arraySize, soap->arrayOffset, &j);");
      if (has_offset(typ) && (p->next->next->info.sto & Sconst) == 0)
      { fprintf(fout,"\n\t\ta->__offset = j;");
      }
      fprintf(fout,"\n\t\tif (j >= 0 && a->__size >= 0)");
      if (((Tnode*)p->info.typ->ref)->type == Tclass)
      { fprintf(fout,"\n\t\t{\ta->%s = soap_instantiate_%s(soap, a->__size, NULL, NULL);", p->sym->name, c_ident(p->info.typ->ref));
        if (!is_external(p->info.typ->ref))
          fprintf(fout, "\n\t\t\tfor (i = 0; i < a->__size; i++)\n\t\t\t\t(a->%s+i)->%s::soap_default(soap);", p->sym->name, c_type(p->info.typ->ref));
        else
          fprintf(fout, "\n\t\t\tfor (i = 0; i < a->__size; i++)\n\t\t\t\tsoap_default_%s(soap, a->%s+i);", c_ident(p->info.typ->ref), p->sym->name);
      }
      else
        {
	if (mflag && typ->type == Tclass)
          fprintf(fout,"\n\t\t{\ta->%s = (%s)malloc(sizeof(%s) * a->__size);", p->sym->name, c_type_id(p->info.typ->ref, "*"),  c_type(p->info.typ->ref));
        else
          fprintf(fout,"\n\t\t{\ta->%s = (%s)soap_malloc(soap, sizeof(%s) * a->__size);", p->sym->name, c_type_id(p->info.typ->ref, "*"),  c_type(p->info.typ->ref));
        fprintf(fout, "\n\t\t\tfor (i = 0; i < a->__size; i++)\n\t\t\t\tsoap_default_%s(soap, a->%s+i);", c_ident(p->info.typ->ref), p->sym->name);
      }
      if (has_offset(typ))
        fprintf(fout,"\n\t\t\tfor (i = 0; i < a->__size; i++)");
      else
        fprintf(fout,"\n\t\t\tfor (i = j; i < j+a->__size; i++)");
      fprintf(fout,"\n\t\t\t{\tsoap_peek_element(soap);\n\t\t\t\tif (soap->position)\n\t\t\t\t{\ti = soap->positions[0]-j;\n\t\t\t\t\tif (i < 0 || i >= a->__size)\n\t\t\t\t\t{\tsoap->error = SOAP_IOB;\n\t\t\t\t\treturn NULL;\n\t\t\t\t\t}\n\t\t\t\t}");
        fprintf(fout,"\n\t\t\t\tif (!soap_in_%s(soap, NULL, a->%s+i, \"%s\"))", c_ident(p->info.typ->ref), p->sym->name, xsi_type(p->info.typ->ref));
      fprintf(fout,"\n\t\t\t\t{\tif (soap->error == SOAP_NO_TAG)");
      fprintf(fout,"\n\t\t\t\t\t{\tsoap->error = SOAP_OK;");
      fprintf(fout,"\n\t\t\t\t\t\tbreak;");
      fprintf(fout,"\n\t\t\t\t\t}\n\t\t\t\t\telse\n\t\t\t\t\t\treturn NULL;\n\t\t\t\t}");
    }
    fprintf(fout,"\n\t\t\t}\n\t\t}\n\t\telse");
    fprintf(fout,"\n\t\t{\tsoap_new_block(soap);");
    if (d)
      fprintf(fout,"\n\t\t\tfor (a->__size[0] = 0; ; a->__size[0]++)");
    else
      fprintf(fout,"\n\t\t\tfor (a->__size = 0; ; a->__size++)");
    fprintf(fout,"\n\t\t\t{\tp = (%s)soap_push_block(soap, sizeof(%s));\n\t\t\t\tif (!p)\n\t\t\t\t\treturn NULL;", c_type(p->info.typ), c_type(p->info.typ->ref));
    if (((Tnode*)p->info.typ->ref)->type == Tclass && !is_external(p->info.typ->ref))
      fprintf(fout,"\n\t\t\t\tp->%s::soap_default(soap);", c_type(p->info.typ->ref));
    else
      fprintf(fout,"\n\t\t\t\tsoap_default_%s(soap, p);", c_ident(p->info.typ->ref));
      fprintf(fout,"\n\t\t\t\tif (!soap_in_%s(soap, NULL, p, \"%s\"))", c_ident(p->info.typ->ref), xsi_type(p->info.typ->ref));
    fprintf(fout,"\n\t\t\t\t{\tif (soap->error == SOAP_NO_TAG)");
    fprintf(fout,"\n\t\t\t\t\t{\tsoap->error = SOAP_OK;");
    fprintf(fout,"\n\t\t\t\t\t\tbreak;");
    fprintf(fout,"\n\t\t\t\t\t}\n\t\t\t\t\telse\n\t\t\t\t\t\treturn NULL;\n\t\t\t\t}");
    fprintf(fout,"\n\t\t\t}");
    fprintf(fout,"\n\t\t\tsoap_pop_block(soap);");
    if (((Tnode*)p->info.typ->ref)->type == Tclass)
      fprintf(fout,"\n\t\t\tif (soap->blist->size)\n\t\t\t\ta->%s = soap_instantiate_%s(soap, soap->blist->size/sizeof(%s), NULL, NULL);\n\t\t\telse\n\t\t\t\ta->%s = NULL;", p->sym->name, c_ident(p->info.typ->ref), c_type(p->info.typ->ref), p->sym->name);
    else
    if (mflag && typ->type == Tclass)
      fprintf(fout,"\n\t\t\tif (soap->blist->size)\n\t\t\t\ta->%s = (%s)malloc(soap->blist->size);\n\t\t\telse\n\t\t\t\ta->%s = NULL;", p->sym->name, c_type(p->info.typ), p->sym->name);
    else
      fprintf(fout,"\n\t\t\ta->%s = (%s)soap_malloc(soap, soap->blist->size);", p->sym->name, c_type(p->info.typ));
    fprintf(fout,"\n\t\t\tsoap_store_block(soap, (char*)a->%s);", p->sym->name);
    fprintf(fout,"\n\t\t}");
    fprintf(fout,"\n\t\tif (soap_element_end_in(soap, tag))\n\t\t\treturn NULL;");
  }
  fprintf(fout,"\n\t\t}");
  if (typ->type == Tclass)
  { fprintf(fout,"\n\t}\n\telse\n\t{\ta = (%s)soap_id_forward(soap, soap->href, (void**)soap_class_id_enter(soap, soap->id, a, SOAP_TYPE_%s, soap->type, soap->arrayType), SOAP_TYPE_%s, sizeof(%s));", c_type_id(typ, "*"), c_ident(typ), /*xsi_type(typ),*/ c_ident(typ), c_type(typ));
    fprintf(fout,"\n\t\tif (soap->alloced)\n\t\t\ta->soap_default(soap);");
  }
  else
  { fprintf(fout,"\n\t}\n\telse\n\t{\ta = (%s)soap_id_forward(soap, soap->href, (void**)soap_id_enter(soap, soap->id, a, SOAP_TYPE_%s, sizeof(%s), 0), SOAP_TYPE_%s, sizeof(%s));", c_type_id(typ, "*"), c_ident(typ), c_type(typ), c_ident(typ), c_type(typ));
    fprintf(fout,"\n\t\tif (soap->alloced)\n\t\t\tsoap_default_%s(soap, a);",c_ident(typ));
  }
  fprintf(fout,"\n\t\tif (soap->body && soap_element_end_in(soap, tag))\n\t\treturn NULL;");
  fprintf(fout,"\n\t}\n\treturn a;\n}");
}

