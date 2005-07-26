/*

wsa3.h

This is an older version of the WS-Addressing spec. See wsa.h for more details.

Generated with:
wsdl2h -cgy -o wsa3.h -t WS/WS-typemap.dat WS/WS-Addressing03.xsd

Modified by Robert van Engelen:

- Removed //gsoapopt
- Added the following directive to import WS-Addressing namespace:
  //gsoap wsa3 schema import: http://schemas.xmlsoap.org/ws/2003/03/addressing
  This ensures that the WS-Addressing schemas are not copied into the generated
  WSDL by soapcpp2 but are referenced with schema import in the generated WSDL.

*/

/******************************************************************************\
 *                                                                            *
 * http://schemas.xmlsoap.org/ws/2003/03/addressing                           *
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

//gsoap wsa3  schema import:	http://schemas.xmlsoap.org/ws/2003/03/addressing
//gsoap wsa3  schema elementForm:	qualified
//gsoap wsa3  schema attributeForm:	unqualified

/******************************************************************************\
 *                                                                            *
 * Schema Types                                                               *
 *                                                                            *
\******************************************************************************/



/// Typedef synonym for struct wsa3__EndpointReferenceType.
typedef struct wsa3__EndpointReferenceType wsa3__EndpointReferenceType;

/// Typedef synonym for struct wsa3__ReferencePropertiesType.
typedef struct wsa3__ReferencePropertiesType wsa3__ReferencePropertiesType;

/// Typedef synonym for struct wsa3__ServiceNameType.
typedef struct wsa3__ServiceNameType wsa3__ServiceNameType;

/// Typedef synonym for struct wsa3__Relationship.
typedef struct wsa3__Relationship wsa3__Relationship;

/// Imported complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":AttributedQName from typemap WS/WS-typemap.dat.
typedef _QName wsa3__AttributedQName;

/// Imported complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":AttributedURI from typemap WS/WS-typemap.dat.
typedef char* wsa3__AttributedURI;

/// "http://schemas.xmlsoap.org/ws/2003/03/addressing":RelationshipTypeValues is a simpleType restriction of xs:QName.
/// Note: enum values are prefixed with 'wsa3__RelationshipTypeValues' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum wsa3__RelationshipTypeValues
{
	wsa3__RelationshipTypeValues__wsa3__Response,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2003/03/addressing":Response"
};
/// Typedef synonym for enum wsa3__RelationshipTypeValues.
typedef enum wsa3__RelationshipTypeValues wsa3__RelationshipTypeValues;

/// "http://schemas.xmlsoap.org/ws/2003/03/addressing":EndpointReferenceType is a complexType.
struct wsa3__EndpointReferenceType
{
/// Element Address of type "http://schemas.xmlsoap.org/ws/2003/03/addressing":AttributedURI.
    wsa3__AttributedURI                  Address                        1;	///< Required element.
/// Element ReferenceProperties of type "http://schemas.xmlsoap.org/ws/2003/03/addressing":ReferencePropertiesType.
    struct wsa3__ReferencePropertiesType*  ReferenceProperties            0;	///< Optional element.
/// Element PortType of type "http://schemas.xmlsoap.org/ws/2003/03/addressing":AttributedQName.
    wsa3__AttributedQName                PortType                       0;	///< Optional element.
/// Element ServiceName of type "http://schemas.xmlsoap.org/ws/2003/03/addressing":ServiceNameType.
    struct wsa3__ServiceNameType*        ServiceName                    0;	///< Optional element.
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

/// Element "http://schemas.xmlsoap.org/ws/2003/03/addressing":EndpointReference of complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":EndpointReferenceType.
typedef struct wsa3__EndpointReferenceType _wsa3__EndpointReference;

/// Element "http://schemas.xmlsoap.org/ws/2003/03/addressing":From of complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":EndpointReferenceType.
typedef struct wsa3__EndpointReferenceType _wsa3__From;

/// Element "http://schemas.xmlsoap.org/ws/2003/03/addressing":ReplyTo of complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":EndpointReferenceType.
typedef struct wsa3__EndpointReferenceType _wsa3__ReplyTo;

/// Element "http://schemas.xmlsoap.org/ws/2003/03/addressing":FaultTo of complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":EndpointReferenceType.
typedef struct wsa3__EndpointReferenceType _wsa3__FaultTo;

/// Element "http://schemas.xmlsoap.org/ws/2003/03/addressing":Recipient of complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":EndpointReferenceType.
typedef struct wsa3__EndpointReferenceType _wsa3__Recipient;

/// "http://schemas.xmlsoap.org/ws/2003/03/addressing":ReferencePropertiesType is a complexType.
struct wsa3__ReferencePropertiesType
{
/// TODO: <any minOccurs="0" maxOccurs="unbounded">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
    _XML                                 __any                         ;	///< Catch any element content in XML string.
};

/// "http://schemas.xmlsoap.org/ws/2003/03/addressing":ServiceNameType is a complexType with simpleContent.
struct wsa3__ServiceNameType
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

/// "http://schemas.xmlsoap.org/ws/2003/03/addressing":Relationship is a complexType with simpleContent.
struct wsa3__Relationship
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

/// Element "http://schemas.xmlsoap.org/ws/2003/03/addressing":RelatesTo of complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":Relationship.
typedef struct wsa3__Relationship _wsa3__RelatesTo;

/// Element "http://schemas.xmlsoap.org/ws/2003/03/addressing":MessageID of complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":AttributedURI.
typedef wsa3__AttributedURI _wsa3__MessageID;

/// Element "http://schemas.xmlsoap.org/ws/2003/03/addressing":To of complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":AttributedURI.
typedef wsa3__AttributedURI _wsa3__To;

/// Element "http://schemas.xmlsoap.org/ws/2003/03/addressing":Action of complexType "http://schemas.xmlsoap.org/ws/2003/03/addressing":AttributedURI.
typedef wsa3__AttributedURI _wsa3__Action;

/* End of wsa3.h */
