namespace calc {

//gsoap ns service name: Service
//gsoap ns service location: http://www.cs.fsu.edu/~engelen/calc.cgi
//gsoap ns service namespace: urn:calc
//gsoap ns service encoding: encoded

int ns__add(double a, double b, double *result);
int ns__sub(double a, double b, double *result);
int ns__mul(double a, double b, double *result);
int ns__div(double a, double b, double *result);

}
