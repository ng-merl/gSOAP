//gsoap http schema documentation:	WSDL/HTTP binding schema
//gsoap http schema namespace:		http://schemas.xmlsoap.org/wsdl/http/

#import "schema.h"

class http__address
{ public:
	@xsd__anyURI		location;
};

class http__binding
{ public:
	@xsd__NMTOKEN		verb;
};

class http__operation
{ public:
	@xsd__anyURI		location;
};
