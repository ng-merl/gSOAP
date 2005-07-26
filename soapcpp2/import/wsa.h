/*

wsa.h

Generated with:
wsdl2h -cgy -o wsa.h -t WS/WS-typemap.dat WS/WS-Addressing.xsd

Modified by Robert van Engelen:

- Removed //gsoapopt
- Added the following directive to import WS-Addressing namespace:
  //gsoap wsa schema import: http://schemas.xmlsoap.org/ws/2004/08/addressing
  This ensures that the WS-Addressing schemas are not copied into the generated
  WSDL by soapcpp2 but are referenced with schema import in the generated WSDL.

Usage:

// header file for soapcpp2
#import "wsa.h"
struct SOAP_ENV__Header
{
  mustUnderstand _wsa__MessageID  wsa__MessageID 0;
  mustUnderstand _wsa__RelatesTo *wsa__RelatesTo 0;
  mustUnderstand _wsa__From      *wsa__From      0;
  mustUnderstand _wsa__ReplyTo   *wsa__ReplyTo   0;
  mustUnderstand _wsa__FaultTo   *wsa__FaultTo   0;
  mustUnderstand _wsa__To         wsa__To        0;
  mustUnderstand _wsa__Action     wsa__Action    0;
};

// client-side source code
{ struct soap soap;
  struct SOAP_ENV__Header header;
  _wsa__ReplyTo replyTo;
  soap_init(&soap);
  soap_default_SOAP_ENV__Header(&soap, &header);
  soap.header = &header;
  soap_default_wsa__EndpointReferenceType(&soap, &replyTo);
  replyTo.Address = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anony
mous";
  header.wsa__MessageID = "...";
  header.wsa__To = "...";
  header.wsa__Action = "...";
  header.wsa__ReplyTo = &replyTo;
  if (soap_call_ns__method(&soap, ...) != SOAP_OK)

// server-side source code
int soap_ns__method(struct soap *soap, ...)
{ if (!soap->header)
    return soap_sender_fault(soap, "No SOAP header, must send one", NULL);
  if (!soap->header->wsa__MessageID)
    return soap_sender_fault(soap, "No WS-Addressing MessageID", NULL);
  soap->header->wsa__RelatesTo = (struct wsa__Relationship*)soap_malloc(soap, sizeof(struct wsa__Relationship));
  soap_default_wsa__Relationship(soap, soap->header->wsa__RelatesTo);
  soap->header->wsa__RelatesTo->__item = soap->header->wsa__MessageID;
  if (!soap->header->wsa__ReplyTo || !soap->header->wsa__ReplyTo->Address)
    return soap_sender_fault(soap, "No WS-Addressing ReplyTo address", NULL);
  soap->header->wsa__To = soap->header->wsa__ReplyTo->Address;
  soap->header->wsa__MessageID = "...";
  soap->header->wsa__Action = "...";
  ...

*/

/******************************************************************************\
 *                                                                            *
 * http://schemas.xmlsoap.org/ws/2004/08/addressing                           *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Import                                                                     *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Schema Namespaces                                                          *
 *                                                                            *
\******************************************************************************/

//gsoap wsa   schema import:	http://schemas.xmlsoap.org/ws/2004/08/addressing
//gsoap wsa   schema elementForm:	qualified
//gsoap wsa   schema attributeForm:	unqualified

/******************************************************************************\
 *                                                                            *
 * Schema Types                                                               *
 *                                                                            *
\******************************************************************************/



/// Typedef synonym for struct wsa__EndpointReferenceType.
typedef struct wsa__EndpointReferenceType wsa__EndpointReferenceType;

/// Typedef synonym for struct wsa__ReferencePropertiesType.
typedef struct wsa__ReferencePropertiesType wsa__ReferencePropertiesType;

/// Typedef synonym for struct wsa__ReferenceParametersType.
typedef struct wsa__ReferenceParametersType wsa__ReferenceParametersType;

/// Typedef synonym for struct wsa__ServiceNameType.
typedef struct wsa__ServiceNameType wsa__ServiceNameType;

/// Typedef synonym for struct wsa__Relationship.
typedef struct wsa__Relationship wsa__Relationship;

/// Imported complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReplyAfterType from typemap WS/WS-typemap.dat.


/// Imported complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedQName from typemap WS/WS-typemap.dat.
typedef _QName wsa__AttributedQName;

