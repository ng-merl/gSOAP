/*

schema.h

XSD binding schema interface

--------------------------------------------------------------------------------
gSOAP XML Web services tools
Copyright (C) 2004, Robert van Engelen, Genivia, Inc. All Rights Reserved.

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

*/

//gsoap xs schema documentation:	XSD binding schema
//gsoap xs schema namespace:		http://www.w3.org/2000/10/XMLSchema
/* For the wsdl:arrayType attribute to support old style SOAP arrays: */
//gsoap wsdl schema namespace:		http://schemas.xmlsoap.org/wsdl/

#import "imports.h"

class xs__schema;			// forward declaration
class xs__simpleType;			// forward declaration
class xs__complexType;			// forward declaration
class xs__extension;			// forward declaration
class xs__restriction;			// forward declaration
class xs__sequence;			// forward declaration
class xs__choice;			// forward declaration
class xs__list;				// forward declaration
class xs__union;			// forward declaration

class xs__element
{ public:
	// @xsd__ID			id;
	@xsd__NCName			name;
	@xsd__QName			ref;
	@xsd__QName			type;
	@xsd__string			default_;
	@xsd__string			fixed;
	@xsd__boolean			nillable		= false;
	@xsd__string			minOccurs;		// xsd:nonNegativeInteger
	@xsd__string			maxOccurs;		// xsd:nonNegativeInteger|unbounded
	xs__simpleType			*simpleType;		// choice
	xs__complexType			*complexType;		// choice
  private:
	xs__schema			*schemaRef;		// schema to which this belongs
  	xs__element			*elementRef;		// traverse() finds ref
  	xs__simpleType			*simpleTypeRef;		// traverse() finds type or = simpleType above
  	xs__complexType			*complexTypeRef;	// traverse() finds type or = complexType above
  public:
					xs__element();
					~xs__element();
  	int				traverse(xs__schema&);
	void				schemaPtr(xs__schema*);
	void				elementPtr(xs__element*);
	void				simpleTypePtr(xs__simpleType*);
	void				complexTypePtr(xs__complexType*);
	xs__schema			*schemaPtr() const;
	xs__element			*elementPtr() const;
	xs__simpleType			*simpleTypePtr() const;
	xs__complexType			*complexTypePtr() const;
};

enum xs__attribute_use { optional, prohibited, required, default_, fixed_ };

class xs__attribute
{ public:
	@xsd__NCName			name;
	@xsd__QName			ref;
	@xsd__QName			type;
	@enum xs__attribute_use		use			= optional;
	@xsd__string			value;
	@xsd__QName			wsdl__arrayType;	// extensibility attribute added to simplify WSDL parsing
	xs__simpleType			*simpleType;
  private:
	xs__schema			*schemaRef;		// schema to which this belongs
  	xs__attribute			*attributeRef;		// traverse() finds ref
  	xs__simpleType			*simpleTypeRef;		// traverse() finds type or = simpleType above
  public:
  	int				traverse(xs__schema&);
	void				schemaPtr(xs__schema*);
	void				attributePtr(xs__attribute*);
	void				simpleTypePtr(xs__simpleType*);
	xs__schema			*schemaPtr() const;
	xs__attribute			*attributePtr() const;
	xs__simpleType			*simpleTypePtr() const;
};

class xs__all
{ public:
	std::vector<xs__element>	element;
  public:
  	int				traverse(xs__schema&);
};

enum xs__processContents { strict, skip, lax };

typedef char *xs__namespaceList;	// "##any" or "##other" or list of URI, "##targetNamespace", "##local"

class xs__any
{ public:
	@xs__namespaceList		namespace_		= "##any";
	@enum xs__processContents	processContents		= strict;
	@xsd__string			minOccurs;		// xsd:nonNegativeInteger
	@xsd__string			maxOccurs;		// xsd:nonNegativeInteger|unbounded
	std::vector<xs__element>	element;
  public:
  	int				traverse(xs__schema&);
};

class xs__group
{ public:
	@xsd__NCName			name;
	@xsd__QName			ref;
	xs__all				*all;			// choice
	xs__choice			*choice;		// choice
	xs__sequence			*sequence;		// choice
  private:
	xs__schema			*schemaRef;		// schema to which this belongs
  	xs__group			*groupRef;		// traverse() finds ref
  public:
  	int				traverse(xs__schema&);
	void				schemaPtr(xs__schema*);
	void				groupPtr(xs__group*);
	xs__schema			*schemaPtr() const;
	xs__group			*groupPtr() const;
};

