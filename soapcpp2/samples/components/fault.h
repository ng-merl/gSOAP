/*	fault.h

	Defines optional SOAP Fault derail data structures

	Copyright (C) 2000-2004 Robert A. van Engelen. All Rights Reserved.

*/

/*

Add any data structure you want to serialize as part of the SOAP Fault detail
element. The detail element '__type' and 'value' fields should be set to
transmit data. The fields are set when data of corresponding types are received.

For example, we define an <element> of name <f:myData> with a string vector
(note the leading _ in the following declaration):

class _f__myData
{ public:
  std::vector<std::string*> *data;
};

*/
