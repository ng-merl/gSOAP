/*	stringtest.h

	Example use of std::string

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.
*/

#import "stlstring.h"

typedef std::string xsd__string;	// optional: associate xsd:string with std::string

class ArrayOfstring
{ public:
  xsd__string *__ptr;
  int __size;
  struct soap *soap; // holds soap struct when allocated by gSOAP or by soap_new_ArrayOfstring
  ArrayOfstring();
  virtual ~ArrayOfstring();
  void resize(int);
  xsd__string &operator[](int) const;
};

int ns__echoString(xsd__string inputString, struct ns__echoStringResponse { xsd__string _return; } &out);
int ns__echoStringArray(ArrayOfstring *_inputStringArray, ArrayOfstring *_return);
