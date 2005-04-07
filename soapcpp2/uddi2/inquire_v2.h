/** @mainpage gSOAP UDDI v2

@section UDDI2_CLASSES Classes

- @ref uddi2__find_USCOREbinding
- @ref uddi2__find_USCOREbusiness
- @ref uddi2__find_USCORErelatedBusinesses
- @ref uddi2__find_USCOREservice
- @ref uddi2__find_USCOREtModel
- @ref uddi2__get_USCOREbindingDetail
- @ref uddi2__get_USCOREbusinessDetail
- @ref uddi2__get_USCOREbusinessDetailExt
- @ref uddi2__get_USCOREserviceDetail
- @ref uddi2__get_USCOREtModelDetail

@section UDDI2_BINDINGS Binding Reference

- @ref InquireSoap
- @ref PublishSoap

*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:find_binding
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__find_USCOREbinding
@brief

Represents a request to locate bindings that meet the specified requirements.
*/

/**
@fn uddi2__find_USCOREbinding::uddi2__find_USCOREbinding(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the find_USCOREbinding class.
*/

/**
@fn uddi2__find_USCOREbinding::uddi2__find_USCOREbinding(struct soap *soap, const char *tModelKey)
@brief
@param[in] soap gSOAP context
@param[in] tModelKey string

Creates an instance of the find_USCOREbinding class using the specified tModel
key.
*/

/**
@fn uddi2__find_USCOREbinding::uddi2__find_USCOREbinding(struct soap *soap, std::vector<char*> tModelKeys)
@brief
@param[in] soap gSOAP context
@param[in] tModelKeys collection of tModel key strings

Creates an instance of the find_USCOREbinding class using the specified tModel
keys.
*/

/**
@fn uddi2__bindingDetail* uddi2__find_USCOREbinding::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__bindingDetail object or NULL on error

Send a request to a UDDI server to find binding details.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:find_business
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__find_USCOREbusiness
@brief

Represents a request to locate businesses that meet specific requirements. When
the send method is called, the instance returns a uddi2__businessList object
that contains a list of business that matched the search criteria.

See also @ref example2.
*/

/**
@fn uddi2__find_USCOREbusiness::uddi2__find_USCOREbusiness(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the find_USCOREbusiness class.
*/

/**
@fn uddi2__find_USCOREbusiness::uddi2__find_USCOREbusiness(struct soap *soap, const char *name)
@brief
@param[in] soap gSOAP context

Creates an instance of the find_USCOREbusiness class using the specified name
of the requested business.
*/

/**
@fn uddi2__find_USCOREbusiness::uddi2__find_USCOREbusiness(struct soap *soap, std::vector<uddi2__keyedReference*> keyedReferences)
@brief
@param[in] soap gSOAP context

Creates an instance of the find_USCOREbusiness class using the specified
category references.
*/

/**
@fn uddi2__find_USCOREbusiness::uddi2__find_USCOREbusiness(struct soap *soap, std::vector<char*> tModelKeys)
@brief
@param[in] soap gSOAP context

Creates an instance of the find_USCOREbusiness class using the specified tModel
keys.
*/

/**
@fn uddi2__businessList *uddi2__find_USCOREbusiness::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__businessList object or NULL on error

Send a request to a UDDI server to find a list of businesses.

See also @ref example2.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:find_relatedBusinesses
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__find_USCORErelatedBusinesses
@brief

Represents a request to locate businesses that are related to a specific
business. When the send method is called, the instance returns a
uddi2__relatedBusinessList object that contains a list of business that matched
the relationship set.
*/

/**
@fn uddi2__find_USCORErelatedBusinesses::uddi2__find_USCORErelatedBusinesses(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the find_USCORErelatedBusiness class.
*/

/**
@fn uddi2__find_USCORErelatedBusinesses::uddi2__find_USCORErelatedBusinesses(struct soap *soap, const char *businessKey)
@brief
@param[in] soap gSOAP context
@param[in] businessKey string

Creates an instance of the find_USCORErelatedBusiness class using the specified
business key.
*/

/**
@fn uddi2__relatedBusinessesList *uddi2__find_USCORErelatedBusinesses::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__relatedBusinessList object or NULL on error

Send a request to a UDDI server to find a list of related businesses.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:find_service
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__find_USCOREservice
@brief

Represents a request to locate services that meet the specified requirements.

See also @ref example1.
*/

/**
@fn uddi2__find_USCOREservice::uddi2__find_USCOREservice(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the find_USCOREservice class.
*/

/**
@fn uddi2__find_USCOREservice::uddi2__find_USCOREservice(struct soap *soap, const char *name)
@brief
@param[in] soap gSOAP context
@param[in] name of the service

Creates an instance of the find_USCOREservice class using the specified name.
*/

/**
@fn uddi2__find_USCOREservice::uddi2__find_USCOREservice(struct soap *soap, std::vector<uddi2__keyedReference*> keyedReferences)
@brief
@param[in] soap gSOAP context
@param[in] keyedReferences collection of category keys

Creates an instance of the find_USCOREservice class using the specified
category keys.
*/

/**
@fn uddi2__find_USCOREservice::uddi2__find_USCOREservice(struct soap *soap, std::vector<char*> tModelKeys)
@brief
@param[in] soap gSOAP context
@param[in] tModelKeys collection of tModel key strings

Creates an instance of the find_USCOREservice class using the specified tModel
keys.
*/

/**
@fn uddi2__serviceList* uddi2__find_USCOREservice::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__serviceList object or NULL on error

Send a request to a UDDI server to find a list of services.

See also @ref example1.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:find_tModel
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__find_USCOREtModel
@brief

Represents a request to locate a list of tModel entries that match a set of
specific criteria. The result of a search is a uddi2__tModelList object that
contains information about registered tModel data that matches the criteria.
*/

/**
@fn uddi2__find_USCOREtModel::uddi2__find_USCOREtModel(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the find_USCOREtModel class.
*/

/**
@fn uddi2__find_USCOREtModel::uddi2__find_USCOREtModel(struct soap *soap, const char *name)
@brief
@param[in] soap gSOAP context
@param[in] name of the tModel

Creates an instance of the find_USCOREtModel class using the speficied tModel
name.
*/

/**
@fn uddi2__find_USCOREtModel::uddi2__find_USCOREtModel(struct soap *soap, std::vector<uddi2__keyedReference*> keyedReferences)
@brief
@param[in] soap gSOAP context
@param[in] keyedReferences collection of category keys

Creates an instance of the find_USCOREtModel class using the specified category
keys.
*/

/**
@fn uddi2__tModelList* uddi2__find_USCOREtModel::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__tModelList object or NULL on error

Send a request to a UDDI server to find a tModelList.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:get_bindingDetail
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__get_USCOREbindingDetail
@brief

Represents a request to get binding details from a UDDI server.
*/

/**
@fn uddi2__get_USCOREbindingDetail::uddi2__get_USCOREbindingDetail(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the get_USCOREbindingDetail class.
*/

/**
@fn uddi2__get_USCOREbindingDetail::uddi2__get_USCOREbindingDetail(struct soap *soap, const char *bindingKey)
@brief
@param[in] soap gSOAP context
@param[in] bindingKey string

Creates an instance of the get_USCOREbindingDetail class using the specified
binding key.
*/

/**
@fn uddi2__get_USCOREbindingDetail::uddi2__get_USCOREbindingDetail(struct soap *soap, std::vector<char*> bindingKeys)
@brief
@param[in] soap gSOAP context
@param[in] bindingKeys collection of key strings

Creates an instance of the get_USCOREbindingDetail class using the specified
collection of binding keys.
*/

/**
@fn uddi2__bindingDetail* uddi2__get_USCOREbindingDetail::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__bindingDetail object or NULL on error

Send a request to a UDDI server to get the binding details.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:get_businessDetail
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__get_USCOREbusinessDetail
@brief

Represents a request to get business details from a UDDI server.
*/

/**
@fn uddi2__get_USCOREbusinessDetail::uddi2__get_USCOREbusinessDetail(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the get_USCOREbusinessDetail class.
*/

