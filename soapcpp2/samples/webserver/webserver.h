/*

webserver.h

gSOAP XML Web services tools
Copyright (C) 2004, Robert van Engelen, Genivia, Inc. All Rights Reserved.

--------------------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org
--------------------------------------------------------------------------------
*/

//gsoap ns service name: webserver
//gsoap ns service namespace: http://www.cs.fsu.edu/~engelen/webserver.wsdl
//gsoap ns schema namespace: urn:demowebserver
//gsoap ns schema location: http://localhost:8080

int ns__add(double, double, double*);		// HTTP POST request - response
int ns__sub(double, double, double*);		// HTTP POST request - response
int ns__mul(double, double, double*);		// HTTP POST request - response
int ns__div(double, double, double*);		// HTTP POST request - response

int ns__addResponse_(double, void);	// response to HTTP GET
int ns__subResponse_(double, void);	// response to HTTP GET
int ns__mulResponse_(double, void);	// response to HTTP GET
int ns__divResponse_(double, void);	// response to HTTP GET
