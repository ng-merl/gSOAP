/*

wsse.h

Generated with:
wsdl2h -cgy -x -o wsse.h -t WS/WS-typemap.dat WS/wsse.xsd

- Removed //gsoapopt
- Replace #import "wsu.h" with schema import since only wsu__Id is needed
- Added //gsoap wsse  schema import: http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd

*/

/******************************************************************************\
 *                                                                            *
 * http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd*
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

//gsoap wsu   schema import:    http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd
//gsoap wsu   schema elementForm:       qualified
//gsoap wsu   schema attributeForm:     unqualified

//gsoap wsse  schema import:	http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd
//gsoap wsse  schema elementForm:	qualified
//gsoap wsse  schema attributeForm:	unqualified

/******************************************************************************\
 *                                                                            *
 * Schema Types                                                               *
 *                                                                            *
\******************************************************************************/


/// Imported attribute "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id from typemap WS/WS-typemap.dat.
typedef char *wsu__Id;


/// Typedef synonym for struct wsse__AttributedString.
typedef struct wsse__AttributedString wsse__AttributedString;

/// Typedef synonym for struct wsse__PasswordString.
typedef struct wsse__PasswordString wsse__PasswordString;

/// Typedef synonym for struct wsse__EncodedString.
typedef struct wsse__EncodedString wsse__EncodedString;

/// Typedef synonym for struct wsse__UsernameTokenType.
typedef struct wsse__UsernameTokenType wsse__UsernameTokenType;

/// Typedef synonym for struct wsse__BinarySecurityTokenType.
typedef struct wsse__BinarySecurityTokenType wsse__BinarySecurityTokenType;

/// Typedef synonym for struct wsse__KeyIdentifierType.
typedef struct wsse__KeyIdentifierType wsse__KeyIdentifierType;

/// Typedef synonym for struct wsse__ReferenceType.
typedef struct wsse__ReferenceType wsse__ReferenceType;

/// Typedef synonym for struct wsse__EmbeddedType.
typedef struct wsse__EmbeddedType wsse__EmbeddedType;

/// Typedef synonym for struct wsse__SecurityTokenReferenceType.
typedef struct wsse__SecurityTokenReferenceType wsse__SecurityTokenReferenceType;

/// Typedef synonym for struct wsse__SecurityHeaderType.
typedef struct wsse__SecurityHeaderType wsse__SecurityHeaderType;

/// Typedef synonym for struct wsse__TransformationParametersType.
typedef struct wsse__TransformationParametersType wsse__TransformationParametersType;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":tUsage is a simpleType containing a whitespace separated list of xs:anyURI.
/// @brief Typedef to allow a list of usages (as URIs).
typedef char* wsse__tUsage;