/**
@fn uddi2__get_USCOREbusinessDetail::uddi2__get_USCOREbusinessDetail(struct soap *soap, const char *businessKey)
@brief
@param[in] soap gSOAP context
@param[in] businessKey string

Creates an instance of the get_USCOREbusinessDetail class using the specified
business key.
*/

/**
@fn uddi2__get_USCOREbusinessDetail::uddi2__get_USCOREbusinessDetail(struct soap *soap, std::vector<char*> businessKeys)
@brief
@param[in] soap gSOAP context
@param[in] businessKeys collection of key strings

Creates an instance of the get_USCOREbusinessDetail class using the specified
collection of business keys.
*/

/**
@fn uddi2__businessDetail* uddi2__get_USCOREbusinessDetail::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__businessDetail object or NULL on error

Send a request to a UDDI server to get the business details.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:get_businessDetailExt
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__get_USCOREbusinessDetailExt
@brief

Represents a request to get business details from a UDDI server.
*/

/**
@fn uddi2__get_USCOREbusinessDetailExt::uddi2__get_USCOREbusinessDetailExt(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the get_USCOREbusinessDetailExt class.
*/

/**
@fn uddi2__get_USCOREbusinessDetailExt::uddi2__get_USCOREbusinessDetailExt(struct soap *soap, const char *businessKey)
@brief
@param[in] soap gSOAP context
@param[in] businessKey string

Creates an instance of the get_USCOREbusinessDetailExt class using the specified
business key.
*/

/**
@fn uddi2__get_USCOREbusinessDetailExt::uddi2__get_USCOREbusinessDetailExt(struct soap *soap, std::vector<char*> businessKeys)
@brief
@param[in] soap gSOAP context
@param[in] businessKeys collection of key strings

Creates an instance of the get_USCOREbusinessDetailExt class using the specified
collection of business keys.
*/

/**
@fn uddi2__businessDetailExt* uddi2__get_USCOREbusinessDetailExt::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__businessDetailExt object or NULL on error

Send a request to a UDDI server to get the business details.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:get_serviceDetail
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__get_USCOREserviceDetail
@brief

Represents a request to get service details from a UDDI server.
*/

/**
@fn uddi2__get_USCOREserviceDetail::uddi2__get_USCOREserviceDetail(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the get_USCOREserviceDetail class.
*/

/**
@fn uddi2__get_USCOREserviceDetail::uddi2__get_USCOREserviceDetail(struct soap *soap, const char *serviceKey)
@brief
@param[in] soap gSOAP context
@param[in] serviceKey string

Creates an instance of the get_USCOREserviceDetail class using the specified
service key.
*/

/**
@fn uddi2__get_USCOREserviceDetail::uddi2__get_USCOREserviceDetail(struct soap *soap, std::vector<char*> serviceKeys)
@brief
@param[in] soap gSOAP context
@param[in] serviceKeys collection of key strings

Creates an instance of the get_USCOREserviceDetail class using the specified
collection of service keys.
*/

/**
@fn uddi2__serviceDetail* uddi2__get_USCOREserviceDetail::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__serviceDetail object or NULL on error

Send a request to a UDDI server to get the service details.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	uddi2:get_tModelDetail
//
////////////////////////////////////////////////////////////////////////////////

/**
@class uddi2__get_USCOREtModelDetail
@brief

Represents a request to get tModel details from a UDDI server.
*/

/**
@fn uddi2__get_USCOREtModelDetail::uddi2__get_USCOREtModelDetail(struct soap *soap)
@brief
@param[in] soap gSOAP context

Creates an instance of the get_USCOREtModelDetail class.
*/

/**
@fn uddi2__get_USCOREtModelDetail::uddi2__get_USCOREtModelDetail(struct soap *soap, const char *tModelKey)
@brief
@param[in] soap gSOAP context
@param[in] tModelKey string

Creates an instance of the get_USCOREtModelDetail class using the specified
tModel key.
*/

/**
@fn uddi2__get_USCOREtModelDetail::uddi2__get_USCOREtModelDetail(struct soap *soap, std::vector<char*> tModelKeys)
@brief
@param[in] soap gSOAP context
@param[in] tModelKeys collection of key strings

Creates an instance of the get_USCOREtModelDetail class using the specified
collection of tModel keys.
*/

/**
@fn uddi2__tModelDetail* uddi2__get_USCOREtModelDetail::send(const char *endpoint)
@brief
@param[in] endpoint URL of the UDDI server
@return A pointer to a uddi2__tModelDetail object or NULL on error

Send a request to a UDDI server to get the tModel details.
*/

////////////////////////////////////////////////////////////////////////////////
//
//	Code Examples
//
////////////////////////////////////////////////////////////////////////////////

/**
@page example1 Code Example 1: Finding a service

This example shows you how to find Web services. In this case, the example
finds Web services with names that begin with the word "magic".

@code
#include "inqH.h"

int main(int argc, char **argv)
{ 
  char *search_string = "magic";

  if (argc > 1)
    search_string = argv[1];

  // Create a gSOAP context
  struct soap *soap = soap_new();

  // Create an object to find a business
  uddi2__find_USCOREservice fs(soap, search_string);

  // Send the request
  uddi2__serviceList *sl = fs.send("http://uddi.xmethods.net/inquire");

  // Check if result is OK
  if (!sl)
    soap_print_fault(soap, stderr);

  // If OK, report the service name(s) and unique identification keys
  else if (sl->serviceInfos)
  {
    std::cout << "Search results on " << search_string << ":" << std::endl << std::endl;

    for (std::vector<uddi2__serviceInfo*>::const_iterator si = sl->serviceInfos->serviceInfo.begin(); si != sl->serviceInfos->serviceInfo.end(); ++si)
    {
      // Report serviceKey and businessKey
      std::cout << "serviceKey=" << (*si)->serviceKey << std::endl << "businessKey=" << (*si)->businessKey << std::endl;

      // Report names
      for (std::vector<uddi2__name*>::const_iterator n = (*si)->name.begin(); n != (*si)->name.end(); ++n)
        std::cout << "name=" << (*n)->__item << std::endl;

      std::cout << std::endl;
    }
  }

  // Remove deserialized objects
  soap_destroy(soap);

  // Remove temporary data
  soap_end(soap);

  // Detach and free context
  soap_done(soap);
  free(soap);

  return 0;
}
@endcode

To compile:
- wsdl2h -tuddi2map.dat inquire_v2.wsdl
- soapcpp2 -I.. -pinq inquire_v2.h
- g++ -DWITH_NONAMESPACES -I.. -o example1 example1.cpp inquire_v2.cpp inqC.cpp inqClient.cpp ../stdsoap2.cpp
*/

/**
@page example2 Code Example 2: Finding a business

This example shows you how to find a business from a UDDI server.

@code
#include "inqH.h"

int main(int argc, char **argv)
{ 
  char *search_string = "xmethods";

  if (argc > 1)
    search_string = argv[1];

  // Create a gSOAP context
  struct soap *soap = soap_new();

  // Create an object to find a business
  uddi2__find_USCOREbusiness fb(soap, search_string);

  // Send the request
  uddi2__businessList *bl = fb.send("http://uddi.xmethods.net/inquire");

  // Check if result is OK
  if (!bl)
    soap_print_fault(soap, stderr);

  // If OK, report the business name(s) and unique identification keys
  else if (bl->businessInfos)
  {
    std::cout << "Search results on " << search_string << ":" << std::endl << std::endl;

    for (std::vector<uddi2__businessInfo*>::const_iterator bi = bl->businessInfos->businessInfo.begin(); bi != bl->businessInfos->businessInfo.end(); ++bi)
    {
      // Report businessKey
      std::cout << "businessKey=" << (*bi)->businessKey << std::endl;

      // Report names
      for (std::vector<uddi2__name*>::const_iterator n = (*bi)->name.begin(); n != (*bi)->name.end(); ++n)
        std::cout << "name=" << (*n)->__item << std::endl;

      std::cout << std::endl;
    }
  }

  // Remove deserialized objects
  soap_destroy(soap);

  // Remove temporary data
  soap_end(soap);

  // Detach and free context
  soap_done(soap);
  free(soap);

  return 0;
}
@endcode

To compile:
- wsdl2h -tuddi2map.dat inquire_v2.wsdl
- soapcpp2 -I.. -pinq inquire_v2.h
- g++ -DWITH_NONAMESPACES -I.. -o example2 example2.cpp inquire_v2.cpp inqC.cpp inqClient.cpp ../stdsoap2.cpp
*/
/* inquire_v2.h
   Generated by wsdl2h 1.2.0 from inquire_v2.wsdl and uddi2map.dat
   2005-04-01 00:21:50 GMT
   Copyright (C) 2001-2005 Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL or Genivia's license for commercial use.
*/

