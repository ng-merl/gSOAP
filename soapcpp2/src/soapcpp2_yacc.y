/*	soapcpp2_yacc.y

	Yacc/Bison grammar

Note:	Bison 1.6 can crash on Win32 systems if YYINITDEPTH is too small
	Compile with -DYYINITDEPTH=1000

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

%{

#include "soapcpp2.h"
#ifdef WIN32
extern int soapcpp2lex();
#endif

#define MAXNEST 8	/* max. nesting depth of scopes */

struct Scope
{	Table	*table;
	Entry	*entry;
	Node	node;
	LONG64	val;
	int	offset;
	Bool	grow;	/* true if offset grows with declarations */
	Bool	mask;	/* true if enum is mask */
}	stack[MAXNEST],	/* stack of tables and offsets */
	*sp;		/* current scope stack pointer */

Table	*classtable = (Table*)0,
	*uniontable = (Table*)0,
	*enumtable = (Table*)0,
	*typetable = (Table*)0,
	*booltable = (Table*)0;

Symbol	*namespaceid = NULL;
int	transient = 0;
int	custom_header = 1;
int	custom_fault = 1;
Pragma	*pragmas = NULL;

/* function prototypes for support routine section */
static Entry	*undefined(Symbol*);
static Tnode	*mgtype(Tnode*, Tnode*);
static Node	op(const char*, Node, Node), iop(const char*, Node, Node), relop(const char*, Node, Node);
static void	mkscope(Table*, int), enterscope(Table*, int), exitscope();
static int	integer(Tnode*), real(Tnode*), numeric(Tnode*), pointer(Tnode*);
static void	add_header(Table*), add_fault(Table*), add_response(Entry*, Entry*);
extern char	*c_storage(Storage), *c_type(Tnode*);
extern int	is_primitive_or_string(Tnode*);

/* temporaries used in semantic rules */
int	i;
char	*s, *s1, *s2;
Symbol	*sym;
Entry	*p, *q;
Tnode	*t;
Node	tmp, c;
Pragma	**pp;

%}

%union
{	Symbol	*sym;
	LONG64	i;
	double	r;
	char	c;
	char	*s;
	Tnode	*typ;
	Storage	sto;
	Node	rec;
	Entry	*e;
}

/* pragmas */
%token	<s> PRAGMA
/* keywords */
%token	<sym> AUTO     DOUBLE  INT       STRUCT
%token	<sym> BREAK    ELSE    LONG      SWITCH
%token	<sym> CASE     ENUM    REGISTER  TYPEDEF
%token	<sym> CHAR     EXTERN  RETURN    UNION
%token	<sym> CONST    FLOAT   SHORT     UNSIGNED
%token	<sym> CONTINUE FOR     SIGNED    VOID
%token	<sym> DEFAULT  GOTO    SIZEOF    VOLATILE
%token	<sym> DO       IF      STATIC    WHILE
%token	<sym> CLASS    PRIVATE PROTECTED PUBLIC
%token	<sym> VIRTUAL  INLINE  OPERATOR  LLONG
%token	<sym> BOOL     CFALSE  CTRUE	 WCHAR
%token	<sym> TIME     USING   NAMESPACE ULLONG
%token	<sym> MUSTUNDERSTAND   SIZE      FRIEND
/* */
%token	NONE
/* identifiers (TYPE = typedef identifier) */
%token	<sym> ID TYPE
/* constants */
%token	<i> LNG
%token	<r> DBL
%token	<c> CHR
%token	<s> STR
/* types and related */
%type	<typ> type
%type	<sto> store virtual constobj abstract
%type	<e> fname class super
%type	<sym> arg name
/* expressions and statements */
%type	<rec> expr cexp oexp obex aexp abex rexp lexp pexp init spec tspec ptrs array texp qexp occurs
/* terminals */
%left	','
%right	'=' PA NA TA DA MA AA XA OA LA RA  /* += -= *= /= %= &= ^= |= <<= >>= */
%right	'?'
%right	':'
%left	OR		/* || */
%left	AN		/* && */
%left	'|'
%left	'^'
%left	'&'
%left	EQ NE		/* == != */
%left	'<' LE '>' GE	/* <= >= */
%left	LS RS		/* << >> */
%left	'+' '-'
%left	'*' '/' '%'
%left	AR		/* -> */
%token	PP NN		/* ++ -- */

%%

/******************************************************************************\

	Program syntax

\******************************************************************************/

prog	: s1 exts	{ add_header(sp->table);
			  add_fault(sp->table);
			  compile(sp->table);
			  freetable(classtable);
			  freetable(uniontable);
			  freetable(enumtable);
			  freetable(typetable);
			  freetable(booltable);
			}
	;
s1	: /* empty */	{ classtable = mktable((Table*)0);
			  uniontable = mktable((Table*)0);
			  enumtable = mktable((Table*)0);
			  typetable = mktable((Table*)0);
			  booltable = mktable((Table*)0);
			  p = enter(booltable, lookup("false"));
			  p->info.typ = mkint();
			  p->info.val.i = 0;
			  p = enter(booltable, lookup("true"));
			  p->info.typ = mkint();
			  p->info.val.i = 1;
			  mkscope(mktable(mktable((Table*)0)), 0);
			}
	;
exts	: NAMESPACE ID '{' exts1 '}'	/* for future use */
			{ namespaceid = $2; }
	| exts1		{ namespaceid = NULL; }
	;
exts1	: /* empty */	{ }
	| exts1 ext	{ }
	;
ext	: dclrs ';'	{ }
	| pragma	{ }
	| error ';'	{ synerror("input before ; skipped");
			  while (sp > stack)
			  {	freetable(sp->table);
			  	exitscope();
			  }
			  yyerrok;
			}
	| t1		{ }
	| t2		{ }
	;
pragma	: PRAGMA	{ if (iflag && $1[1] >= 'a' && $1[1] <= 'z')
			  {	for (pp = &pragmas; *pp; pp = &(*pp)->next)
			          ;
				*pp = (Pragma*)emalloc(sizeof(Pragma));
				(*pp)->pragma = (char*)emalloc(strlen($1)+1);
				strcpy((*pp)->pragma, $1);
				(*pp)->next = NULL;
			  }
			  else if ((i = atoi($1+2)) > 0)
				yylineno = i;
			  else
			  {	sprintf(errbuf, "directive '%s' ignored (use #import to import files and/or use option -i)", $1);
			  	semwarn(errbuf);
			  }
			}
	;

