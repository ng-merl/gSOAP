#include "soapH.h"
#include "t.nsmap"
int main() { time_t t; soap_call_t__gmt(soap_new(), "http://www.cs.fsu.edu/~engelen/gmtserver.cgi", "", t); return puts(ctime(&t)); }
