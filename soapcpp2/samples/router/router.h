/*	router.h

	Simple Web Service message router (relay server)

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

*/

struct t__Routing
{ char *key;
  char *endpoint;
};

struct RoutingTable
{ struct t__Routing *__ptr;
  int __size;
};
