//gsoap ns1 service name: magic
//gsoap ns1 service namespace: http://websrv.cs.fsu.edu/~engelen/magic.wsdl
//gsoap ns1 service location: http://websrv.cs.fsu.edu/~engelen
//gsoap ns1 service executable: magicserver.cgi
//gsoap ns1 schema namespace: urn:MagicSquare

typedef int xsd__int;

class vector
{ public:
  xsd__int *__ptr;
  int __size;
  struct soap *soap;
  vector();
  vector(int);
  ~vector();
  void resize(int);
  int& operator[](int);
};

class matrix
{ public:
  vector *__ptr;
  int __size;
  struct soap *soap;
  matrix();
  matrix(int, int);
  ~matrix();
  void resize(int, int);
  vector& operator[](int);
};

int ns1__magic(xsd__int rank, matrix *result);