class xs__choice
{ public:
	@xsd__string			minOccurs;		// xsd:nonNegativeInteger
	@xsd__string			maxOccurs;		// xsd:nonNegativeInteger|unbounded
	std::vector<xs__element>	element;
	std::vector<xs__group>		group;
	//std::vector<xs__choice>		choice;
	std::vector<xs__sequence*>	sequence;
	std::vector<xs__any>		any;
  public:
  	int				traverse(xs__schema&);
};

class xs__sequence
{ public:
	@xsd__string			minOccurs;		// xsd:nonNegativeInteger
	@xsd__string			maxOccurs;		// xsd:nonNegativeInteger|unbounded
	std::vector<xs__element>	element;
	std::vector<xs__group>		group;
	std::vector<xs__choice>		choice;
	//std::vector<xs__sequence>	sequence;
	std::vector<xs__any>		any;
  public:
  	int				traverse(xs__schema&);
};

class xs__attributeGroup
{ public:
	@xsd__NCName			name;
	@xsd__QName			ref;
	std::vector<xs__attribute>	attribute;
  private:
	xs__schema			*schemaRef;
  	xs__attributeGroup		*attributeGroupRef;
  public:
  	int				traverse(xs__schema&);
	void				schemaPtr(xs__schema*);
	void				attributeGroupPtr(xs__attributeGroup*);
	xs__schema			*schemaPtr() const;
	xs__attributeGroup		*attributeGroupPtr() const;
};

class xs__anyAttribute
{ public:
	@xs__namespaceList		namespace_		= "##any";
	@enum xs__processContents	processContents		= strict;
};

class xs__enumeration
{ public:
	@xsd__string			value;
  public:
  	int				traverse(xs__schema&);
};

class xs__pattern
{ public:
	@xsd__string			value;
  public:
  	int				traverse(xs__schema&);
};

class xs__simpleContent
{ public:
	xs__extension			*extension;	// choice
 	xs__restriction			*restriction;	// choice
  public:
  	int				traverse(xs__schema&);
};

class xs__simpleType
{ public:
	@xsd__NMTOKEN			name;
 	xs__restriction			*restriction;	// choice
 	xs__list			*list;		// choice
 	xs__union			*union_;	// choice
  private:
	xs__schema			*schemaRef;
	int				level;
  public:
					xs__simpleType();
					~xs__simpleType();
  	int				traverse(xs__schema&);
	void				schemaPtr(xs__schema*);
	xs__schema			*schemaPtr() const;
	int				baseLevel();
};

class xs__extension
{ public:
	@xsd__QName			base;
	xs__group			*group;
	xs__all				*all;
	xs__choice			*choice;
	xs__sequence			*sequence;
	std::vector<xs__attribute>	attribute;
  private:
  	xs__simpleType			*simpleTypeRef;		// traverse() finds type
  	xs__complexType			*complexTypeRef;	// traverse() finds type
  public:
  	int				traverse(xs__schema&);
	void				simpleTypePtr(xs__simpleType*);
	void				complexTypePtr(xs__complexType*);
	xs__simpleType			*simpleTypePtr() const;
	xs__complexType			*complexTypePtr() const;
};

class xs__length
{ public:
	@xsd__string			value;
	@xsd__boolean			fixed;
};

class xs__whiteSpace
{ public:
	@xsd__string			value;
};

class xs__restriction
{ public:
	@xsd__QName			base;
	xs__group			*group;		// not used in <simpleType><restriction>
	xs__all				*all;		// not used in <simpleType><restriction>
	xs__choice			*choice;	// not used in <simpleType><restriction>
	xs__sequence			*sequence;	// not used in <simpleType><restriction>
	std::vector<xs__attribute>	attribute;	// not used in <simpleType><restriction>
	std::vector<xs__enumeration>	enumeration;
	std::vector<xs__pattern>	pattern;
	xs__whiteSpace			*whiteSpace;
	xs__length			*length;
	xs__length			*minLength;
	xs__length			*maxLength;
	xs__length			*precision;		// 2000 schema
	xs__length			*scale;			// 2000 schema
	xs__length			*totalDigits;		// 2001 schema
	xs__length			*fractionDigits;	// 2001 schema
	xs__length			*minInclusive;
	xs__length			*maxInclusive;
	xs__length			*minExclusive;
	xs__length			*maxExclusive;
  private:
  	xs__simpleType			*simpleTypeRef;		// traverse() finds type
  	xs__complexType			*complexTypeRef;	// traverse() finds type
  public:
  	int				traverse(xs__schema&);
	void				simpleTypePtr(xs__simpleType*);
	void				complexTypePtr(xs__complexType*);
	xs__simpleType			*simpleTypePtr() const;
	xs__complexType			*complexTypePtr() const;
};