/* NOTE:

 - Compile this file with soapcpp2 to complete the code generation process.
 - Use wsdl2h option -l to view the software license terms.
 - Use wsdl2h options -c and -s to generate pure C code or C++ code without STL.
 - To build with STL, stlvector.h from the gSOAP distribution must be in the
   current directory. Or use soapcpp2 option -I<path> with path to stlvector.h.
 - Use typemap.dat to control schema namespace bindings and type mappings.
   It is strongly recommended to customize the names of the namespace prefixes
   generated by wsdl2h. To do so, modify the prefix bindings in the Namespaces
   section below and add the modified lines to typemap.dat to rerun wsdl2h.
 - Use Doxygen (www.doxygen.org) to browse this file.

*/

/******************************************************************************\
 *                                                                            *
 * urn:uddi-org:inquiry_v2                                                    *
 *                                                                            *
\******************************************************************************/

/* WSDL Documentation:


		Copyright (c) 2000 - 2002 by Accenture, Ariba, Inc., Commerce One, Inc. 
		Fujitsu Limited, Hewlett-Packard Company, i2 Technologies, Inc., 
		Intel Corporation, International Business Machines Corporation,  
		Microsoft Corporation, Oracle Corporation, SAP AG, Sun Microsystems, Inc., 
		and VeriSign, Inc.  All Rights Reserved.
		
		WSDL Service Interface for UDDI Inquiry API V2.0    
		
		This WSDL document defines the inquiry API calls for interacting with 
		the UDDI registry.  The complete UDDI API specification is available
		at http://www.uddi.org/specification.html.
	
*/

//gsoapopt w

#import "stlvector.h"

/******************************************************************************\
 *                                                                            *
 * Schema Namespaces                                                          *
 *                                                                            *
\******************************************************************************/


/* NOTE:

It is strongly recommended to customize the names of the namespace prefixes
generated by wsdl2h. To do so, modify the prefix bindings below and add the
modified lines to typemap.dat to rerun wsdl2h:

inq2 = urn:uddi-org:inquiry_v2
uddi2 = urn:uddi-org:api_v2

*/

//gsoap inq2  schema namespace:	urn:uddi-org:inquiry_v2
//gsoap uddi2 schema namespace:	urn:uddi-org:api_v2
//gsoap inq2  schema form:	unqualified
//gsoap uddi2 schema elementForm:	qualified
//gsoap uddi2 schema attributeForm:	unqualified

/******************************************************************************\
 *                                                                            *
 * Schema Types                                                               *
 *                                                                            *
\******************************************************************************/


/// Primitive built-in type "xs:NMTOKEN"
typedef char* xsd__NMTOKEN;

/// Built-in attribute "xml:lang"
typedef char* xml__lang;

class uddi2__accessPoint;
class uddi2__address;
class uddi2__addressLine;
class uddi2__assertionStatusItem;
class uddi2__bindingTemplate;
class uddi2__bindingTemplates;
class uddi2__businessEntity;
class uddi2__businessEntityExt;
class uddi2__businessInfo;
class uddi2__businessInfos;
class uddi2__businessService;
class uddi2__businessServices;
class uddi2__categoryBag;
class uddi2__contact;
class uddi2__contacts;
class uddi2__description;
class uddi2__discoveryURL;
class uddi2__discoveryURLs;
class uddi2__dispositionReport;
class uddi2__email;
class uddi2__errInfo;
class uddi2__findQualifiers;
class uddi2__hostingRedirector;
class uddi2__identifierBag;
class uddi2__instanceDetails;
class uddi2__keyedReference;
class uddi2__keysOwned;
class uddi2__name;
class uddi2__overviewDoc;
class uddi2__phone;
class uddi2__publisherAssertion;
class uddi2__relatedBusinessInfo;
class uddi2__relatedBusinessInfos;
class uddi2__result;
class uddi2__serviceInfo;
class uddi2__serviceInfos;
class uddi2__sharedRelationships;
class uddi2__tModel;
class uddi2__tModelBag;
class uddi2__tModelInfo;
class uddi2__tModelInfos;
class uddi2__tModelInstanceDetails;
class uddi2__tModelInstanceInfo;
class uddi2__add_USCOREpublisherAssertions;
class uddi2__delete_USCOREbinding;
class uddi2__delete_USCOREbusiness;
class uddi2__delete_USCOREpublisherAssertions;
class uddi2__delete_USCOREservice;
class uddi2__delete_USCOREtModel;
class uddi2__discard_USCOREauthToken;
class uddi2__find_USCOREbinding;
class uddi2__find_USCOREbusiness;
class uddi2__find_USCORErelatedBusinesses;
class uddi2__find_USCOREservice;
class uddi2__find_USCOREtModel;
class uddi2__get_USCOREassertionStatusReport;
class uddi2__get_USCOREauthToken;
class uddi2__get_USCOREbindingDetail;
class uddi2__get_USCOREbusinessDetail;
class uddi2__get_USCOREbusinessDetailExt;
class uddi2__get_USCOREpublisherAssertions;
class uddi2__get_USCOREregisteredInfo;
class uddi2__get_USCOREserviceDetail;
class uddi2__get_USCOREtModelDetail;
class uddi2__save_USCOREbinding;
class uddi2__save_USCOREbusiness;
class uddi2__save_USCOREservice;
class uddi2__save_USCOREtModel;
class uddi2__set_USCOREpublisherAssertions;
class uddi2__validate_USCOREvalues;
class uddi2__assertionStatusReport;
class uddi2__authToken;
class uddi2__bindingDetail;
class uddi2__businessDetail;
class uddi2__businessDetailExt;
class uddi2__businessList;
class uddi2__publisherAssertions;
class uddi2__registeredInfo;
class uddi2__relatedBusinessesList;
class uddi2__serviceDetail;
class uddi2__serviceList;
class uddi2__tModelDetail;
class uddi2__tModelList;

/// Schema urn:uddi-org:api_v2 simpleType "bindingKey" restriction of xs:string

typedef char* uddi2__bindingKey;

/// Schema urn:uddi-org:api_v2 simpleType "businessKey" restriction of xs:string

typedef char* uddi2__businessKey;

/// Schema urn:uddi-org:api_v2 simpleType "serviceKey" restriction of xs:string

typedef char* uddi2__serviceKey;

/// Schema urn:uddi-org:api_v2 simpleType "tModelKey" restriction of xs:string

typedef char* uddi2__tModelKey;

/// Schema urn:uddi-org:api_v2 simpleType "direction" restriction of xs:NMTOKEN

/// Note: enum values are prefixed with 'uddi2__direction' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum uddi2__direction
{	uddi2__direction__fromKey,	///< xs:NMTOKEN value="fromKey"
	uddi2__direction__toKey,	///< xs:NMTOKEN value="toKey"
};

/// Schema urn:uddi-org:api_v2 simpleType "truncated" restriction of xs:NMTOKEN

/// Note: enum values are prefixed with 'uddi2__truncated' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum uddi2__truncated
{	uddi2__truncated__true_,	///< xs:NMTOKEN value="true"
	uddi2__truncated__false_,	///< xs:NMTOKEN value="false"
};

/// Schema urn:uddi-org:api_v2 simpleType "URLType" restriction of xs:NMTOKEN