/******************************************************************************\

	Declarations

\******************************************************************************/

decls	: /* empty */	{ }
	| dclrs ';' decls
			{ }
	| PRIVATE ':' decls
			{ }
	| PROTECTED ':' decls
			{ }
	| PUBLIC ':' decls
			{ }
	| t1 decls t2 decls
			{ }
	;
t1	: '['		{ transient++; }
	;
t2	: ']'		{ transient--;
			  if (transient < 0)
			  {	semwarn("Too many ']'");
				transient = 0;
			  }
			}
	;
dclrs	: spec		{ }
	| spec dclr	{ }
	| spec fdclr func
			{ }
	| constr func	{ }
	| destr func	{ }
	| dclrs ',' dclr{ }
	| dclrs ',' fdclr func
			{ }
	;
dclr	: ptrs ID array occurs init
			{ if ($3.sto & Stypedef)
			  {	p = enter(typetable, $2);
				p->info.typ = mksymtype($3.typ, $2);
			  	if ($3.sto & Sextern)
					p->info.typ->transient = -1;
				else if ($3.typ->transient)
					p->info.typ->transient = 1;
			  	p->info.sto = $3.sto;
				$2->token = TYPE;
			  }
			  else
			  {	p = enter(sp->table, $2);
			  	p->info.typ = $3.typ;
			  	p->info.sto = $3.sto;
				if ($5.hasval)
				{	p->info.hasval = True;
					switch ($3.typ->type)
					{	case Tchar:
						case Tuchar:
						case Tshort:
						case Tushort:
						case Tint:
						case Tuint:
						case Tlong:
						case Tulong:
						case Tllong:
						case Tullong:
						case Tenum:
						case Ttime:
							if ($5.typ->type == Tint || $5.typ->type == Tchar || $5.typ->type == Tenum)
								sp->val = p->info.val.i = $5.val.i;
							else
								semerror("type error in initialization constant");
							break;
						case Tfloat:
						case Tdouble:
							if ($5.typ->type == Tfloat || $5.typ->type == Tdouble)
								p->info.val.r = $5.val.r;
							else
								semerror("type error in initialization constant");
							break;
						default:
							if ($5.typ->type == Tpointer && ((Tnode*)$5.typ->ref)->type == Tchar)
								p->info.val.s = $5.val.s;
							else
								semerror("Initialization constant type error");
							break;
					}
				}
				else
					p->info.val.i = sp->val;
				p->info.minOccurs = $4.minOccurs;
				p->info.maxOccurs = $4.maxOccurs;
				if (sp->mask)
				  sp->val <<= 1;
				else
				  sp->val++;
			  	p->info.offset = sp->offset;
				if ($3.sto & Sextern)
					p->level = GLOBAL;
			  	else if (sp->grow)
					sp->offset += p->info.typ->width;
				else if (p->info.typ->width > sp->offset)
					sp->offset = p->info.typ->width;
			  }
			  sp->entry = p;
			}
	;
fdclr	: ptrs name	{ if ($1.sto & Stypedef)
			  {	sprintf(errbuf, "invalid typedef qualifier for `%s'", $2->name);
				semwarn(errbuf);
			  }
			  p = enter(sp->table, $2);
			  p->info.typ = $1.typ;
			  p->info.sto = $1.sto;
			  p->info.hasval = False;
			  p->info.offset = sp->offset;
			  if (sp->grow)
				sp->offset += p->info.typ->width;
			  else if (p->info.typ->width > sp->offset)
				sp->offset = p->info.typ->width;
			  sp->entry = p;
			}
	;
name	: ID		{ $$ = $1; }
	| OPERATOR '='	{ $$ = lookup("operator="); }
	| OPERATOR PA	{ $$ = lookup("operator+="); }
	| OPERATOR NA	{ $$ = lookup("operator-="); }
	| OPERATOR TA	{ $$ = lookup("operator*="); }
	| OPERATOR DA	{ $$ = lookup("operator/="); }
	| OPERATOR MA	{ $$ = lookup("operator%="); }
	| OPERATOR AA	{ $$ = lookup("operator&="); }
	| OPERATOR XA	{ $$ = lookup("operator^="); }
	| OPERATOR OA	{ $$ = lookup("operator|="); }
	| OPERATOR LA	{ $$ = lookup("operator<<="); }
	| OPERATOR RA	{ $$ = lookup("operator>>="); }
	| OPERATOR OR	{ $$ = lookup("operator||"); }
	| OPERATOR AN	{ $$ = lookup("operator&&"); }
	| OPERATOR '|'	{ $$ = lookup("operator|"); }
	| OPERATOR '^'	{ $$ = lookup("operator^"); }
	| OPERATOR '&'	{ $$ = lookup("operator&"); }
	| OPERATOR EQ	{ $$ = lookup("operator=="); }
	| OPERATOR NE	{ $$ = lookup("operator!="); }
	| OPERATOR '<'	{ $$ = lookup("operator<"); }
	| OPERATOR LE	{ $$ = lookup("operator<="); }
	| OPERATOR '>'	{ $$ = lookup("operator>"); }
	| OPERATOR GE	{ $$ = lookup("operator>="); }
	| OPERATOR LS	{ $$ = lookup("operator<<"); }
	| OPERATOR RS	{ $$ = lookup("operator>>"); }
	| OPERATOR '+'	{ $$ = lookup("operator+"); }
	| OPERATOR '-'	{ $$ = lookup("operator-"); }
	| OPERATOR '*'	{ $$ = lookup("operator*"); }
	| OPERATOR '/'	{ $$ = lookup("operator/"); }
	| OPERATOR '%'	{ $$ = lookup("operator%"); }
	| OPERATOR PP	{ $$ = lookup("operator++"); }
	| OPERATOR NN	{ $$ = lookup("operator--"); }
	| OPERATOR'['']'{ $$ = lookup("operator[]"); }
	| OPERATOR'('')'{ $$ = lookup("operator()"); }
	| OPERATOR texp { s1 = c_storage($2.sto);
			  s2 = c_type($2.typ);
			  s = (char*)emalloc(strlen(s1)+strlen(s2)+10);
			  strcpy(s, "operator ");
			  strcat(s, s1);
			  strcat(s, s2);
			  $$ = lookup(s);
			  if (!$$)
				$$ = install(s, ID);
			}
	;
