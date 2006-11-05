/*

struct_tm.h

Custom serializer for <time.h> struct tm

#import this file into your gSOAP .h file to enable struct tm serialization and
use the serializable xsd__dateTime type.

To automate the wsdl2h-mapping of xsd:dateTime to struct tm, add this line to
the typemap.dat file:

xsd__dateTime = #import "custom/struct_tm.h" | xsd__dateTime

The typemap.dat file is used by wsdl2h to map types (use wsdl2h option -t).

Link your code with struct_tm.c

gSOAP XML Web services tools
Copyright (C) 2000-2005, Robert van Engelen, Genivia Inc., All Rights Reserved.
This part of the software is released under one of the following licenses:
GPL, the gSOAP public license, or Genivia's license for commercial use.
--------------------------------------------------------------------------------
gSOAP public license.

The contents of this file are subject to the gSOAP Public License Version 1.3
(the "License"); you may not use this file except in compliance with the
License. You may obtain a copy of the License at
http://www.cs.fsu.edu/~engelen/soaplicense.html
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.

The Initial Developer of the Original Code is Robert A. van Engelen.
Copyright (C) 2000-2005, Robert van Engelen, Genivia, Inc., All Rights Reserved.
--------------------------------------------------------------------------------
GPL license.

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
A commercial use license is available from Genivia, Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

#include <time.h>

volatile struct tm
{	int	tm_sec;		// seconds (0 - 60)
	int	tm_min;		// minutes (0 - 59)
	int	tm_hour;	// hours (0 - 23)
	int	tm_mday;	// day of month (1 - 31)
	int	tm_mon;		// month of year (0 - 11)
	int	tm_year;	// year - 1900
	int	tm_wday;	// day of week (Sunday = 0)
	int	tm_yday;	// day of year (0 - 365)
	int	tm_isdst;	// is summer time in effect?
	char*	tm_zone;	// abbreviation of timezone
};

extern typedef struct tm xsd__dateTime;
