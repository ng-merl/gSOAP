namespace rate {

//gsoap ns service name: Service
//gsoap ns service location: http://services.xmethods.net/soap
//gsoap ns schema namespace: urn:xmethods-CurrencyExchange
//gsoap ns service method-action: getRate ""

int ns__getRate(char *country1, char *country2, float &Result);

}