constr	: TYPE		{ if (!(p = entry(classtable, $1)))
			  	semerror("invalid constructor");
			  sp->entry = enter(sp->table, $1);
			  sp->entry->info.typ = mknone();
			  sp->entry->info.sto = Snone;
			  sp->entry->info.offset = sp->offset;
			  sp->node.typ = mkvoid();
			  sp->node.sto = Snone;
			}
	;
destr	: virtual '~' TYPE
			{ if (!(p = entry(classtable, $3)))
			  	semerror("invalid destructor");
			  s = (char*)emalloc(strlen($3->name) + 2);
			  strcpy(s, "~");
			  strcat(s, $3->name);
			  sym = lookup(s);
			  if (!sym)
				sym = install(s, ID);
			  sp->entry = enter(sp->table, sym);
			  sp->entry->info.typ = mknone();
			  sp->entry->info.sto = $1;
			  sp->entry->info.offset = sp->offset;
			  sp->node.typ = mkvoid();
			  sp->node.sto = Snone;
			}
	;
func	: fname '(' s2 fargso ')' constobj abstract
			{ if ($1->level == GLOBAL)
			  {	if (sp->entry && sp->entry->info.typ->type == Tpointer && ((Tnode*)sp->entry->info.typ->ref)->type == Tchar)
			  	{	sprintf(errbuf, "last output parameter of remote method function prototype `%s' is a pointer to a char which will only return one byte: use char** instead to return a string", $1->sym->name);
					semwarn(errbuf);
				}
			  	else if (sp->entry && (sp->entry->info.typ->type == Tpointer || sp->entry->info.typ->type == Treference || sp->entry->info.typ->type == Tarray || is_transient(sp->entry->info.typ)))
					if ($1->info.typ->type == Tint)
					{	$1->info.typ = mkfun(sp->entry);
						if (!is_transient(sp->entry->info.typ) && !is_response(sp->entry->info.typ))
							add_response($1, sp->entry);
					}
					else
					{	sprintf(errbuf, "return type of remote method function prototype `%s' must be integer", $1->sym->name);
						semerror(errbuf);
					}
			  	else
			  	{	sprintf(errbuf, "last output parameter of remote method function prototype `%s' is a return parameter and must be a pointer or reference", $1->sym->name);
					semerror(errbuf);
			  	}
			  	unlinklast(sp->table);
			  	if ((p = entry(classtable, $1->sym)) != (Entry*) 0)
					if ((Table*) p->info.typ->ref != (Table*) 0)
					{	sprintf(errbuf, "remote method name clash: struct/class `%s' already defined (reference from line %d)", $1->sym->name, p->lineno);
						semerror(errbuf);
					}
					else
					{	p->info.typ->ref = sp->table;
						p->info.typ->width = sp->offset;
					}
			  	else
			  	{	p = enter(classtable, $1->sym);
					p->info.typ = mkstruct(sp->table, sp->offset);
					p->info.typ->id = $1->sym;
			  	}
			  }
			  else if ($1->level == INTERNAL)
			  {	$1->info.typ = mkmethod($1->info.typ, sp->table);
				$1->info.sto |= $6 | $7;
			  }
			  exitscope();
			}
	;
fname	:		{ $$ = sp->entry; }
	;
fargso	: /* empty */	{ }
	| fargs		{ }
	;
fargs	: farg		{ }
	| farg ',' fargs{ }
	;
farg	: tspec ptrs arg array init
			{ if ($4.sto & Stypedef)
			  	semwarn("typedef in function argument");
			  p = enter(sp->table, $3);
			  p->info.typ = $4.typ;
			  p->info.sto = $4.sto;
			  if ($5.hasval)
			  {	p->info.hasval = True;
				switch ($4.typ->type)
				{	case Tchar:
					case Tuchar:
					case Tshort:
					case Tushort:
					case Tint:
					case Tuint:
					case Tlong:
					case Tulong:
					case Tenum:
					case Ttime:
						if ($5.typ->type == Tint || $5.typ->type == Tchar || $5.typ->type == Tenum)
							sp->val = p->info.val.i = $5.val.i;
						else
							semerror("type error in initialization constant");
						break;
					case Tfloat:
					case Tdouble:
						if ($5.typ->type == Tfloat || $5.typ->type == Tdouble)
							p->info.val.r = $5.val.r;
						else
							semerror("type error in initialization constant");
						break;
					default:
						if ($5.typ->type == Tpointer && ((Tnode*)$5.typ->ref)->type == Tchar)
							p->info.val.s = $5.val.s;
						else
							semerror("Initialization constant type error");
						break;
				}
			  }
			  p->info.offset = sp->offset;
			  if ($4.sto & Sextern)
				p->level = GLOBAL;
			  else if (sp->grow)
				sp->offset += p->info.typ->width;
			  else if (p->info.typ->width > sp->offset)
				sp->offset = p->info.typ->width;
			  sp->entry = p;
			}
	;
arg	: /* empty */	{ $$ = gensymidx("_param", ++sp->val); }
	| ID		{ $$ = $1; }
	;

/******************************************************************************\

	Type specification

\******************************************************************************/

/* texp : type expression (subset of C) */
texp	: tspec ptrs array
			{ $$ = $3; }
	| tspec ptrs ID array
			{ $$ = $4; }
	;