/// Note: enum values are prefixed with 'uddi2__URLType' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum uddi2__URLType
{	uddi2__URLType__mailto,	///< xs:NMTOKEN value="mailto"
	uddi2__URLType__http,	///< xs:NMTOKEN value="http"
	uddi2__URLType__https,	///< xs:NMTOKEN value="https"
	uddi2__URLType__ftp,	///< xs:NMTOKEN value="ftp"
	uddi2__URLType__fax,	///< xs:NMTOKEN value="fax"
	uddi2__URLType__phone,	///< xs:NMTOKEN value="phone"
	uddi2__URLType__other,	///< xs:NMTOKEN value="other"
};

/// Schema urn:uddi-org:api_v2 simpleType "keyType" restriction of xs:NMTOKEN

/// Note: enum values are prefixed with 'uddi2__keyType' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum uddi2__keyType
{	uddi2__keyType__businessKey,	///< xs:NMTOKEN value="businessKey"
	uddi2__keyType__tModelKey,	///< xs:NMTOKEN value="tModelKey"
	uddi2__keyType__serviceKey,	///< xs:NMTOKEN value="serviceKey"
	uddi2__keyType__bindingKey,	///< xs:NMTOKEN value="bindingKey"
};

/// Schema urn:uddi-org:api_v2 complexType "address"