/// Attribute "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":Usage of simpleType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":tUsage.
/// @brief This global attribute is used to indicate the usage of a referenced or indicated token within the containing context
typedef wsse__tUsage _wsse__Usage;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":FaultcodeEnum is a simpleType restriction of xs:QName.
/// Note: enum values are prefixed with 'wsse__FaultcodeEnum' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum wsse__FaultcodeEnum
{
	wsse__FaultcodeEnum__wsse__UnsupportedSecurityToken,	///< xs:QName value=""http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":UnsupportedSecurityToken"
	wsse__FaultcodeEnum__wsse__UnsupportedAlgorithm,	///< xs:QName value=""http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":UnsupportedAlgorithm"
	wsse__FaultcodeEnum__wsse__InvalidSecurity,	///< xs:QName value=""http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":InvalidSecurity"
	wsse__FaultcodeEnum__wsse__InvalidSecurityToken,	///< xs:QName value=""http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":InvalidSecurityToken"
	wsse__FaultcodeEnum__wsse__FailedAuthentication,	///< xs:QName value=""http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":FailedAuthentication"
	wsse__FaultcodeEnum__wsse__FailedCheck,	///< xs:QName value=""http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":FailedCheck"
	wsse__FaultcodeEnum__wsse__SecurityTokenUnavailable,	///< xs:QName value=""http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":SecurityTokenUnavailable"
};
/// Typedef synonym for enum wsse__FaultcodeEnum.
typedef enum wsse__FaultcodeEnum wsse__FaultcodeEnum;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":UsernameTokenType is a complexType.
/// @brief This type represents a username token per Section 4.1
struct wsse__UsernameTokenType
{
/// Element Username of type "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":AttributedString.
    struct wsse__AttributedString*       Username                       1;	///< Required element.
/// TODO: <any minOccurs="0" maxOccurs="unbounded">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
   @wsu__Id                              wsu__Id_                       0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":UsernameToken of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":UsernameTokenType.
/// @brief This element defines the wsse:UsernameToken element per Section 4.1.
typedef struct wsse__UsernameTokenType _wsse__UsernameToken;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":ReferenceType is a complexType.
/// @brief This type represents a reference to an external security token.
struct wsse__ReferenceType
{
/// Attribute URI of type xs:anyURI.
   @char*                                URI                            0;	///< Optional attribute.
/// Attribute ValueType of type xs:anyURI.
   @char*                                ValueType                      0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":Reference of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":ReferenceType.
/// @brief This element defines a security token reference
typedef struct wsse__ReferenceType _wsse__Reference;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":EmbeddedType is a complexType.
/// @brief This type represents a reference to an embedded security token.
struct wsse__EmbeddedType
{
/// CHOICE OF ELEMENTS <choice minOccurs="0" maxOccurs="unbounded">
    int                                  __sizeunion_1                  0;
struct __wsse__union_1
{
    int                                  __union_1                      0;	///< Union wsse__union_1 selector: set to SOAP_UNION_wsse__union_1_<fieldname> or 0
union wsse__union_1
{
/// TODO: <any>
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
    }                                    union_1                       ;
    }                                   *__union_1                     ;
//  END OF CHOICE
/// Attribute ValueType of type xs:anyURI.
   @char*                                ValueType                      0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":Embedded of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":EmbeddedType.
/// @brief This element defines a security token embedded reference
typedef struct wsse__EmbeddedType _wsse__Embedded;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":SecurityTokenReferenceType is a complexType.
/// @brief This type is used reference a security token.
struct wsse__SecurityTokenReferenceType
{
/// CHOICE OF ELEMENTS <choice minOccurs="0" maxOccurs="unbounded">
    int                                  __sizeunion_2                  0;
struct __wsse__union_2
{
    int                                  __union_2                      0;	///< Union wsse__union_2 selector: set to SOAP_UNION_wsse__union_2_<fieldname> or 0
union wsse__union_2
{
/// TODO: <any>
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
    }                                    union_2                       ;
    }                                   *__union_2                     ;
//  END OF CHOICE
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
   @wsu__Id                              wsu__Id_                       0;	///< Optional attribute.
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":Usage.
/// @brief This global attribute is used to indicate the usage of a referenced or indicated token within the containing context
   @wsse__tUsage                         wsse__Usage                    0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":SecurityTokenReference of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":SecurityTokenReferenceType.
/// @brief This element defines the wsse:SecurityTokenReference per Section 4.3.
typedef struct wsse__SecurityTokenReferenceType _wsse__SecurityTokenReference;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":SecurityHeaderType is a complexType.
/// @brief This complexType defines header block to use for security-relevant data directed at a specific SOAP actor.
struct wsse__SecurityHeaderType
{
/// TODO: <any minOccurs="0" maxOccurs="unbounded">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":Security of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":SecurityHeaderType.
/// @brief This element defines the wsse:Security SOAP header element per Section 4.
typedef struct wsse__SecurityHeaderType _wsse__Security;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":TransformationParametersType is a complexType.
/// @brief This complexType defines a container for elements to be specified from any namespace as properties/parameters of a DSIG transformation.
struct wsse__TransformationParametersType
{
/// TODO: <any minOccurs="0" maxOccurs="unbounded">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this element.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":TransformationParameters of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":TransformationParametersType.
/// @brief This element contains properties for transformations from any namespace, including DSIG.
typedef struct wsse__TransformationParametersType _wsse__TransformationParameters;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":AttributedString is a complexType with simpleContent.
/// @brief This type represents an element with arbitrary attributes.
struct wsse__AttributedString
{
/// __item wraps simpleContent.
    char*                                __item                        ;
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
   @wsu__Id                              wsu__Id_                       0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":PasswordString is a complexType with simpleContent.
/// @brief This type is used for password elements per Section 4.1.
struct wsse__PasswordString
{
/// __item wraps simpleContent.
    char*                                __item                        ;
/// Attribute Type of type xs:anyURI.
   @char*                                Type                           0;	///< Optional attribute.
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
   @wsu__Id                              wsu__Id_                       0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":Password of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":PasswordString.
typedef struct wsse__PasswordString _wsse__Password;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":EncodedString is a complexType with simpleContent.
/// @brief This type is used for elements containing stringified binary data.
struct wsse__EncodedString
{
/// __item wraps simpleContent.
    char*                                __item                        ;
/// Attribute EncodingType of type xs:anyURI.
   @char*                                EncodingType                   0;	///< Optional attribute.
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
   @wsu__Id                              wsu__Id_                       0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":Nonce of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":EncodedString.
typedef struct wsse__EncodedString _wsse__Nonce;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":BinarySecurityTokenType is a complexType with simpleContent.
/// @brief A security token that is encoded in binary
struct wsse__BinarySecurityTokenType
{
/// __item wraps simpleContent.
    char*                                __item                        ;
/// Attribute ValueType of type xs:anyURI.
   @char*                                ValueType                      0;	///< Optional attribute.
/// Attribute EncodingType of type xs:anyURI.
   @char*                                EncodingType                   0;	///< Optional attribute.
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
   @wsu__Id                              wsu__Id_                       0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":BinarySecurityToken of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":BinarySecurityTokenType.
/// @brief This element defines the wsse:BinarySecurityToken element per Section 4.2.
typedef struct wsse__BinarySecurityTokenType _wsse__BinarySecurityToken;

/// "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":KeyIdentifierType is a complexType with simpleContent.
/// @brief A security token key identifier
struct wsse__KeyIdentifierType
{
/// __item wraps simpleContent.
    char*                                __item                        ;
/// Attribute ValueType of type xs:anyURI.
   @char*                                ValueType                      0;	///< Optional attribute.
/// Attribute EncodingType of type xs:anyURI.
   @char*                                EncodingType                   0;	///< Optional attribute.
/// Attribute reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Id.
   @wsu__Id                              wsu__Id_                       0;	///< Optional attribute.
/// TODO: <anyAttribute namespace="##other">
///       Schema extensibility is user-definable.
///       Consult the protocol documentation to change and/or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
};

/// Element "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":KeyIdentifier of complexType "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":KeyIdentifierType.
/// @brief This element defines a key identifier reference
typedef struct wsse__KeyIdentifierType _wsse__KeyIdentifier;

/* End of wsse.h */