/// Imported complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI from typemap WS/WS-typemap.dat.
typedef char* wsa__AttributedURI;

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":RelationshipTypeValues is a simpleType restriction of xs:QName.
/// Note: enum values are prefixed with 'wsa__RelationshipTypeValues' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum wsa__RelationshipTypeValues
{
	wsa__RelationshipTypeValues__wsa__Reply,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":Reply"
};
/// Typedef synonym for enum wsa__RelationshipTypeValues.
typedef enum wsa__RelationshipTypeValues wsa__RelationshipTypeValues;

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":FaultSubcodeValues is a simpleType restriction of xs:QName.
/// Note: enum values are prefixed with 'wsa__FaultSubcodeValues' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum wsa__FaultSubcodeValues
{
	wsa__FaultSubcodeValues__wsa__InvalidMessageInformationHeader,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":InvalidMessageInformationHeader"
	wsa__FaultSubcodeValues__wsa__MessageInformationHeaderRequired,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":MessageInformationHeaderRequired"
	wsa__FaultSubcodeValues__wsa__DestinationUnreachable,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":DestinationUnreachable"
	wsa__FaultSubcodeValues__wsa__ActionNotSupported,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":ActionNotSupported"
	wsa__FaultSubcodeValues__wsa__EndpointUnavailable,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointUnavailable"
};
/// Typedef synonym for enum wsa__FaultSubcodeValues.
typedef enum wsa__FaultSubcodeValues wsa__FaultSubcodeValues;

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType is a complexType.
struct wsa__EndpointReferenceType
{
/// Element Address of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI.
    wsa__AttributedURI                   Address                        1;	///< Required element.
/// Element ReferenceProperties of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReferencePropertiesType.
    struct wsa__ReferencePropertiesType*  ReferenceProperties            0;	///< Optional element.
/// Element ReferenceParameters of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReferenceParametersType.
    struct wsa__ReferenceParametersType*  ReferenceParameters            0;	///< Optional element.
/// Element PortType of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedQName.
    wsa__AttributedQName                 PortType                       0;	///< Optional element.
/// Element ServiceName of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":ServiceNameType.
    struct wsa__ServiceNameType*         ServiceName                    0;	///< Optional element.
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
    _XML                                 __any                         ;	///< Catch any element content in XML string.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
   @_XML                                 __anyAttribute                ;	///< Catch any attribute content in XML string.
};

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReference of complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType.
typedef struct wsa__EndpointReferenceType _wsa__EndpointReference;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":From of complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType.
typedef struct wsa__EndpointReferenceType _wsa__From;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReplyTo of complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType.
typedef struct wsa__EndpointReferenceType _wsa__ReplyTo;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":FaultTo of complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType.
typedef struct wsa__EndpointReferenceType _wsa__FaultTo;

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReferencePropertiesType is a complexType.
struct wsa__ReferencePropertiesType
{
/// TODO: <any minOccurs="0" maxOccurs="unbounded">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
    _XML                                 __any                         ;	///< Catch any element content in XML string.
};

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReferenceParametersType is a complexType.
struct wsa__ReferenceParametersType
{
/// TODO: <any minOccurs="0" maxOccurs="unbounded">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
    _XML                                 __any                         ;	///< Catch any element content in XML string.
};

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":ServiceNameType is a complexType with simpleContent.
struct wsa__ServiceNameType
{
/// __item wraps simpleContent.
    _QName                               __item                        ;
/// Attribute PortName of type xs:NCName.
   @char*                                PortName                       0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
   @_XML                                 __anyAttribute                ;	///< Catch any attribute content in XML string.
};

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":Relationship is a complexType with simpleContent.
struct wsa__Relationship
{
/// __item wraps simpleContent.
    char*                                __item                        ;
/// Attribute RelationshipType of type xs:QName.
   @_QName                               RelationshipType               0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
   @_XML                                 __anyAttribute                ;	///< Catch any attribute content in XML string.
};

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":RelatesTo of complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":Relationship.
typedef struct wsa__Relationship _wsa__RelatesTo;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReplyAfter of complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReplyAfterType.
typedef unsigned int _wsa__ReplyAfter;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":MessageID of complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI.
typedef wsa__AttributedURI _wsa__MessageID;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":To of complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI.
typedef wsa__AttributedURI _wsa__To;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":Action of complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI.
typedef wsa__AttributedURI _wsa__Action;

/* End of wsa.h */
