//gsoap ns service name: Quote
//gsoap ns service location: http://services.xmethods.net/soap
//gsoap ns service method-action: getQuote ""
//gsoap ns service encoding: encoded
//gsoap ns schema namespace: urn:xmethods-delayed-quotes
int ns__getQuote(char *symbol, float &Result);