spec	: /*empty */	{ $$.typ = mkint();
			  $$.sto = Snone;
			  sp->node = $$;
			}
	| store spec	{ $$.typ = $2.typ;
			  $$.sto = $1 | $2.sto;
			  if (($$.sto & Sattribute) && (!is_primitive_or_string($2.typ)))
			  {	semwarn("invalid attribute type");
			  	$$.sto &= ~Sattribute;
			  }
			  sp->node = $$;
			  if ($1 & Sextern)
				transient--;
			}
	| type spec	{ if ($1->type == Tint)
				switch ($2.typ->type)
				{ case Tchar:	$$.typ = $2.typ; break;
				  case Tshort:	$$.typ = $2.typ; break;
				  case Tint:	$$.typ = $1; break;
				  case Tlong:	$$.typ = $2.typ; break;
				  case Tllong:	$$.typ = $2.typ; break;
				  default:	semwarn("illegal use of `signed'");
						$$.typ = $2.typ;
				}
			  else if ($1->type == Tuint)
				switch ($2.typ->type)
				{ case Tchar:	$$.typ = mkuchar(); break;
				  case Tshort:	$$.typ = mkushort(); break;
				  case Tint:	$$.typ = $1; break;
				  case Tlong:	$$.typ = mkulong(); break;
				  case Tllong:	$$.typ = mkullong(); break;
				  default:	semwarn("illegal use of `unsigned'");
						$$.typ = $2.typ;
				}
			  else if ($1->type == Tlong)
				switch ($2.typ->type)
				{ case Tint:	$$.typ = $1; break;
				  case Tlong:	$$.typ = mkllong(); break;
				  case Tuint:	$$.typ = mkulong(); break;
				  case Tulong:	$$.typ = mkullong(); break;
				  default:	semwarn("illegal use of `long'");
						$$.typ = $2.typ;
				}
			  else if ($1->type == Tulong)
				switch ($2.typ->type)
				{ case Tint:	$$.typ = $1; break;
				  case Tlong:	$$.typ = mkullong(); break;
				  case Tuint:	$$.typ = $1; break;
				  case Tulong:	$$.typ = mkullong(); break;
				  default:	semwarn("illegal use of `long'");
						$$.typ = $2.typ;
				}
			  else
				$$.typ = $1;
			  $$.sto = $2.sto;
			  sp->node = $$;
			}
	;
tspec	: store		{ $$.typ = mkint();
			  $$.sto = $1;
			  sp->node = $$;
			  if ($1 & Sextern)
				transient--;
			}
	| type		{ $$.typ = $1;
			  $$.sto = Snone;
			  sp->node = $$;
			}
	| store tspec	{ $$.typ = $2.typ;
			  $$.sto = $1 | $2.sto;
			  if (($$.sto & Sattribute) && (!is_primitive_or_string($2.typ)))
			  {	semwarn("invalid attribute type");
			  	$$.sto &= ~Sattribute;
			  }
			  sp->node = $$;
			  if ($1 & Sextern)
				transient--;
			}
	| type tspec	{ if ($1->type == Tint)
				switch ($2.typ->type)
				{ case Tchar:	$$.typ = $2.typ; break;
				  case Tshort:	$$.typ = $2.typ; break;
				  case Tint:	$$.typ = $1; break;
				  case Tlong:	$$.typ = $2.typ; break;
				  case Tllong:	$$.typ = $2.typ; break;
				  default:	semwarn("illegal use of `signed'");
						$$.typ = $2.typ;
				}
			  else if ($1->type == Tuint)
				switch ($2.typ->type)
				{ case Tchar:	$$.typ = mkuchar(); break;
				  case Tshort:	$$.typ = mkushort(); break;
				  case Tint:	$$.typ = $1; break;
				  case Tlong:	$$.typ = mkulong(); break;
				  case Tllong:	$$.typ = mkullong(); break;
				  default:	semwarn("illegal use of `unsigned'");
						$$.typ = $2.typ;
				}
			  else if ($1->type == Tlong)
				switch ($2.typ->type)
				{ case Tint:	$$.typ = $1; break;
				  case Tlong:	$$.typ = mkllong(); break;
				  case Tuint:	$$.typ = mkulong(); break;
				  case Tulong:	$$.typ = mkullong(); break;
				  default:	semwarn("illegal use of `long'");
						$$.typ = $2.typ;
				}
			  else if ($1->type == Tulong)
				switch ($2.typ->type)
				{ case Tint:	$$.typ = $1; break;
				  case Tlong:	$$.typ = mkullong(); break;
				  case Tuint:	$$.typ = $1; break;
				  case Tulong:	$$.typ = mkullong(); break;
				  default:	semwarn("illegal use of `long'");
						$$.typ = $2.typ;
				}
			  else
				$$.typ = $1;
			  $$.sto = $2.sto;
			  sp->node = $$;
			}
	;
