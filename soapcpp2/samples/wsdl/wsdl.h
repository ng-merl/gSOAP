//gsoap wsdl schema documentation:	WSDL 1.1 binding schema
//gsoap wsdl schema namespace:		http://schemas.xmlsoap.org/wsdl/

#import "stlvector.h"
#import "soap.h"
#import "http.h"

class wsdl__definitions;		// forward declaration

class wsdl__import
{ public:
	@xsd__anyURI			namespace_;
	@xsd__anyURI			location;
  private:
  	const wsdl__definitions		*definitionsRef;	// not yet handled
  public:
  	int				traverse(const wsdl__definitions&);
	void				definitionsPtr(const wsdl__definitions*);
	const wsdl__definitions		*definitionsPtr() const;
};

class wsdl__types
{ public:
	xsd__string			documentation;		// <wsdl:documentation>?
	std::vector<xs__schema>		xs__schema_; 		// <xs:schema>*
  public:
	int				traverse(const wsdl__definitions&);
};

class wsdl__part
{ public:
	@xsd__NMTOKEN			name;
	@xsd__QName			element;
	@xsd__QName			type;
  private:
  	const xs__element		*elementRef;		// traverse() finds element
  	const xs__simpleType		*simpleTypeRef;		// traverse() finds simpleType
  	const xs__complexType		*complexTypeRef;	// traverse() finds complexType
  public:
  	int				traverse(const wsdl__definitions&);
	void				elementPtr(const xs__element*);
	void				simpleTypePtr(const xs__simpleType*);
	void				complexTypePtr(const xs__complexType*);
	const xs__element		*elementPtr() const;
	const xs__simpleType		*simpleTypePtr() const;
	const xs__complexType		*complexTypePtr() const;
};

class wsdl__message
{ public:
	@xsd__NMTOKEN			name;
	xsd__string			documentation;		// <wsdl:documentation>?
	std::vector<wsdl__part>		part;			// <wsdl:part>*
  public:
  	int				traverse(const wsdl__definitions&);
};

class wsdl__input
{ public:
	@xsd__NMTOKEN			name;
	@xsd__QName			message;
	xsd__string			documentation;		// <wsdl:documentation>?
  private:
  	const wsdl__message		*messageRef;		// traverse() finds message
  public:
  	int				traverse(const wsdl__definitions&);
	void				messagePtr(const wsdl__message*);
	const wsdl__message		*messagePtr() const;
};

class wsdl__output
{ public:
	@xsd__NMTOKEN			name;
	@xsd__QName			message;
	xsd__string			documentation;		// <wsdl:documentation>?
  private:
  	const wsdl__message		*messageRef;		// traverse() finds message
  public:
  	int				traverse(const wsdl__definitions&);
	void				messagePtr(const wsdl__message*);
	const wsdl__message		*messagePtr() const;
};

class wsdl__fault
{ public:
	@xsd__NMTOKEN			name;
	@xsd__QName			message;
	xsd__string			documentation;		// <wsdl:documentation>?
  private:
  	const wsdl__message		*messageRef;		// traverse() finds message
  public:
  	int				traverse(const wsdl__definitions&);
	void				messagePtr(const wsdl__message*);
	const wsdl__message		*messagePtr() const;
};

class wsdl__operation
{ public:
	@xsd__NMTOKEN			name;
	@xsd__string			parameterOrder;
	xsd__string			documentation;		// <wsdl:documentation>?
	wsdl__input			*input;			// <wsdl:input>?
	wsdl__output			*output;		// <wsdl:output>?
	std::vector<wsdl__fault>	fault;			// <wsdl:fault>*
  public:
  	int				traverse(const wsdl__definitions&);
};

class wsdl__portType
{ public:
	@xsd__NMTOKEN			name;
	xsd__string			documentation;		// <wsdl:documentation>?
	std::vector<wsdl__operation>	operation;		// <wsdl:operation>*
  public:
  	int				traverse(const wsdl__definitions&);
};	

class wsdl__ext_input			// extensibility element
{ public:
	xsd__string			documentation;		// <wsdl:documentation>?
	soap__body			*soap__body_;		// <soap:body>?
	std::vector<soap__header>	soap__header_;		// <soap:header>*
	std::vector<soap__headerfault>	soap__headerfault_;	// <soap:headerfault>*
  public:
  	int				traverse(const wsdl__definitions&);
};

