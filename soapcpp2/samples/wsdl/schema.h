//gsoap xs schema documentation:	XSD 2000/10 binding schema
//gsoap xs schema namespace:		http://www.w3.org/2000/10/XMLSchema

#import "stlvector.h"

extern class std::ostream;
extern class std::istream;

typedef char	*xsd__anyURI,
		*xsd__ID,
		*xsd__NCName,
		*xsd__NMTOKEN,
		*xsd__NMTOKENS,
		*xsd__QName,
		*xsd__string;
typedef bool	xsd__boolean;

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
	@xsd__boolean			nullable		= false;	// should be 'nillable' ???
	@xsd__string			minOccurs;		// xsd:nonNegativeInteger
	@xsd__string			maxOccurs;		// xsd:nonNegativeInteger|unbounded
	xs__simpleType			*simpleType;		// choice
	xs__complexType			*complexType;		// choice
  private:
  	const xs__element		*elementRef;		// traverse() finds ref
  	const xs__simpleType		*simpleTypeRef;		// traverse() finds type
  	const xs__complexType		*complexTypeRef;	// traverse() finds type
  public:
  	int				traverse(const xs__schema&);
	void				elementPtr(const xs__element*);
	void				simpleTypePtr(const xs__simpleType*);
	void				complexTypePtr(const xs__complexType*);
	const xs__element		*elementPtr() const;
	const xs__simpleType		*simpleTypePtr() const;
	const xs__complexType		*complexTypePtr() const;
};

enum xs__attribute_use { optional, prohibited, required, default_, fixed };

class xs__attribute
{ public:
	@xsd__NCName			name;
	@xsd__QName			ref;
	@xsd__QName			type;
	@enum xs__attribute_use		use			= optional;
	@xsd__string			value;
	xs__simpleType			*simpleType;
  private:
  	const xs__attribute		*attributeRef;		// traverse() finds ref
  	const xs__simpleType		*simpleTypeRef;		// traverse() finds type
  public:
  	int				traverse(const xs__schema&);
	void				attributePtr(const xs__attribute*);
	void				simpleTypePtr(const xs__simpleType*);
	const xs__attribute		*attributePtr() const;
	const xs__simpleType		*simpleTypePtr() const;
};

class xs__all
{ public:
	std::vector<xs__element>	element;
  public:
  	int				traverse(const xs__schema&);
};

class xs__any
{ public:
	std::vector<xs__element>	element;
  public:
  	int				traverse(const xs__schema&);
};

class xs__group
{ public:
	@xsd__NCName			name;
	@xsd__QName			ref;
	xs__all				*all;			// choice
	xs__choice			*choice;		// choice
	xs__sequence			*sequence;		// choice
  private:
  	const xs__group			*groupRef;		// traverse() finds ref
  public:
  	int				traverse(const xs__schema&);
	void				groupPtr(const xs__group*);
	const xs__group			*groupPtr() const;
};

class xs__choice
{ public:
	@xsd__string			minOccurs;		// xsd:nonNegativeInteger
	@xsd__string			maxOccurs;		// xsd:nonNegativeInteger|unbounded
	std::vector<xs__element>	element;
	std::vector<xs__group>		group;
	//std::vector<xs__choice>		choice;
	//std::vector<xs__sequence>	sequence;
	std::vector<xs__any>		any;
  public:
  	int				traverse(const xs__schema&);
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
  	int				traverse(const xs__schema&);
};

/* TODO TODO TODO
class xs__attributeGroup
{
}
*/

class xs__enumeration
{ public:
	@xsd__string			value;
  public:
  	int				traverse(const xs__schema&);
};

class xs__pattern
{ public:
	@xsd__string			value;
  public:
  	int				traverse(const xs__schema&);
};

class xs__simpleContent
{ public:
	xs__extension			*extension;	// choice
 	xs__restriction			*restriction;	// choice
  public:
  	int				traverse(const xs__schema&);
};

class xs__simpleType
{ public:
	@xsd__NMTOKEN			name;
 	xs__list			*list;		// choice
 	xs__restriction			*restriction;	// choice
 	xs__union			*union_;	// choice
  public:
  	int				traverse(const xs__schema&);
};

class xs__extension
{ public:
	@xsd__QName			base;
	xs__group			*group;
	xs__all				*all;
	xs__choice			*choice;
	xs__sequence			*sequence;
	std::vector<xs__attribute>	attribute;
  public:
  	int				traverse(const xs__schema&);
};

class xs__restriction
{ public:
	@xsd__QName			base;
	xs__group			*group;
	xs__all				*all;
	xs__choice			*choice;
	xs__sequence			*sequence;
	std::vector<xs__attribute>	attribute;
	std::vector<xs__enumeration>	enumeration;
	std::vector<xs__pattern>	pattern;
  public:
  	int				traverse(const xs__schema&);
};

class xs__list
{ public:
	std::vector<xs__simpleType>	simpleType;
  public:
  	int				traverse(const xs__schema&);
};

class xs__union
{ public:
	std::vector<xs__simpleType>	simpleType;
  public:
  	int				traverse(const xs__schema&);
};

class xs__complexContent
{ public:
	@xsd__boolean			mixed			= false;
	xs__extension			*extension;
 	xs__restriction			*restriction;
  public:
  	int				traverse(const xs__schema&);
};

class xs__complexType
{ public:
	@xsd__NMTOKEN			name;
	xs__simpleContent		*simpleContent;
	xs__complexContent		*complexContent;
	xs__all				*all;
	xs__choice			*choice;
	xs__sequence			*sequence;
	xs__any				*any;
  public:
  	int				traverse(const xs__schema&);
};

class xs__import
{ public:
	@xsd__anyURI			namespace_;
	@xsd__anyURI			schemaLocation;
  private:
  	const xs__schema		*schemaRef;		// set by WSDL parser or via schemaLocation
  public:
  	int				traverse(const xs__schema&);
	void				schemaPtr(const xs__schema*);
	const xs__schema		*schemaPtr() const;
};

class xs__include
{ public:
	@xsd__anyURI			schemaLocation;
  private:
  	const xs__schema		*schemaRef;		// set by WSDL parser or via schemaLocation
  public:
  	int				traverse(const xs__schema&);
	void				schemaPtr(const xs__schema*);
	const xs__schema		*schemaPtr() const;
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
	std::vector<xs__simpleType>	simpleType;
	std::vector<xs__complexType>	complexType;
  private:
  	struct soap			*soap;
  public:
					xs__schema();
					xs__schema(struct soap*);
					xs__schema(const char*);
					~xs__schema();
	int				traverse();
	int				error();
	void				print_fault();
	friend xs__schema		*soap_instantiate_xs__schema(struct soap*, int, const char*, const char*, size_t*);	// soap struct is private, so gSOAP's instantiator must be a friend
	friend std::ostream&		operator<<(std::ostream&, const xs__schema&);
	friend std::istream&		operator>>(std::istream&, xs__schema&);
};

extern std::ostream &operator<<(std::ostream &o, const xs__schema &e);
extern std::istream &operator>>(std::istream &i, xs__schema &e);