type	: VOID		{ $$ = mkvoid(); }
	| BOOL		{ $$ = mkbool(); }
	| CHAR		{ $$ = mkchar(); }
	| WCHAR		{ $$ = mkwchart(); }
	| SHORT		{ $$ = mkshort(); }
	| INT		{ $$ = mkint(); }
	| LONG		{ $$ = mklong(); }
	| LLONG		{ $$ = mkllong(); }
	| ULLONG	{ $$ = mkullong(); }
	| SIZE		{ $$ = mkuint(); }
	| FLOAT		{ $$ = mkfloat(); }
	| DOUBLE	{ $$ = mkdouble(); }
	| SIGNED	{ $$ = mkint(); }
	| UNSIGNED	{ $$ = mkuint(); }
	| TIME		{ $$ = mktimet(); }
	| CLASS '{' s2 decls '}'
			{ sym = gensym("Struct");
			  sprintf(errbuf, "nameless class will be named `%s'", sym->name);
			  semwarn(errbuf);
			  if ((p = entry(classtable, sym)))
			  {	if (p->info.typ->ref)
				{	sprintf(errbuf, "class `%s' already defined", sym->name);
					semerror(errbuf);
				}
			  }
			  else
			  {	p = enter(classtable, sym);
				p->info.typ = mkclass((Table*)0, 0);
			  }
			  sym->token = TYPE;
			  sp->table->sym = sym;
			  p->info.typ->ref = sp->table;
			  p->info.typ->width = sp->offset;
			  p->info.typ->id = sym;
			  $$ = p->info.typ;
			  exitscope();
			}
	| class '{' s2 decls '}'
			{ sp->table->sym = $1->sym;
			  $1->info.typ->ref = sp->table;
			  $1->info.typ->width = sp->offset;
			  $1->info.typ->id = $1->sym;
			  if ($1->info.typ->base)
			  	sp->table->prev = (Table*)entry(classtable, $1->info.typ->base)->info.typ->ref;
			  $$ = $1->info.typ;
			  exitscope();
			}
	| class ':' super '{' s2 decls '}'
			{ sp->table->sym = $1->sym;
			  if (!$3)
				semerror("invalid base class");
			  else
			  {	sp->table->prev = (Table*)$3->info.typ->ref;
				if (!sp->table->prev && !$3->info.typ->transient)
				{	sprintf(errbuf, "class `%s' has incomplete type", $3->sym->name);
					semerror(errbuf);
				}
			  }
			  $1->info.typ->ref = sp->table;
			  $1->info.typ->width = sp->offset;
			  $1->info.typ->id = $1->sym;
			  $1->info.typ->base = $3->info.typ->id;
			  $$ = $1->info.typ;
			  exitscope();
			}
	| class		{ $1->info.typ->id = $1->sym;
			  $$ = $1->info.typ;
			}
	| class ':' super
			{ if (!$3)
				semerror("invalid base class");
			  else
			  {	if (!$3->info.typ->ref && !$3->info.typ->transient)
				{	sprintf(errbuf, "class `%s' has incomplete type", $3->sym->name);
					semerror(errbuf);
				}
			  }
			  $1->info.typ->id = $1->sym;
			  $1->info.typ->base = $3->info.typ->id;
			  $$ = $1->info.typ;
			}
	| STRUCT '{' s2 decls '}'
			{ sym = gensym("Struct");
			  sprintf(errbuf, "nameless struct will be named `%s'", sym->name);
			  semwarn(errbuf);
			  if ((p = entry(classtable, sym)))
			  {	if (p->info.typ->ref)
				{	sprintf(errbuf, "struct `%s' already defined", sym->name);
					semerror(errbuf);
				}
				else
				{	p->info.typ->ref = sp->table;
					p->info.typ->width = sp->offset;
				}
			  }
			  else
			  {	p = enter(classtable, sym);
				p->info.typ = mkstruct(sp->table, sp->offset);
			  }
			  p->info.typ->id = sym;
			  $$ = p->info.typ;
			  exitscope();
			}
	| STRUCT ID '{' s2 decls '}'
			{ if ((p = entry(classtable, $2)) != (Entry*)0)
				if (p->info.typ->ref)
				{	sprintf(errbuf, "struct `%s' already defined (referenced from line %d)", $2->name, p->lineno);
					semerror(errbuf);
				} else {
					p->info.typ->ref = sp->table;
					p->info.typ->width = sp->offset;
				}
			  else {
			  	p = enter(classtable, $2);
				p->info.typ = mkstruct(sp->table, sp->offset);
			  	p->info.typ->id = $2;
			  }
			  $$ = p->info.typ;
			  exitscope();
			}
	| STRUCT ID	{ if ((p = entry(classtable, $2)) != (Entry*)0) {
			  	$$ = p->info.typ;
			  } else {
			  	p = enter(classtable, $2);
			  	$$ = p->info.typ = mkstruct((Table*)0, 0);
				p->info.typ->id = $2;
			  }
			}
	| UNION '{' s3 decls '}'
			{ $$ = mkunion(sp->table, sp->offset);
			  semwarn("Unions cannot be (de)serialized");
			  exitscope();
			}
	| UNION ID '{' s3 decls '}'
			{ semwarn("Unions cannot be (de)serialized");
			  if ((p = entry(uniontable, $2)) != (Entry*) 0)
				if ((Table*) p->info.typ->ref != (Table*) 0) {
					sprintf(errbuf, "union `%s' already defined", $2->name);
					semerror(errbuf);
				} else {
					p->info.typ->ref = sp->table;
					p->info.typ->width = sp->offset;
				}
			  else {
			  	p = enter(uniontable, $2);
				p->info.typ = mkunion(sp->table, sp->offset);
			  }
			  p->info.typ->id = $2;
			  $$ = p->info.typ;
			  exitscope();
			}
	| UNION ID	{ semwarn("Unions cannot be (de)serialized");
			  if ((p = entry(uniontable, $2)) != (Entry*) 0)
			  	$$ = p->info.typ;
			  else {
			  	p = enter(uniontable, $2);
			  	$$ = p->info.typ = mkunion((Table*) 0, 0);
				p->info.typ->id = $2;
			  }
			}
	| ENUM '{' s2 dclrs s5 '}'
			{ sym = gensym("Enum");
			  sprintf(errbuf, "nameless enum will be named `%s'", sym->name);
			  semwarn(errbuf);
			  if ((p = entry(enumtable, sym)) != (Entry*) 0)
				if ((Table*) p->info.typ->ref != (Table*) 0) {
					sprintf(errbuf, "enum `%s' already defined", sym->name);
					semerror(errbuf);
				} else {
					p->info.typ->ref = sp->table;
					p->info.typ->width = sp->offset;
				}
			  else {
			  	p = enter(enumtable, sym);
				p->info.typ = mkenum(sp->table);
			  }
			  p->info.typ->id = sym;
			  $$ = p->info.typ;
			  exitscope();
			}
	| ENUM ID '{' s2 dclrs s5 '}'
			{ if ((p = entry(enumtable, $2)) != (Entry*) 0)
				if ((Table*) p->info.typ->ref != (Table*) 0) {
					sprintf(errbuf, "enum `%s' already defined (referenced from line %d)", $2->name, p->lineno);
					semerror(errbuf);
				} else {
					p->info.typ->ref = sp->table;
					p->info.typ->width = sp->offset;
				}
			  else {
			  	p = enter(enumtable, $2);
				p->info.typ = mkenum(sp->table);
			  }
			  p->info.typ->id = $2;
			  $$ = p->info.typ;
			  exitscope();
			}
	| ENUM '*' ID '{' s4 dclrs s5 '}'
			{ if ((p = entry(enumtable, $3)))
				if (p->info.typ->ref)
				{	sprintf(errbuf, "enum `%s' already defined (referenced from line %d)", $3->name, p->lineno);
					semerror(errbuf);
				} else {
					p->info.typ->ref = sp->table;
					p->info.typ->width = sp->offset;
				}
			  else
			  {	p = enter(enumtable, $3);
				p->info.typ = mkmask(sp->table);
			  }
			  p->info.typ->id = $3;
			  $$ = p->info.typ;
			  exitscope();
			}
	| ENUM ID	{ if ((p = entry(enumtable, $2)))
			  	$$ = p->info.typ;
			  else
			  {	p = enter(enumtable, $2);
			  	$$ = p->info.typ = mkenum((Table*)0);
				p->info.typ->id = $2;
			  }
			}
	| TYPE		{ if ((p = entry(typetable, $1)))
				$$ = p->info.typ;
			  else if ((p = entry(classtable, $1)))
			  	$$ = p->info.typ;
			  else
			  {	sprintf(errbuf, "unknown type `%s'", $1->name);
				semerror(errbuf);
				$$ = mkint();
			  }
			}
	;
