/*

wsu.h

Generated with:
wsdl2h -cgy -o wsu.h -t WS/WS-typemap.dat WS/wsu.xsd

Modified by Robert van Engelen:

- Removed //gsoapopt
- Added the following directive to import wsu namespace
  //gsoap wsu schema import: http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd
  This ensures that the WS-Addressing schemas are not copied into the generated
  WSDL by soapcpp2 but are referenced with schema import in the generated WSDL.

*/

/******************************************************************************\
 *                                                                            *
 * http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd*
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

//gsoap wsu   schema namespace:	http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd
//gsoap wsu   schema elementForm:	qualified
//gsoap wsu   schema attributeForm:	unqualified

/******************************************************************************\
 *                                                                            *
 * Schema Types                                                               *
 *                                                                            *
\******************************************************************************/



/// Typedef synonym for struct wsu__AttributedDateTime.
typedef struct wsu__AttributedDateTime wsu__AttributedDateTime;

/// Typedef synonym for struct wsu__AttributedURI.
typedef struct wsu__AttributedURI wsu__AttributedURI;

/// Typedef synonym for struct wsu__TimestampType.
typedef struct wsu__TimestampType wsu__TimestampType;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":tTimestampFault is a simpleType restriction of xs:QName.
/// @brief This type defines the fault code value for Timestamp message expiration.
/// Note: enum values are prefixed with 'wsu__tTimestampFault' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum wsu__tTimestampFault
{
	wsu__tTimestampFault__wsu__MessageExpired,	///< xs:QName value=""http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":MessageExpired"
};
/// Typedef synonym for enum wsu__tTimestampFault.
typedef enum wsu__tTimestampFault wsu__tTimestampFault;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":TimestampType is a complexType.
/// @brief This complex type ties together the timestamp related elements into a composite type.
struct wsu__TimestampType
{
/// Element reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Created.
/// @brief This element allows a creation time to be applied anywhere element wildcards are present.
    struct wsu__AttributedDateTime*      Created                        0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Expires.
/// @brief This element allows an expiration time to be applied anywhere element wildcards are present.
    struct wsu__AttributedDateTime*      Expires                        0;	///< Optional element.
/// CHOICE OF ELEMENTS <choice minOccurs="0" maxOccurs="unbounded">
    int                                  __sizeunion_1                  0;
struct __wsu__union_1
{
    int                                  __union_1                      0;	///< Union wsu__union_1 selector: set to SOAP_UNION_wsu__union_1_<fieldname> or 0
union wsu__union_1
{
/// TODO: <any namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
    _XML                                 __any                         ;	///< Catch any element content in XML string.
    }                                    union_1                       ;
    }                                   *__union_1                     ;
//  END OF CHOICE
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
/// @brief This global attribute supports annotating arbitrary elements with an ID.
   @char*                                wsu__Id                        0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
   @_XML                                 __anyAttribute                ;	///< Catch any attribute content in XML string.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Timestamp of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":TimestampType.
/// @brief This element allows Timestamps to be applied anywhere element wildcards are present, including as a SOAP header.
typedef struct wsu__TimestampType _wsu__Timestamp;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":AttributedDateTime is a complexType with simpleContent.
/// @brief This type is for elements whose [children] is a psuedo-dateTime and can have arbitrary attributes.
struct wsu__AttributedDateTime
{
/// __item wraps simpleContent.
    char*                                __item                        ;
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
/// @brief This global attribute supports annotating arbitrary elements with an ID.
   @char*                                wsu__Id                        0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
   @_XML                                 __anyAttribute                ;	///< Catch any attribute content in XML string.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Expires of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":AttributedDateTime.
/// @brief This element allows an expiration time to be applied anywhere element wildcards are present.
typedef struct wsu__AttributedDateTime _wsu__Expires;

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Created of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":AttributedDateTime.
/// @brief This element allows a creation time to be applied anywhere element wildcards are present.
typedef struct wsu__AttributedDateTime _wsu__Created;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":AttributedURI is a complexType with simpleContent.
/// @brief This type is for elements whose [children] is an anyURI and can have arbitrary attributes.
struct wsu__AttributedURI
{
/// __item wraps simpleContent.
    char*                                __item                        ;
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
/// @brief This global attribute supports annotating arbitrary elements with an ID.
   @char*                                wsu__Id                        0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
   @_XML                                 __anyAttribute                ;	///< Catch any attribute content in XML string.
};

/* End of wsu.h */
