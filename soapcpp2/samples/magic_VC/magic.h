typedef int xsd__int;
class vector
{ public:
	xsd__int	*__ptr;
	int		__size;
			vector();
			vector(int size);
			~vector();
	virtual	void	resize(int size);
	virtual int&	operator[](int idx);
};

class matrix
{ public:
	vector		*__ptr;
	int		__size;
			matrix();
			matrix(int rows, int cols);
			~matrix();
	virtual void	resize(int rows, int cols);
	virtual vector&	operator[](int idx);
};

int ns1__magic(xsd__int rank, matrix *&result);