class	: CLASS ID	{ if ((p = entry(classtable, $2)))
			  {	if (p->info.typ->ref)
				{	sprintf(errbuf, "class `%s' already defined (referenced from line %d)", $2->name, p->lineno);
					semerror(errbuf);
				}
			  }
			  else
			  {	p = enter(classtable, $2);
				p->info.typ = mkclass((Table*)0, 0);
			  }
			  $2->token = TYPE;
			  $$ = p;
			}
	| CLASS TYPE	{ if ((p = entry(classtable, $2)))
			  {	if (p->info.typ->ref)
				{	sprintf(errbuf, "class `%s' already defined (referenced from line %d)", $2->name, p->lineno);
					semerror(errbuf);
				}
			  }
			  else
			  {	sprintf(errbuf, "invalid class name `%s'", $2->name);
				semerror(errbuf);
			  }
			  $$ = p;
			}
	;
super	: PROTECTED TYPE{ $$ = entry(classtable, $2); }
	| PRIVATE TYPE	{ $$ = entry(classtable, $2); }
	| PUBLIC TYPE	{ $$ = entry(classtable, $2); }
	| TYPE		{ $$ = entry(classtable, $1); }
	;
s2	: /* empty */	{ enterscope(mktable((Table*) 0), 0);
			  sp->entry = (Entry*)0;
			}
	;
s3	: /* empty */	{ enterscope(mktable((Table*) 0), 0);
			  sp->entry = (Entry*)0;
			  sp->grow = False;
			}
	;
s4	: /* empty */	{ enterscope(mktable((Table*) 0), 0);
			  sp->entry = (Entry*)0;
			  sp->mask = True;
			  sp->val = 1;
			}
	;
s5	: /* empty */	{ }
	| ','		{ }
	;
store	: AUTO		{ $$ = Sauto; }
	| REGISTER	{ $$ = Sregister; }
	| STATIC	{ $$ = Sstatic; }
	| EXTERN	{ $$ = Sextern; transient++; }
	| TYPEDEF	{ $$ = Stypedef; }
	| VIRTUAL	{ $$ = Svirtual; }
	| CONST		{ $$ = Sconst; }
	| FRIEND	{ $$ = Sfriend; }
	| INLINE	{ $$ = Sinline; }
	| MUSTUNDERSTAND{ $$ = SmustUnderstand; }
	| '@'		{ $$ = Sattribute; }
	;
constobj: /* empty */	{ $$ = Snone; }
	| CONST		{ $$ = Sconstobj; }
	;
abstract: /* empty */	{ $$ = Snone; }
	| '=' LNG	{ $$ = Sabstract; }
	;
virtual : /* empty */	{ $$ = Snone; }
	| VIRTUAL	{ $$ = Svirtual; }
	;
ptrs	: /* empty */	{ $$ = tmp = sp->node; }
	| ptrs '*'	{ tmp.typ = mkpointer(tmp.typ);
			  tmp.typ->transient = transient;
			  $$ = tmp;
			}
	| ptrs '&'	{ tmp.typ = mkreference(tmp.typ);
			  tmp.typ->transient = transient;
			  $$ = tmp;
			}
	;
array	: /* empty */ 	{ $$ = tmp;	/* tmp is inherited */
			  switch (tmp.typ->type)
			  {	case Tstruct:
					if (!tmp.typ->ref && !tmp.typ->transient)
			   		{	sprintf(errbuf, "struct `%s' has incomplete type", tmp.typ->id->name);
						semerror(errbuf);
					}
					break;
			   	case Tclass:
					if (!tmp.typ->ref && !tmp.typ->transient)
			   		{	sprintf(errbuf, "class `%s' has incomplete type", tmp.typ->id->name);
						semerror(errbuf);
						break;
			  		}
			  }
			}
	| '[' cexp ']' array
			{ if ($4.typ->type == Tchar)
			  {	sprintf(errbuf, "char[%d] will be encoded as an array of %d bytes: use char* for strings", $2.val.i, $2.val.i);
			  	semwarn(errbuf);
			  }
			  if ($2.hasval && $2.typ->type == Tint && $2.val.i > 0 && $4.typ->width > 0)
				$$.typ = mkarray($4.typ, (int) $2.val.i * $4.typ->width);
			  else
			  {	$$.typ = mkarray($4.typ, 0);
			  	semerror("undetermined array size");
			  }
			  $$.sto = $4.sto;
			}
	| '[' ']' array	{ $$.typ = mkpointer($3.typ); /* zero size array = pointer */
			  $$.sto = $3.sto;
			}
	;
init	: /* empty */   { $$.hasval = False; }
	| '=' cexp      { if ($2.hasval)
			  {	$$.typ = $2.typ;
				$$.hasval = True;
				$$.val = $2.val;
			  }
			  else
			  {	$$.hasval = False;
				semerror("initialization expression not constant");
			  }
			}
        ;
occurs	: /* empty */	{ $$.minOccurs = 1;
			  $$.maxOccurs = 1;
			}
	| LNG		{ $$.minOccurs = $1;
			  $$.maxOccurs = 1;
			}
	| LNG ':'	{ $$.minOccurs = $1;
			  $$.maxOccurs = 1;
			}
	| LNG ':' LNG	{ $$.minOccurs = $1;
			  $$.maxOccurs = $3;
			}
	| ':' LNG	{ $$.minOccurs = 1;
			  $$.maxOccurs = $2;
			}
	;

