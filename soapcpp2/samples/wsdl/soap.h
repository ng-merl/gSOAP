//gsoap soap schema documentation:	WSDL/SOAP binding schema
//gsoap soap schema namespace:		http://schemas.xmlsoap.org/wsdl/soap/

#import "stlvector.h"
#import "schema.h"

enum soap__styleChoice { rpc, document };

class soap__binding
{ public:
	@xsd__anyURI			transport;
 	@enum soap__styleChoice		style;
};

class soap__operation
{ public:
	@xsd__anyURI			soapAction;
	@enum soap__styleChoice		style;
};

enum soap__useChoice { literal, encoded };

class soap__body
{ public:
	@xsd__anyURI			encodingStyle;
 	@xsd__NMTOKENS			parts;
	@enum soap__useChoice		use;
	@xsd__anyURI			namespace_;
};

class soap__fault
{ public:
	@xsd__anyURI			encodingStyle;
	@enum soap__useChoice		use;
	@xsd__anyURI			namespace_;
};

class soap__headerfault
{ public:
	@xsd__QName			message;
 	@xsd__NMTOKEN			part;
	@enum soap__useChoice		use;
	@xsd__anyURI			encodingStyle;
	@xsd__anyURI			namespace_;
};

class soap__header
{ public:
	@xsd__QName			message;
 	@xsd__NMTOKEN			part;
	@enum soap__useChoice		use;
	@xsd__anyURI			encodingStyle;
	@xsd__anyURI			namespace_;
	std::vector<soap__headerfault>	headerfault;		// <soap:headerfault>*
};

class soap__address
{ public:
	@xsd__anyURI			location;
};
