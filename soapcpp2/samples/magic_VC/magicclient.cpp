#include "soapH.h"

////////////////////////////////////////////////////////////////////////////////
//
//	Magic Squares Client
//
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{ struct soap *soap;
  int r;
  matrix *A;
  if (argc <= 1)
#ifdef WIN32
    r = 10;
#else
  { if ((r = atoi(getenv("QUERY_STRING"))) == 0)
      r = rand()%20;
  }
#endif
  else
    r = atoi(argv[1]);
  printf("Content-type: text/html\r\n\r\n<html><h1>Magic Square of Rank %d</h1><pre>\n", r);
  soap = soap_new();	// allocate and initialize gSOAP runtime environment
  if (soap_call_ns1__magic(soap, "http://www.cs.fsu.edu/~engelen/magicserver.cgi", NULL, r, A))
  { soap_print_fault(soap, stderr);
    soap_print_fault_location(soap, stderr);
    exit(-1);
  }
  for (int i = 0; i < A->__size; i++)
  { for (int j = 0; j < (*A)[i].__size; j++)
      printf("%4d", (*A)[i][j]);
    printf("\n");
  }
  printf("</pre></html>\n");
  soap_end(soap);
  free(soap);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//	Class vector Methods
//
////////////////////////////////////////////////////////////////////////////////

vector::vector()
{ __ptr = 0;
  __size = 0;
}

vector::vector(int size)
{ __ptr = (int*)malloc(size*sizeof(int));
  __size = size;
}

vector::~vector()
{ if (__ptr)
    free(__ptr);
}

void vector::resize(int size)
{ int *p;
  if (__size == size)
    return;
  p = (int*)malloc(size*sizeof(int));
  if (__ptr)
  { for (int i = 0; i < (size <= __size ? size : __size); i++)
      p[i] = __ptr[i];
    free(__ptr);
  }
  __ptr = p;
  __size = size;
}

int& vector::operator[](int idx)
{ if (!__ptr || idx < 0 || idx >= __size)
    fprintf(stderr, "Array index out of bounds\n");
  return __ptr[idx];
}

////////////////////////////////////////////////////////////////////////////////
//
//	Class matrix Methods
//
////////////////////////////////////////////////////////////////////////////////

matrix::matrix()
{ __ptr = 0;
  __size = 0;
}

matrix::matrix(int rows, int cols)
{ 
#ifdef WIN32
  __ptr = new vector[rows];
  for (int i = 0; i < cols; i++)
    __ptr[i].resize(cols);
#else
  __ptr = new vector[rows](cols); // Visual C++ doesn't like this
#endif
  __size = rows;
}

matrix::~matrix()
{ if (__ptr)
    delete[] __ptr;
}

void matrix::resize(int rows, int cols)
{ int i;
  vector *p;
  if (__size != rows)
  { if (__ptr)
    { p = new vector[rows];
      for (i = 0; i < (rows <= __size ? rows : __size); i++)
      { if (this[i].__size != cols)
          (*this)[i].resize(cols);
	(p+i)->__ptr = __ptr[i].__ptr;
	(p+i)->__size = cols;
      }
      for (; i < rows; i++)
        __ptr[i].resize(cols);
    }
    else
    { 
#ifdef WIN32
      __ptr = new vector[rows];
      for (i = 0; i < rows; i++)
        __ptr[i].resize(cols);
#else
      __ptr = new vector[rows](cols); // Visual C++ doesn't like this
#endif
      __size = rows;
    }
  }
  else
    for (i = 0; i < __size; i++)
      __ptr[i].resize(cols);
}

vector& matrix::operator[](int idx)
{ if (!__ptr || idx < 0 || idx >= __size)
    fprintf(stderr, "Array index out of bounds\n");
  return __ptr[idx];
}

////////////////////////////////////////////////////////////////////////////////
//
//	Namespace Definition Table
//
////////////////////////////////////////////////////////////////////////////////

struct Namespace namespaces[] =
{ { "SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/" }, // must be first
  { "SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/" }, // must be second
  { "xsi", "http://www.w3.org/1999/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance" },
  { "xsd", "http://www.w3.org/1999/XMLSchema", "http://www.w3.org/*/XMLSchema" },
  { "ns1", "urn:MagicSquare" },
  { NULL, NULL }
};