/******************************************************************************\

	Expressions

\******************************************************************************/

expr	: expr ',' expr	{ $$ = $3; }
	| cexp		{ $$ = $1; }
	;
/* cexp : conditional expression */
cexp	: obex '?' qexp ':' cexp
			{ $$.typ = $3.typ;
			  $$.sto = Snone;
			  $$.hasval = False;
			}
	| oexp
	;
/* qexp : true-branch of ? : conditional expression */
qexp	: expr		{ $$ = $1; }
	;
/* oexp : or-expression */
oexp	: obex OR aexp	{ $$.hasval = False;
			  $$.typ = mkint();
			}
	| aexp		{ $$ = $1; }
	;
obex	: oexp		{ $$ = $1; }
	;
/* aexp : and-expression */
aexp	: abex AN rexp	{ $$.hasval = False;
			  $$.typ = mkint();
			}
	| rexp		{ $$ = $1; }
	;
abex	: aexp		{ $$ = $1; }
	;
/* rexp : relational expression */
rexp	: rexp '|' rexp	{ $$ = iop("|", $1, $3); }
	| rexp '^' rexp	{ $$ = iop("^", $1, $3); }
	| rexp '&' rexp	{ $$ = iop("&", $1, $3); }
	| rexp EQ  rexp	{ $$ = relop("==", $1, $3); }
	| rexp NE  rexp	{ $$ = relop("!=", $1, $3); }
	| rexp '<' rexp	{ $$ = relop("<", $1, $3); }
	| rexp LE  rexp	{ $$ = relop("<=", $1, $3); }
	| rexp '>' rexp	{ $$ = relop(">", $1, $3); }
	| rexp GE  rexp	{ $$ = relop(">=", $1, $3); }
	| rexp LS  rexp	{ $$ = iop("<<", $1, $3); }
	| rexp RS  rexp	{ $$ = iop(">>", $1, $3); }
	| rexp '+' rexp	{ $$ = op("+", $1, $3); }
	| rexp '-' rexp	{ $$ = op("-", $1, $3); }
	| rexp '*' rexp	{ $$ = op("*", $1, $3); }
	| rexp '/' rexp	{ $$ = op("/", $1, $3); }
	| rexp '%' rexp	{ $$ = iop("%", $1, $3); }
	| lexp		{ $$ = $1; }
	;
/* lexp : lvalue kind of expression with optional prefix contructs */
lexp	: '!' lexp	{ if ($2.hasval)
				$$.val.i = !$2.val.i;
			  $$.typ = $2.typ;
			  $$.hasval = $2.hasval;
			}
	| '~' lexp	{ if ($2.hasval)
				$$.val.i = ~$2.val.i;
			  $$.typ = $2.typ;
			  $$.hasval = $2.hasval;
			}
	| '-' lexp	{ if ($2.hasval) {
				if (integer($2.typ))
					$$.val.i = -$2.val.i;
				else if (real($2.typ))
					$$.val.r = -$2.val.r;
				else	typerror("string?");
			  }
			  $$.typ = $2.typ;
			  $$.hasval = $2.hasval;
			}
	| '+' lexp	{ $$ = $2; }
	| '*' lexp	{ if ($2.typ->type == Tpointer) {
			  	$$.typ = (Tnode*) $2.typ->ref;
			  } else
			  	typerror("dereference of non-pointer type");
			  $$.sto = Snone;
			  $$.hasval = False;
			}
	| '&' lexp	{ $$.typ = mkpointer($2.typ);
			  $$.sto = Snone;
			  $$.hasval = False;
			}
	| SIZEOF '(' texp ')'
			{ $$.hasval = True;
			  $$.typ = mkint();
			  $$.val.i = $3.typ->width;
			}
	| pexp		{ $$ = $1; }
	;
/* pexp : primitive expression with optional postfix constructs */
pexp	: '(' expr ')'	{ $$ = $2; }
	| ID		{ if ((p = enumentry($1)) == (Entry*) 0)
				p = undefined($1);
			  else
			  	$$.hasval = True;
			  $$.typ = p->info.typ;
			  $$.val = p->info.val;
			}
	| LNG		{ $$.typ = mkint();
			  $$.hasval = True;
			  $$.val.i = $1;
			}
	| DBL		{ $$.typ = mkfloat();
			  $$.hasval = True;
			  $$.val.r = $1;
			}
	| CHR		{ $$.typ = mkchar();
			  $$.hasval = True;
			  $$.val.i = $1;
			}
	| STR		{ $$.typ = mkstring();
			  $$.hasval = True;
			  $$.val.s = $1;
			}
	| CFALSE	{ $$.typ = mkbool();
			  $$.hasval = True;
			  $$.val.i = 0;
			}
	| CTRUE		{ $$.typ = mkbool();
			  $$.hasval = True;
			  $$.val.i = 1;
			}
	;

%%

/*
 * ???
 */
int
yywrap()
{	return 1;
}

/******************************************************************************\

	Support routines

\******************************************************************************/

static Node
op(const char *op, Node p, Node q)
{	Node	r;
	Tnode	*typ;
	r.typ = p.typ;
	r.sto = Snone;
	if (p.hasval && q.hasval) {
		if (integer(p.typ) && integer(q.typ))
			switch (op[0]) {
			case '|':	r.val.i = p.val.i |  q.val.i; break;
			case '^':	r.val.i = p.val.i ^  q.val.i; break;
			case '&':	r.val.i = p.val.i &  q.val.i; break;
			case '<':	r.val.i = p.val.i << q.val.i; break;
			case '>':	r.val.i = p.val.i >> q.val.i; break;
			case '+':	r.val.i = p.val.i +  q.val.i; break;
			case '-':	r.val.i = p.val.i -  q.val.i; break;
			case '*':	r.val.i = p.val.i *  q.val.i; break;
			case '/':	r.val.i = p.val.i /  q.val.i; break;
			case '%':	r.val.i = p.val.i %  q.val.i; break;
			default:	typerror(op);
			}
		else if (real(p.typ) && real(q.typ))
			switch (op[0]) {
			case '+':	r.val.r = p.val.r + q.val.r; break;
			case '-':	r.val.r = p.val.r - q.val.r; break;
			case '*':	r.val.r = p.val.r * q.val.r; break;
			case '/':	r.val.r = p.val.r / q.val.r; break;
			default:	typerror(op);
			}
		else	semerror("illegal constant operation");
		r.hasval = True;
	} else {
		typ = mgtype(p.typ, q.typ);
		r.hasval = False;
	}
	return r;
}