class uddi2__address
{ public:
/// Vector of uddi2__addressLine* with length 0..unbounded
    std::vector<uddi2__addressLine*    > addressLine                    0;
/// Attribute useType of type xs:string
   @char*                                useType                        0;	///< Optional attribute
/// Attribute sortCode of type xs:string
   @char*                                sortCode                       0;	///< Optional attribute
/// Attribute tModelKey of type "urn:uddi-org:api_v2":tModelKey
   @uddi2__tModelKey                     tModelKey                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "assertionStatusItem"

class uddi2__assertionStatusItem
{ public:
/// Element reference "urn:uddi-org:api_v2":fromKey
    uddi2__businessKey                   fromKey                        1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":toKey
    uddi2__businessKey                   toKey                          1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":keyedReference
    uddi2__keyedReference*               keyedReference                 1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":keysOwned
    uddi2__keysOwned*                    keysOwned                      1;	///< Required element
/// Attribute completionStatus of type xs:string
   @char*                                completionStatus               1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "bindingTemplate"

class uddi2__bindingTemplate
{ public:
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Element reference "urn:uddi-org:api_v2":tModelInstanceDetails
    uddi2__tModelInstanceDetails*        tModelInstanceDetails          1;	///< Required element
// Choice of elements:
/// Element reference "urn:uddi-org:api_v2":accessPoint
    uddi2__accessPoint*                  accessPoint                    1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":hostingRedirector
    uddi2__hostingRedirector*            hostingRedirector              1;	///< Required element
// end choice
/// Attribute serviceKey of type "urn:uddi-org:api_v2":serviceKey
   @uddi2__serviceKey                    serviceKey                     0;	///< Optional attribute
/// Attribute bindingKey of type "urn:uddi-org:api_v2":bindingKey
   @uddi2__bindingKey                    bindingKey                     1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "bindingTemplates"

class uddi2__bindingTemplates
{ public:
/// Vector of uddi2__bindingTemplate* with length 0..unbounded
    std::vector<uddi2__bindingTemplate*> bindingTemplate                0;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "businessEntity"

class uddi2__businessEntity
{ public:
/// Element reference "urn:uddi-org:api_v2":discoveryURLs
    uddi2__discoveryURLs*                discoveryURLs                  0;	///< Optional element
/// Vector of uddi2__name* with length 0..unbounded
    std::vector<uddi2__name*           > name                           1;
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Element reference "urn:uddi-org:api_v2":contacts
    uddi2__contacts*                     contacts                       0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":businessServices
    uddi2__businessServices*             businessServices               0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":identifierBag
    uddi2__identifierBag*                identifierBag                  0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":categoryBag
    uddi2__categoryBag*                  categoryBag                    0;	///< Optional element
/// Attribute businessKey of type "urn:uddi-org:api_v2":businessKey
   @uddi2__businessKey                   businessKey                    1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      0;	///< Optional attribute
/// Attribute authorizedName of type xs:string
   @char*                                authorizedName                 0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "businessEntityExt"

class uddi2__businessEntityExt
{ public:
/// Element reference "urn:uddi-org:api_v2":businessEntity
    uddi2__businessEntity*               businessEntity                 1;	///< Required element
/// A gSOAP type indicator SOAP_TYPE_X (e.g. SOAP_TYPE_int, SOAP_TYPE_std__string, ..., as defined by soapH.h)
    int                                  __type                        ;
/// Pointer to an instance of the type indicated with __type
    void*                                _any                          ;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "businessInfo"

class uddi2__businessInfo
{ public:
/// Vector of uddi2__name* with length 0..unbounded
    std::vector<uddi2__name*           > name                           1;
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Element reference "urn:uddi-org:api_v2":serviceInfos
    uddi2__serviceInfos*                 serviceInfos                   1;	///< Required element
/// Attribute businessKey of type "urn:uddi-org:api_v2":businessKey
   @uddi2__businessKey                   businessKey                    1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "businessInfos"

class uddi2__businessInfos
{ public:
/// Vector of uddi2__businessInfo* with length 0..unbounded
    std::vector<uddi2__businessInfo*   > businessInfo                   0;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "businessService"

class uddi2__businessService
{ public:
/// Vector of uddi2__name* with length 0..unbounded
    std::vector<uddi2__name*           > name                           0;
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Element reference "urn:uddi-org:api_v2":bindingTemplates
    uddi2__bindingTemplates*             bindingTemplates               0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":categoryBag
    uddi2__categoryBag*                  categoryBag                    0;	///< Optional element
/// Attribute serviceKey of type "urn:uddi-org:api_v2":serviceKey
   @uddi2__serviceKey                    serviceKey                     1;	///< Required attribute
/// Attribute businessKey of type "urn:uddi-org:api_v2":businessKey
   @uddi2__businessKey                   businessKey                    0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "businessServices"

class uddi2__businessServices
{ public:
/// Vector of uddi2__businessService* with length 0..unbounded
    std::vector<uddi2__businessService*> businessService                0;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "categoryBag"

class uddi2__categoryBag
{ public:
/// Vector of uddi2__keyedReference* with length 0..unbounded
    std::vector<uddi2__keyedReference* > keyedReference                 1;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "contact"

class uddi2__contact
{ public:
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Element reference "urn:uddi-org:api_v2":personName
    char*                                personName                     1;	///< Required element
/// Vector of uddi2__phone* with length 0..unbounded
    std::vector<uddi2__phone*          > phone                          0;
/// Vector of uddi2__email* with length 0..unbounded
    std::vector<uddi2__email*          > email                          0;
/// Vector of uddi2__address* with length 0..unbounded
    std::vector<uddi2__address*        > address                        0;
/// Attribute useType of type xs:string
   @char*                                useType                        0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "contacts"

class uddi2__contacts
{ public:
/// Vector of uddi2__contact* with length 0..unbounded
    std::vector<uddi2__contact*        > contact                        1;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "discoveryURLs"

class uddi2__discoveryURLs
{ public:
/// Vector of uddi2__discoveryURL* with length 0..unbounded
    std::vector<uddi2__discoveryURL*   > discoveryURL                   1;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "dispositionReport"

class uddi2__dispositionReport
{ public:
/// Vector of uddi2__result* with length 0..unbounded
    std::vector<uddi2__result*         > result                         1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "findQualifiers"

class uddi2__findQualifiers
{ public:
/// Vector of char* with length 0..unbounded
    std::vector<char*                  > findQualifier                  0;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "hostingRedirector"

class uddi2__hostingRedirector
{ public:
/// Attribute bindingKey of type "urn:uddi-org:api_v2":bindingKey
   @uddi2__bindingKey                    bindingKey                     1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "identifierBag"

class uddi2__identifierBag
{ public:
/// Vector of uddi2__keyedReference* with length 0..unbounded
    std::vector<uddi2__keyedReference* > keyedReference                 1;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "instanceDetails"

class uddi2__instanceDetails
{ public:
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Element reference "urn:uddi-org:api_v2":overviewDoc
    uddi2__overviewDoc*                  overviewDoc                    0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":instanceParms
    char*                                instanceParms                  0;	///< Optional element
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "keyedReference"

class uddi2__keyedReference
{ public:
/// Attribute tModelKey of type "urn:uddi-org:api_v2":tModelKey
   @uddi2__tModelKey                     tModelKey                      0;	///< Optional attribute
/// Attribute keyName of type xs:string
   @char*                                keyName                        0;	///< Optional attribute
/// Attribute keyValue of type xs:string
   @char*                                keyValue                       1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "keysOwned"

class uddi2__keysOwned
{ public:
/// Element reference "urn:uddi-org:api_v2":fromKey
    uddi2__businessKey                   fromKey                        0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":toKey
    uddi2__businessKey                   toKey                          0;	///< Optional element
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "overviewDoc"

class uddi2__overviewDoc
{ public:
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Element reference "urn:uddi-org:api_v2":overviewURL
    char*                                overviewURL                    0;	///< Optional element
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "publisherAssertion"

class uddi2__publisherAssertion
{ public:
/// Element reference "urn:uddi-org:api_v2":fromKey
    uddi2__businessKey                   fromKey                        1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":toKey
    uddi2__businessKey                   toKey                          1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":keyedReference
    uddi2__keyedReference*               keyedReference                 1;	///< Required element
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "relatedBusinessInfo"

class uddi2__relatedBusinessInfo
{ public:
/// Element reference "urn:uddi-org:api_v2":businessKey
    uddi2__businessKey                   businessKey                    1;	///< Required element
/// Vector of uddi2__name* with length 0..unbounded
    std::vector<uddi2__name*           > name                           1;
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Vector of uddi2__sharedRelationships* with length 0..2
    std::vector<uddi2__sharedRelationships*> sharedRelationships            1:2;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "relatedBusinessInfos"

class uddi2__relatedBusinessInfos
{ public:
/// Vector of uddi2__relatedBusinessInfo* with length 0..unbounded
    std::vector<uddi2__relatedBusinessInfo*> relatedBusinessInfo            0;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "result"

class uddi2__result
{ public:
/// Element reference "urn:uddi-org:api_v2":errInfo
    uddi2__errInfo*                      errInfo                        0;	///< Optional element
/// Attribute keyType of type "urn:uddi-org:api_v2":keyType
   @enum uddi2__keyType*                 keyType                        0;	///< Optional attribute
/// Attribute errno of type xs:int
   @int                                  errno_                         1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "serviceInfo"

class uddi2__serviceInfo
{ public:
/// Vector of uddi2__name* with length 0..unbounded
    std::vector<uddi2__name*           > name                           0;
/// Attribute serviceKey of type "urn:uddi-org:api_v2":serviceKey
   @uddi2__serviceKey                    serviceKey                     1;	///< Required attribute
/// Attribute businessKey of type "urn:uddi-org:api_v2":businessKey
   @uddi2__businessKey                   businessKey                    1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "serviceInfos"

class uddi2__serviceInfos
{ public:
/// Vector of uddi2__serviceInfo* with length 0..unbounded
    std::vector<uddi2__serviceInfo*    > serviceInfo                    0;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "sharedRelationships"

class uddi2__sharedRelationships
{ public:
/// Vector of uddi2__keyedReference* with length 0..unbounded
    std::vector<uddi2__keyedReference* > keyedReference                 1;
/// Attribute direction of type "urn:uddi-org:api_v2":direction
   @enum uddi2__direction                direction                      1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "tModel"

class uddi2__tModel
{ public:
/// Element reference "urn:uddi-org:api_v2":name
    uddi2__name*                         name                           1;	///< Required element
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Element reference "urn:uddi-org:api_v2":overviewDoc
    uddi2__overviewDoc*                  overviewDoc                    0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":identifierBag
    uddi2__identifierBag*                identifierBag                  0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":categoryBag
    uddi2__categoryBag*                  categoryBag                    0;	///< Optional element
/// Attribute tModelKey of type "urn:uddi-org:api_v2":tModelKey
   @uddi2__tModelKey                     tModelKey                      1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      0;	///< Optional attribute
/// Attribute authorizedName of type xs:string
   @char*                                authorizedName                 0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "tModelBag"

class uddi2__tModelBag
{ public:
/// Vector of uddi2__tModelKey with length 0..unbounded
    std::vector<uddi2__tModelKey       > tModelKey                      1;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "tModelInfo"

class uddi2__tModelInfo
{ public:
/// Element reference "urn:uddi-org:api_v2":name
    uddi2__name*                         name                           1;	///< Required element
/// Attribute tModelKey of type "urn:uddi-org:api_v2":tModelKey
   @uddi2__tModelKey                     tModelKey                      1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "tModelInfos"

class uddi2__tModelInfos
{ public:
/// Vector of uddi2__tModelInfo* with length 0..unbounded
    std::vector<uddi2__tModelInfo*     > tModelInfo                     0;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "tModelInstanceDetails"

class uddi2__tModelInstanceDetails
{ public:
/// Vector of uddi2__tModelInstanceInfo* with length 0..unbounded
    std::vector<uddi2__tModelInstanceInfo*> tModelInstanceInfo             0;
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "tModelInstanceInfo"

class uddi2__tModelInstanceInfo
{ public:
/// Vector of uddi2__description* with length 0..unbounded
    std::vector<uddi2__description*    > description                    0;
/// Element reference "urn:uddi-org:api_v2":instanceDetails
    uddi2__instanceDetails*              instanceDetails                0;	///< Optional element
/// Attribute tModelKey of type "urn:uddi-org:api_v2":tModelKey
   @uddi2__tModelKey                     tModelKey                      1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "add_publisherAssertions"

class uddi2__add_USCOREpublisherAssertions
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__publisherAssertion* with length 0..unbounded
    std::vector<uddi2__publisherAssertion*> publisherAssertion             1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "delete_binding"

class uddi2__delete_USCOREbinding
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__bindingKey with length 0..unbounded
    std::vector<uddi2__bindingKey      > bindingKey                     1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "delete_business"

class uddi2__delete_USCOREbusiness
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__businessKey with length 0..unbounded
    std::vector<uddi2__businessKey     > businessKey                    1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "delete_publisherAssertions"

class uddi2__delete_USCOREpublisherAssertions
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__publisherAssertion* with length 0..unbounded
    std::vector<uddi2__publisherAssertion*> publisherAssertion             1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "delete_service"

class uddi2__delete_USCOREservice
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__serviceKey with length 0..unbounded
    std::vector<uddi2__serviceKey      > serviceKey                     1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "delete_tModel"

class uddi2__delete_USCOREtModel
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__tModelKey with length 0..unbounded
    std::vector<uddi2__tModelKey       > tModelKey                      1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "discard_authToken"

class uddi2__discard_USCOREauthToken
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "find_binding"

class uddi2__find_USCOREbinding
{ public:
/// Element reference "urn:uddi-org:api_v2":findQualifiers
    uddi2__findQualifiers*               findQualifiers                 0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":tModelBag
    uddi2__tModelBag*                    tModelBag                      1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute maxRows of type xs:int
   @int*                                 maxRows                        0;	///< Optional attribute
/// Attribute serviceKey of type "urn:uddi-org:api_v2":serviceKey
   @uddi2__serviceKey                    serviceKey                     1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__find_USCOREbinding(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREbinding(struct soap*, const char *tModelKey);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREbinding(struct soap*, std::vector<char*> tModelKeys);
/// Member declared in uddi2map.dat
    uddi2__bindingDetail *send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "find_business"

class uddi2__find_USCOREbusiness
{ public:
/// Element reference "urn:uddi-org:api_v2":findQualifiers
    uddi2__findQualifiers*               findQualifiers                 0;	///< Optional element
/// Vector of uddi2__name* with length 0..unbounded
    std::vector<uddi2__name*           > name                           0;
/// Element reference "urn:uddi-org:api_v2":identifierBag
    uddi2__identifierBag*                identifierBag                  0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":categoryBag
    uddi2__categoryBag*                  categoryBag                    0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":tModelBag
    uddi2__tModelBag*                    tModelBag                      0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":discoveryURLs
    uddi2__discoveryURLs*                discoveryURLs                  0;	///< Optional element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute maxRows of type xs:int
   @int*                                 maxRows                        0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__find_USCOREbusiness(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREbusiness(struct soap*, const char *name);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREbusiness(struct soap*, std::vector<char*> tModelKeys);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREbusiness(struct soap*, std::vector<uddi2__keyedReference*> keyedReferences);
/// Member declared in uddi2map.dat
    uddi2__businessList *send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "find_relatedBusinesses"

class uddi2__find_USCORErelatedBusinesses
{ public:
/// Element reference "urn:uddi-org:api_v2":findQualifiers
    uddi2__findQualifiers*               findQualifiers                 0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":businessKey
    uddi2__businessKey                   businessKey                    1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":keyedReference
    uddi2__keyedReference*               keyedReference                 0;	///< Optional element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute maxRows of type xs:int
   @int*                                 maxRows                        0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__find_USCORErelatedBusinesses(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__find_USCORErelatedBusinesses(struct soap*, const char *businessKey);
/// Member declared in uddi2map.dat
    uddi2__relatedBusinessesList *send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "find_service"

class uddi2__find_USCOREservice
{ public:
/// Element reference "urn:uddi-org:api_v2":findQualifiers
    uddi2__findQualifiers*               findQualifiers                 0;	///< Optional element
/// Vector of uddi2__name* with length 0..unbounded
    std::vector<uddi2__name*           > name                           0;
/// Element reference "urn:uddi-org:api_v2":categoryBag
    uddi2__categoryBag*                  categoryBag                    0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":tModelBag
    uddi2__tModelBag*                    tModelBag                      0;	///< Optional element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute maxRows of type xs:int
   @int*                                 maxRows                        0;	///< Optional attribute
/// Attribute businessKey of type "urn:uddi-org:api_v2":businessKey
   @uddi2__businessKey                   businessKey                    0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__find_USCOREservice(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREservice(struct soap*, const char *name);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREservice(struct soap*, std::vector<char*> tModelKeys);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREservice(struct soap*, std::vector<uddi2__keyedReference*> keyedReferences);
/// Member declared in uddi2map.dat
    uddi2__serviceList* send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "find_tModel"

class uddi2__find_USCOREtModel
{ public:
/// Element reference "urn:uddi-org:api_v2":findQualifiers
    uddi2__findQualifiers*               findQualifiers                 0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":name
    uddi2__name*                         name                           0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":identifierBag
    uddi2__identifierBag*                identifierBag                  0;	///< Optional element
/// Element reference "urn:uddi-org:api_v2":categoryBag
    uddi2__categoryBag*                  categoryBag                    0;	///< Optional element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute maxRows of type xs:int
   @int*                                 maxRows                        0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__find_USCOREtModel(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREtModel(struct soap*, const char *name);
/// Member declared in uddi2map.dat
    uddi2__find_USCOREtModel(struct soap*, std::vector<uddi2__keyedReference*> keyedReferences);
/// Member declared in uddi2map.dat
    uddi2__tModelList* send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "get_assertionStatusReport"

class uddi2__get_USCOREassertionStatusReport
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":completionStatus
    char*                                completionStatus               0;	///< Optional element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "get_authToken"

class uddi2__get_USCOREauthToken
{ public:
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute userID of type xs:string
   @char*                                userID                         1;	///< Required attribute
/// Attribute cred of type xs:string
   @char*                                cred                           1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "get_bindingDetail"

class uddi2__get_USCOREbindingDetail
{ public:
/// Vector of uddi2__bindingKey with length 0..unbounded
    std::vector<uddi2__bindingKey      > bindingKey                     1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__get_USCOREbindingDetail(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREbindingDetail(struct soap*, const char *bindingKey);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREbindingDetail(struct soap*, std::vector<char*> bindingKeys);
/// Member declared in uddi2map.dat
    uddi2__bindingDetail* send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "get_businessDetail"

class uddi2__get_USCOREbusinessDetail
{ public:
/// Vector of uddi2__businessKey with length 0..unbounded
    std::vector<uddi2__businessKey     > businessKey                    1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__get_USCOREbusinessDetail(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREbusinessDetail(struct soap*, const char *businessKey);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREbusinessDetail(struct soap*, std::vector<char*> businessKeys);
/// Member declared in uddi2map.dat
    uddi2__businessDetail* send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "get_businessDetailExt"

class uddi2__get_USCOREbusinessDetailExt
{ public:
/// Vector of uddi2__businessKey with length 0..unbounded
    std::vector<uddi2__businessKey     > businessKey                    1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__get_USCOREbusinessDetailExt(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREbusinessDetailExt(struct soap*, const char *businessKey);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREbusinessDetailExt(struct soap*, std::vector<char*> businessKeys);
/// Member declared in uddi2map.dat
    uddi2__businessDetailExt* send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "get_publisherAssertions"

class uddi2__get_USCOREpublisherAssertions
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "get_registeredInfo"

class uddi2__get_USCOREregisteredInfo
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "get_serviceDetail"

class uddi2__get_USCOREserviceDetail
{ public:
/// Vector of uddi2__serviceKey with length 0..unbounded
    std::vector<uddi2__serviceKey      > serviceKey                     1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__get_USCOREserviceDetail(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREserviceDetail(struct soap*, const char *serviceKey);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREserviceDetail(struct soap*, std::vector<char*> serviceKeys);
/// Member declared in uddi2map.dat
    uddi2__serviceDetail* send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "get_tModelDetail"

class uddi2__get_USCOREtModelDetail
{ public:
/// Vector of uddi2__tModelKey with length 0..unbounded
    std::vector<uddi2__tModelKey       > tModelKey                      1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
/// Member declared in uddi2map.dat
    uddi2__get_USCOREtModelDetail(struct soap*);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREtModelDetail(struct soap*, const char *tModelKey);
/// Member declared in uddi2map.dat
    uddi2__get_USCOREtModelDetail(struct soap*, std::vector<char*> tModelKeys);
/// Member declared in uddi2map.dat
    uddi2__tModelDetail* send(const char *endpoint);
};

/// Schema urn:uddi-org:api_v2 complexType "save_binding"

class uddi2__save_USCOREbinding
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__bindingTemplate* with length 0..unbounded
    std::vector<uddi2__bindingTemplate*> bindingTemplate                1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "save_business"

class uddi2__save_USCOREbusiness
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__businessEntity* with length 0..unbounded
    std::vector<uddi2__businessEntity* > businessEntity                 0;
/// Vector of char* with length 0..unbounded
    std::vector<char*                  > uploadRegister                 0;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "save_service"

class uddi2__save_USCOREservice
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__businessService* with length 0..unbounded
    std::vector<uddi2__businessService*> businessService                1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "save_tModel"

class uddi2__save_USCOREtModel
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__tModel* with length 0..unbounded
    std::vector<uddi2__tModel*         > tModel                         0;
/// Vector of char* with length 0..unbounded
    std::vector<char*                  > uploadRegister                 0;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "set_publisherAssertions"

class uddi2__set_USCOREpublisherAssertions
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Vector of uddi2__publisherAssertion* with length 0..unbounded
    std::vector<uddi2__publisherAssertion*> publisherAssertion             0;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "validate_values"

class uddi2__validate_USCOREvalues
{ public:
// Choice of elements:
/// Vector of uddi2__businessEntity* with length 0..unbounded
    std::vector<uddi2__businessEntity* > businessEntity                 0;
/// Vector of uddi2__businessService* with length 0..unbounded
    std::vector<uddi2__businessService*> businessService                0;
/// Vector of uddi2__tModel* with length 0..unbounded
    std::vector<uddi2__tModel*         > tModel                         0;
// end choice
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "assertionStatusReport"

class uddi2__assertionStatusReport
{ public:
/// Vector of uddi2__assertionStatusItem* with length 0..unbounded
    std::vector<uddi2__assertionStatusItem*> assertionStatusItem            0;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "authToken"

class uddi2__authToken
{ public:
/// Element reference "urn:uddi-org:api_v2":authInfo
    char*                                authInfo                       1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "bindingDetail"

class uddi2__bindingDetail
{ public:
/// Vector of uddi2__bindingTemplate* with length 0..unbounded
    std::vector<uddi2__bindingTemplate*> bindingTemplate                0;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "businessDetail"

class uddi2__businessDetail
{ public:
/// Vector of uddi2__businessEntity* with length 0..unbounded
    std::vector<uddi2__businessEntity* > businessEntity                 0;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "businessDetailExt"

class uddi2__businessDetailExt
{ public:
/// Vector of uddi2__businessEntityExt* with length 0..unbounded
    std::vector<uddi2__businessEntityExt*> businessEntityExt              1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "businessList"

class uddi2__businessList
{ public:
/// Element reference "urn:uddi-org:api_v2":businessInfos
    uddi2__businessInfos*                businessInfos                  1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "publisherAssertions"

class uddi2__publisherAssertions
{ public:
/// Vector of uddi2__publisherAssertion* with length 0..unbounded
    std::vector<uddi2__publisherAssertion*> publisherAssertion             0;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute authorizedName of type xs:string
   @char*                                authorizedName                 1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "registeredInfo"

class uddi2__registeredInfo
{ public:
/// Element reference "urn:uddi-org:api_v2":businessInfos
    uddi2__businessInfos*                businessInfos                  1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":tModelInfos
    uddi2__tModelInfos*                  tModelInfos                    1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "relatedBusinessesList"

class uddi2__relatedBusinessesList
{ public:
/// Element reference "urn:uddi-org:api_v2":businessKey
    uddi2__businessKey                   businessKey                    1;	///< Required element
/// Element reference "urn:uddi-org:api_v2":relatedBusinessInfos
    uddi2__relatedBusinessInfos*         relatedBusinessInfos           1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "serviceDetail"

class uddi2__serviceDetail
{ public:
/// Vector of uddi2__businessService* with length 0..unbounded
    std::vector<uddi2__businessService*> businessService                0;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "serviceList"

class uddi2__serviceList
{ public:
/// Element reference "urn:uddi-org:api_v2":serviceInfos
    uddi2__serviceInfos*                 serviceInfos                   1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "tModelDetail"

class uddi2__tModelDetail
{ public:
/// Vector of uddi2__tModel* with length 0..unbounded
    std::vector<uddi2__tModel*         > tModel                         1;
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "tModelList"

class uddi2__tModelList
{ public:
/// Element reference "urn:uddi-org:api_v2":tModelInfos
    uddi2__tModelInfos*                  tModelInfos                    1;	///< Required element
/// Attribute generic of type xs:string
   @char*                                generic                        1;	///< Required attribute
/// Attribute operator of type xs:string
   @char*                                operator_                      1;	///< Required attribute
/// Attribute truncated of type "urn:uddi-org:api_v2":truncated
   @enum uddi2__truncated*               truncated                      0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "accessPoint"

class uddi2__accessPoint
{ public:
    char*                                __item;                       ;
/// Attribute URLType of type "urn:uddi-org:api_v2":URLType
   @enum uddi2__URLType                  URLType                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "addressLine"

class uddi2__addressLine
{ public:
    char*                                __item;                       ;
/// Attribute keyName of type xs:string
   @char*                                keyName                        0;	///< Optional attribute
/// Attribute keyValue of type xs:string
   @char*                                keyValue                       0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "description"

class uddi2__description
{ public:
    char*                                __item;                       ;
/// Attribute reference xml:lang
   @xml__lang                            xml__lang_                     0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "discoveryURL"

class uddi2__discoveryURL
{ public:
    char*                                __item;                       ;
/// Attribute useType of type xs:string
   @char*                                useType                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "email"

class uddi2__email
{ public:
    char*                                __item;                       ;
/// Attribute useType of type xs:string
   @char*                                useType                        0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "errInfo"

class uddi2__errInfo
{ public:
    char*                                __item;                       ;
/// Attribute errCode of type xs:string
   @char*                                errCode                        1;	///< Required attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "name"

class uddi2__name
{ public:
    char*                                __item;                       ;
/// Attribute reference xml:lang
   @xml__lang                            xml__lang_                     0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/// Schema urn:uddi-org:api_v2 complexType "phone"

class uddi2__phone
{ public:
    char*                                __item;                       ;
/// Attribute useType of type xs:string
   @char*                                useType                        0;	///< Optional attribute
/// A handle to the soap struct context that manages this class instance
    struct soap                         *soap                          ;
};

/******************************************************************************\
 *                                                                            *
 * Services                                                                   *
 *                                                                            *
\******************************************************************************/

//gsoap inq2 service name:	InquireSoap 
//gsoap inq2 service type:	Inquire 
//gsoap inq2 service namespace:	urn:uddi-org:inquiry_v2 

/** @mainpage UDDI_Inquiry_API_V2 Definitions

@section UDDI_Inquiry_API_V2_documentation Documentation

		Copyright (c) 2000 - 2002 by Accenture, Ariba, Inc., Commerce One, Inc. 
		Fujitsu Limited, Hewlett-Packard Company, i2 Technologies, Inc., 
		Intel Corporation, International Business Machines Corporation,  
		Microsoft Corporation, Oracle Corporation, SAP AG, Sun Microsystems, Inc., 
		and VeriSign, Inc.  All Rights Reserved.
		
		WSDL Service Interface for UDDI Inquiry API V2.0    
		
		This WSDL document defines the inquiry API calls for interacting with 
		the UDDI registry.  The complete UDDI API specification is available
		at http://www.uddi.org/specification.html.
	

@section UDDI_Inquiry_API_V2_bindings Bindings
  - @ref InquireSoap

*/

/**

@page InquireSoap Binding "InquireSoap"

@section InquireSoap_operations Operations of Binding  "InquireSoap"
  - @ref __inq2__find_USCOREbinding
  - @ref __inq2__find_USCOREbusiness
  - @ref __inq2__find_USCORErelatedBusinesses
  - @ref __inq2__find_USCOREservice
  - @ref __inq2__find_USCOREtModel
  - @ref __inq2__get_USCOREbindingDetail
  - @ref __inq2__get_USCOREbusinessDetail
  - @ref __inq2__get_USCOREbusinessDetailExt
  - @ref __inq2__get_USCOREserviceDetail
  - @ref __inq2__get_USCOREtModelDetail

@section InquireSoap_ports Endpoints of Binding  "InquireSoap"

*/

/******************************************************************************\
 *                                                                            *
 * SOAP Fault Detail                                                          *
 *                                                                            *
\******************************************************************************/

/// The SOAP Fault Detail element contains one of the following types serialized
// in the __type and fault fields of the SOAP_ENV__Detail struct (see docs)
class _dispositionReport
{ public:
    uddi2__dispositionReport*           uddi2__dispositionReport_;
    struct soap                         *soap                          ;
};

/******************************************************************************\
 *                                                                            *
 * InquireSoap                                                                *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * __inq2__find_USCOREbinding                                                 *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__find_USCOREbinding" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="find_binding"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__find_USCOREbinding(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__find_USCOREbinding*          uddi2__find_USCOREbinding_,
    // response parameters:
    uddi2__bindingDetail*               uddi2__bindingDetail_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	find_USCOREbinding document
//gsoap inq2 service method-encoding:	find_USCOREbinding literal
//gsoap inq2 service method-action:	find_USCOREbinding find_binding
//gsoap inq2 service method-fault:	find_USCOREbinding _dispositionReport
int __inq2__find_USCOREbinding(
    uddi2__find_USCOREbinding*          uddi2__find_USCOREbinding_,
    uddi2__bindingDetail*               uddi2__bindingDetail_ ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * __inq2__find_USCOREbusiness                                                *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__find_USCOREbusiness" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="find_business"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__find_USCOREbusiness(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__find_USCOREbusiness*         uddi2__find_USCOREbusiness_,
    // response parameters:
    uddi2__businessList*                uddi2__businessList_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	find_USCOREbusiness document
//gsoap inq2 service method-encoding:	find_USCOREbusiness literal
//gsoap inq2 service method-action:	find_USCOREbusiness find_business
//gsoap inq2 service method-fault:	find_USCOREbusiness _dispositionReport
int __inq2__find_USCOREbusiness(
    uddi2__find_USCOREbusiness*         uddi2__find_USCOREbusiness_,
    uddi2__businessList*                uddi2__businessList_ ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * __inq2__find_USCORErelatedBusinesses                                       *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__find_USCORErelatedBusinesses" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="find_relatedBusinesses"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__find_USCORErelatedBusinesses(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__find_USCORErelatedBusinesses* uddi2__find_USCORErelatedBusinesses_,
    // response parameters:
    uddi2__relatedBusinessesList*       uddi2__relatedBusinessesList_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	find_USCORErelatedBusinesses document
//gsoap inq2 service method-encoding:	find_USCORErelatedBusinesses literal
//gsoap inq2 service method-action:	find_USCORErelatedBusinesses find_relatedBusinesses
//gsoap inq2 service method-fault:	find_USCORErelatedBusinesses _dispositionReport
int __inq2__find_USCORErelatedBusinesses(
    uddi2__find_USCORErelatedBusinesses* uddi2__find_USCORErelatedBusinesses_,
    uddi2__relatedBusinessesList*       uddi2__relatedBusinessesList_ ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * __inq2__find_USCOREservice                                                 *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__find_USCOREservice" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="find_service"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__find_USCOREservice(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__find_USCOREservice*          uddi2__find_USCOREservice_,
    // response parameters:
    uddi2__serviceList*                 uddi2__serviceList_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	find_USCOREservice document
//gsoap inq2 service method-encoding:	find_USCOREservice literal
//gsoap inq2 service method-action:	find_USCOREservice find_service
//gsoap inq2 service method-fault:	find_USCOREservice _dispositionReport
int __inq2__find_USCOREservice(
    uddi2__find_USCOREservice*          uddi2__find_USCOREservice_,
    uddi2__serviceList*                 uddi2__serviceList_ ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * __inq2__find_USCOREtModel                                                  *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__find_USCOREtModel" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="find_tModel"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__find_USCOREtModel(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__find_USCOREtModel*           uddi2__find_USCOREtModel_,
    // response parameters:
    uddi2__tModelList*                  uddi2__tModelList_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	find_USCOREtModel document
//gsoap inq2 service method-encoding:	find_USCOREtModel literal
//gsoap inq2 service method-action:	find_USCOREtModel find_tModel
//gsoap inq2 service method-fault:	find_USCOREtModel _dispositionReport
int __inq2__find_USCOREtModel(
    uddi2__find_USCOREtModel*           uddi2__find_USCOREtModel_,
    uddi2__tModelList*                  uddi2__tModelList_ ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * __inq2__get_USCOREbindingDetail                                            *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__get_USCOREbindingDetail" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="get_bindingDetail"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__get_USCOREbindingDetail(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__get_USCOREbindingDetail*     uddi2__get_USCOREbindingDetail_,
    // response parameters:
    uddi2__bindingDetail*               uddi2__bindingDetail_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	get_USCOREbindingDetail document
//gsoap inq2 service method-encoding:	get_USCOREbindingDetail literal
//gsoap inq2 service method-action:	get_USCOREbindingDetail get_bindingDetail
//gsoap inq2 service method-fault:	get_USCOREbindingDetail _dispositionReport
int __inq2__get_USCOREbindingDetail(
    uddi2__get_USCOREbindingDetail*     uddi2__get_USCOREbindingDetail_,
    uddi2__bindingDetail*               uddi2__bindingDetail_ ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * __inq2__get_USCOREbusinessDetail                                           *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__get_USCOREbusinessDetail" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="get_businessDetail"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__get_USCOREbusinessDetail(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__get_USCOREbusinessDetail*    uddi2__get_USCOREbusinessDetail_,
    // response parameters:
    uddi2__businessDetail*              uddi2__businessDetail_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	get_USCOREbusinessDetail document
//gsoap inq2 service method-encoding:	get_USCOREbusinessDetail literal
//gsoap inq2 service method-action:	get_USCOREbusinessDetail get_businessDetail
//gsoap inq2 service method-fault:	get_USCOREbusinessDetail _dispositionReport
int __inq2__get_USCOREbusinessDetail(
    uddi2__get_USCOREbusinessDetail*    uddi2__get_USCOREbusinessDetail_,
    uddi2__businessDetail*              uddi2__businessDetail_ ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * __inq2__get_USCOREbusinessDetailExt                                        *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__get_USCOREbusinessDetailExt" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="get_businessDetailExt"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__get_USCOREbusinessDetailExt(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__get_USCOREbusinessDetailExt* uddi2__get_USCOREbusinessDetailExt_,
    // response parameters:
    uddi2__businessDetailExt*           uddi2__businessDetailExt_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	get_USCOREbusinessDetailExt document
//gsoap inq2 service method-encoding:	get_USCOREbusinessDetailExt literal
//gsoap inq2 service method-action:	get_USCOREbusinessDetailExt get_businessDetailExt
//gsoap inq2 service method-fault:	get_USCOREbusinessDetailExt _dispositionReport
int __inq2__get_USCOREbusinessDetailExt(
    uddi2__get_USCOREbusinessDetailExt* uddi2__get_USCOREbusinessDetailExt_,
    uddi2__businessDetailExt*           uddi2__businessDetailExt_ ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * __inq2__get_USCOREserviceDetail                                            *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__get_USCOREserviceDetail" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="get_serviceDetail"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__get_USCOREserviceDetail(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__get_USCOREserviceDetail*     uddi2__get_USCOREserviceDetail_,
    // response parameters:
    uddi2__serviceDetail*               uddi2__serviceDetail_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	get_USCOREserviceDetail document
//gsoap inq2 service method-encoding:	get_USCOREserviceDetail literal
//gsoap inq2 service method-action:	get_USCOREserviceDetail get_serviceDetail
//gsoap inq2 service method-fault:	get_USCOREserviceDetail _dispositionReport
int __inq2__get_USCOREserviceDetail(
    uddi2__get_USCOREserviceDetail*     uddi2__get_USCOREserviceDetail_,
    uddi2__serviceDetail*               uddi2__serviceDetail_ ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * __inq2__get_USCOREtModelDetail                                             *
 *                                                                            *
\******************************************************************************/


/// Operation "__inq2__get_USCOREtModelDetail" of service binding "InquireSoap"

/**

Operation details:

  - SOAP document/literal style
  - SOAP action="get_tModelDetail"
  - SOAP Fault: _dispositionReport

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call___inq2__get_USCOREtModelDetail(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    uddi2__get_USCOREtModelDetail*      uddi2__get_USCOREtModelDetail_,
    // response parameters:
    uddi2__tModelDetail*                uddi2__tModelDetail_
  );
@endcode

C++ proxy class (defined in soapInquireSoapProxy.h):
  class InquireSoap;

*/

//gsoap inq2 service method-style:	get_USCOREtModelDetail document
//gsoap inq2 service method-encoding:	get_USCOREtModelDetail literal
//gsoap inq2 service method-action:	get_USCOREtModelDetail get_tModelDetail
//gsoap inq2 service method-fault:	get_USCOREtModelDetail _dispositionReport
int __inq2__get_USCOREtModelDetail(
    uddi2__get_USCOREtModelDetail*      uddi2__get_USCOREtModelDetail_,
    uddi2__tModelDetail*                uddi2__tModelDetail_ ///< response parameter
);

/* End of inquire_v2.h */