class xs__list
{ public:
	@xsd__QName			itemType;
 	xs__restriction			*restriction;	// choice
	std::vector<xs__simpleType>	simpleType;	// choice
  private:
  	xs__simpleType			*itemTypeRef;
  public:
  	int				traverse(xs__schema&);
	void				itemTypePtr(xs__simpleType*);
	xs__simpleType			*itemTypePtr() const;
};

class xs__union
{ public:
	@xsd__NMTOKENS			memberTypes;	// check if NMTOKENS is ok???
	std::vector<xs__simpleType>	simpleType;
  public:
  	int				traverse(xs__schema&);
};

class xs__complexContent
{ public:
	@xsd__boolean			mixed			= false;
	xs__extension			*extension;
 	xs__restriction			*restriction;
  public:
  	int				traverse(xs__schema&);
};

class xs__complexType
{ public:
	@xsd__NMTOKEN			name;
	@xsd__boolean			abstract		= false;
	xs__simpleContent		*simpleContent;
	xs__complexContent		*complexContent;
	xs__all				*all;
	xs__choice			*choice;
	xs__sequence			*sequence;
	xs__any				*any;
	std::vector<xs__attribute>	attribute;
	std::vector<xs__attributeGroup>	attributeGroup;
	xs__anyAttribute		*anyAttribute;
  private:
	xs__schema			*schemaRef;
  	int				level;
  public:
					xs__complexType();
					~xs__complexType();
  	int				traverse(xs__schema&);
	void				schemaPtr(xs__schema*);
	xs__schema			*schemaPtr() const;
	int				baseLevel();
};

class xs__import
{ public:
	@xsd__anyURI			namespace_;
	@xsd__anyURI			schemaLocation;
  private:
  	xs__schema			*schemaRef;		// set by WSDL parser or via schemaLocation
  public:
					xs__import();
					~xs__import();
  	int				traverse(xs__schema&);
	void				schemaPtr(xs__schema*);
	xs__schema			*schemaPtr() const;
};

class xs__include
{ public:
	@xsd__anyURI			schemaLocation;
  private:
  	xs__schema			*schemaRef;		// set by WSDL parser or via schemaLocation
  public:
					xs__include();
					~xs__include();
  	int				traverse(xs__schema&);
	void				schemaPtr(xs__schema*);
	xs__schema			*schemaPtr() const;
};
	
enum xs__formChoice { unqualified, qualified };

class xs__schema
{ public:
	@xsd__anyURI			targetNamespace;
	@xsd__string			version;
	@enum xs__formChoice		attributeFormDefault	= unqualified;
	@enum xs__formChoice		elementFormDefault	= unqualified;
	std::vector<xs__include>	include;
	std::vector<xs__import>		import;
	std::vector<xs__attribute>	attribute;
	std::vector<xs__element>	element;
	std::vector<xs__group>		group;
	std::vector<xs__attributeGroup>	attributeGroup;
	std::vector<xs__simpleType>	simpleType;
	std::vector<xs__complexType>	complexType;
  	struct soap			*soap;
  private:
	SetOfString			builtinTypeSet;
	SetOfString			builtinElementSet;
	SetOfString			builtinAttributeSet;
  public:
					xs__schema();
					xs__schema(struct soap*);
					xs__schema(struct soap*, const char*);
					~xs__schema();
	int				traverse();
	int				read(const char*);
	int				error();
	void				print_fault();
	void				builtinType(const char*);
	void				builtinElement(const char*);
	void				builtinAttribute(const char*);
	const SetOfString&		builtinTypes() const;
	const SetOfString&		builtinElements() const;
	const SetOfString&		builtinAttributes() const;
	friend ostream&			operator<<(ostream&, const xs__schema&);
	friend istream&			operator>>(istream&, xs__schema&);
};

extern ostream &operator<<(ostream &o, const xs__schema &e);
extern istream &operator>>(istream &i, xs__schema &e);