class wsdl__ext_output			// extensibility element
{ public:
	xsd__string			documentation;		// <wsdl:documentation>?
	soap__body			*soap__body_;		// <soap:body>?
	std::vector<soap__header>	soap__header_;		// <soap:header>*
	std::vector<soap__headerfault>	soap__headerfault_;	// <soap:headerfault>*
  public:
  	int				traverse(const wsdl__definitions&);
};

class wsdl__ext_fault			// extensibility element
{ public:
	@xsd__NMTOKEN			name;
	xsd__string			documentation;		// <wsdl:documentation>?
	soap__fault			*soap__fault_;		// <soap:fault>?
  public:
  	int				traverse(const wsdl__definitions&);
};

class wsdl__binding_operation
{ public:
	@xsd__NMTOKEN			name;
	xsd__string			documentation;		// <wsdl:documentation>?
	soap__operation			*soap__operation_;	// <soap:operation>?
	http__operation			*http__operation_;	// <http:operation>?
	wsdl__ext_input			*input;			// <wsdl:input>?
	wsdl__ext_output		*output;		// <wsdl:output>?
	std::vector<wsdl__ext_fault>	fault;			// <wsdl:fault>*
  private:
  	const wsdl__operation		*operationRef;		// traverse() finds operation in <wsdl:portType>
  public:
  	int				traverse(const wsdl__definitions&, const wsdl__portType*);
	void				operationPtr(const wsdl__operation*);
	const wsdl__operation		*operationPtr() const;
};

class wsdl__binding
{ public:
	@xsd__NMTOKEN			name;
	@xsd__QName			type;
	xsd__string			documentation;		// <wsdl:documentation>?
	soap__binding			*soap__binding_;	// <soap:binding>?
	http__binding			*http__binding_;	// <http:binding>?
	std::vector<wsdl__binding_operation>
					operation;		// <wsdl:operation>*
  private:
	const wsdl__portType		*portTypeRef;		// traverse() finds portType
  public:
	int				traverse(const wsdl__definitions&);
	void				portTypePtr(const wsdl__portType*);
	const wsdl__portType		*portTypePtr() const;
};

class wsdl__port
{ public:
	@xsd__NMTOKEN			name;
	@xsd__QName			binding;
	xsd__string			documentation;		// <wsdl:documentation>?
	soap__address			*soap__address_;	// <soap:address>?
	http__address			*http__address_;	// <http:address>?
  private:
	const wsdl__binding		*bindingRef;		// traverse() finds binding
  public:
	int				traverse(const wsdl__definitions&);
	void				bindingPtr(const wsdl__binding*);
	const wsdl__binding		*bindingPtr() const;
};

class wsdl__service
{ public:
	@xsd__NMTOKEN			name;
	xsd__string			documentation;		// <wsdl:documentation>?
	std::vector<wsdl__port>		port;			// <wsdl:port>*
  public:
	int				traverse(const wsdl__definitions&);
};

class wsdl__definitions
{ public:
	@xsd__NMTOKEN			name;
	@xsd__anyURI			targetNamespace;
	std::vector<wsdl__import>	import;			// <wsdl:import>*
	xsd__string			documentation;		// <wsdl:documentation>?
	wsdl__types			*types;			// <wsdl:types>?
	std::vector<wsdl__message>	message;		// <wsdl:message>*
	std::vector<wsdl__portType>	portType;		// <wsdl:portType>*
	std::vector<wsdl__binding>	binding;		// <wsdl:binding>*
	std::vector<wsdl__service>	service;		// <wsdl:service>*
  private:
	struct soap			*soap;
  public:
					wsdl__definitions();
					wsdl__definitions(struct soap*);
					wsdl__definitions(const char*);
					~wsdl__definitions();
	int				get(struct soap*);	// gSOAP getter is triggered after parsing
	int				traverse();
	int				error();
	void				print_fault();
	friend wsdl__definitions	*soap_instantiate_wsdl__definitions(struct soap*, int, const char*, const char*, size_t*);	// soap struct is private, so gSOAP's instantiator must be a friend
	friend std::ostream&		operator<<(std::ostream&, const wsdl__definitions&);
	friend std::istream&		operator>>(std::istream&, wsdl__definitions&);
};

extern std::ostream &operator<<(std::ostream &o, const wsdl__definitions &e);
extern std::istream &operator>>(std::istream &i, wsdl__definitions &e);