static Node
iop(const char *iop, Node p, Node q)
{	if (integer(p.typ) && integer(q.typ))
		return op(iop, p, q);
	typerror("integer operands only");
	return p;
}

static Node
relop(const char *op, Node p, Node q)
{	Node	r;
	Tnode	*typ;
	r.typ = mkint();
	r.sto = Snone;
	r.hasval = False;
	if (p.typ->type != Tpointer || p.typ != q.typ)
		typ = mgtype(p.typ, q.typ);
	return r;
}

/******************************************************************************\

	Scope management

\******************************************************************************/

/*
mkscope - initialize scope stack with a new table and offset
*/
static void
mkscope(Table *table, int offset)
{	sp = stack-1;
	enterscope(table, offset);
}

/*
enterscope - enter a new scope by pushing a new table and offset on the stack
*/
static void
enterscope(Table *table, int offset)
{	if (++sp == stack+MAXNEST)
		execerror("maximum scope depth exceeded");
	sp->table = table;
	sp->val = 0;
	sp->offset = offset;
	sp->grow = True;	/* by default, offset grows */
	sp->mask = False;
}

/*
exitscope - exit a scope by popping the table and offset from the stack
*/
static void
exitscope()
{	check(sp-- != stack, "exitscope() has no matching enterscope()");
}

/******************************************************************************\

	Undefined symbol

\******************************************************************************/

static Entry*
undefined(Symbol *sym)
{	Entry	*p;
	sprintf(errbuf, "undefined identifier `%s'", sym->name);
	semwarn(errbuf);
	p = enter(sp->table, sym);
	p->level = GLOBAL;
	p->info.typ = mkint();
	p->info.sto = Sextern;
	p->info.hasval = False;
	return p;
}

/*
mgtype - return most general type among two numerical types
*/
Tnode*
mgtype(Tnode *typ1, Tnode *typ2)
{	if (numeric(typ1) && numeric(typ2)) {
		if (typ1->type < typ2->type)
			return typ2;
	} else	typerror("non-numeric type");
	return typ1;
}

/******************************************************************************\

	Type checks

\******************************************************************************/

static int
integer(Tnode *typ)
{	switch (typ->type) {
	case Tchar:
	case Tshort:
	case Tint:
	case Tlong:	return True;
	}
	return False;
}

static int
real(Tnode *typ)
{	switch (typ->type) {
	case Tfloat:
	case Tdouble:	return True;
	}
	return False;
}

static int
numeric(Tnode *typ)
{	return integer(typ) || real(typ);
}

static int
pointer(Tnode *typ)
{	return typ->type == Tpointer;
}

static void
add_fault(Table *gt)
{ Table *t;
  Entry *p1, *p2;
  Symbol *s1, *s2;
  s1 = lookup("SOAP_ENV__Code");
  p1 = entry(classtable, s1);
  if (!p1 || !p1->info.typ->ref)
  { t = mktable((Table*)0);
    if (!p1)
    { p1 = enter(classtable, s1);
      p1->info.typ = mkstruct(t, 3*4);
      p1->info.typ->id = s1;
    }
    else
      p1->info.typ->ref = t;
    p2 = enter(t, lookup("SOAP_ENV__Value"));
    p2->info.typ = mkstring();
    p2 = enter(t, lookup("SOAP_ENV__Node"));
    p2->info.typ = mkstring();
    p2 = enter(t, lookup("SOAP_ENV__Role"));
    p2->info.typ = mkstring();
  }
  s2 = lookup("SOAP_ENV__Fault");
  p2 = entry(classtable, s2);
  if (!p2)
  { t = mktable((Table*)0);
    p2 = enter(classtable, s2);
    p2->info.typ = mkstruct(t, 9*4);
    p2->info.typ->id = s2;
    p2 = enter(t, lookup("faultcode"));
    p2->info.typ = mkstring();
    p2 = enter(t, lookup("faultstring"));
    p2->info.typ = mkstring();
    p2 = enter(t, lookup("faultactor"));
    p2->info.typ = mkstring();
    p2 = enter(t, lookup("detail"));
    p2->info.typ = mkstring();
    p2 = enter(t, s1);
    p2->info.typ = mkpointer(p1->info.typ);
    p2 = enter(t, lookup("SOAP_ENV__Reason"));
    p2->info.typ = mkstring();
    p2 = enter(t, lookup("SOAP_ENV__Detail"));
    p2->info.typ = mkstring();
    custom_fault = 0;
  }
}

static void
add_header(Table *gt)
{ Table *t;
  Entry *p;
  Symbol *s = lookup("SOAP_ENV__Header");
  p = entry(classtable, s);
  if (!p)
  { t = mktable((Table*)0);
    p = enter(t, lookup("dummy"));
    p->info.typ = mkpointer(mkvoid());
    p = enter(classtable, s);
    p->info.typ = mkstruct(t, 4);
    p->info.typ->id = s;
    custom_header = 0;
  }
}

static void
add_response(Entry *fun, Entry *ret)
{ Table *t;
  Entry *p, *q;
  Symbol *s;
  int n = strlen(fun->sym->name);
  char *r = (char*)emalloc(n+9);
  strcpy(r, fun->sym->name);
  strcat(r, "Response");
  if (!(s = lookup(r)))
    s = install(r, ID);
  free(r);
  t = mktable((Table*)0);
  q = enter(t, ret->sym);
  q->info = ret->info;
  if (q->info.typ->type == Treference)
    q->info.typ = (Tnode*)q->info.typ->ref;
  p = enter(classtable, s);
  p->info.typ = mkstruct(t, 4);
  p->info.typ->id = s;
  fun->info.typ->response = p;
}
