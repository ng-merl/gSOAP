
Thanks for using gSOAP!

SOAP Web Services is an emerging technology. SOAP enables cross-platform
development of networked applications.

* The gSOAP compiler and run-time are stable since version 2.1.3

* The gSOAP WSDL importer tool is BETA 0.9.1

* Numerous examples can be found in the 'samples' directory. See also
  NOTES.txt

The software is provided "as is", without any warranty. However, gSOAP
has received a lot of support from users and has been extensively tested
in the real world. We also continue to improve gSOAP and add new features.

INSTALLATION:
See NOTES.txt

DISCLAIMER:
WE TRY OUR BEST TO PROVIDE YOU WITH "REAL-WORLD" EXAMPLES BUT WE CANNOT
GUARANTEE THAT ALL CLIENT EXAMPLES CAN CONNECT TO THIRD PARTY WEB SERVICES
WHEN THESE SERVICES ARE DOWN OR HAVE BEEN REMOVED.

LICENSE:
gSOAP is distributed under the gSOAP public open source license which is based
on the Mozilla public license 1.1.  See license.html or license.pdf for
further details.  The license allows for commercial use of gSOAP. It also
allows products to be built on top and distributed under any license
(including commercial).  Products developed with gSOAP should include a notice
of copyright and a disclaimer of warranty in the product's documentation
(License Exhibit B).  gSOAP source code modifications that are distributed
part of an open source product should be submitted back to us for quality
control. Please note that modifications to the gSOAP runtime source codes are
not required to build applications so this requirement should not prohibit
(commercial) product development in any way.

COPYRIGHT:
gSOAP is copyrighted by Robert A. van Engelen, Genivia inc.
Copyright (C) 2000-2003 Robert A. van Engelen, Genivia inc. All Rights Reserved.

USE RESTRICTIONS:
You may not: (i) transfer rights to gSOAP or claim authorship; or (ii) remove
any product identification, copyright, proprietary notices or labels from gSOAP.

DOCS:
See NOTES.txt for installation instructions.
See soapdoc2.html or soapdoc2.pdf with the gSOAP documentation.

LIBS:
Win32 build of clients and services requires winsock.dll. To do this in
Visual C++ 6.0, go to "Project", "settings", select the "Link" tab (the
project file needs to be selected in the file view) and add "wsock32.lib" to
the "Object/library modules" entry. The Win32 distribution contains two
MSVC++ project examples. The custom build in VC++ 6.0 has been configured to
invoke the gSOAP compiler automatically. The VC++ projects can be converted to
MSVC++ 7.0.

