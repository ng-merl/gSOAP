namespace quote {

//gsoap ns service name: Service
//gsoap ns service location: http://services.xmethods.net/soap
//gsoap ns schema namespace: urn:xmethods-delayed-quotes
//gsoap ns service method-action: getQuote ""

int ns__getQuote(char *symbol, float &Result);

}
