#ifndef UTILKERNELS_H
#define UTILKERNELS_H

#include <cstdio>

#define DEVICE __device__
#ifdef DEBUG_ENABLED
#define handleLastError() cudaDeviceSynchronize(); handle_error( cudaGetLastError(),"Kernel Error occured:\"", __LINE__, __FILE__)
#else
#define handleLastError()
#endif

void handle_error(cudaError_t err, const char* error, int line, const char* file) {
	if(err != cudaSuccess) std::cerr << error << cudaGetErrorString(err) << "\" in Line " << line << " in File " << file << std::endl;
}

struct identity {
	DEVICE uint2 operator()(const uint2& input) const {
		return input;
	}
};
struct trans {
	DEVICE uint2 operator()(const uint2& input) const {
		return make_uint2(input.y, input.x);
	}
};

template <class T = float, class AccessMode = identity>
class MatrixAccess;

template <class AccessMode>
struct GetTransposed;

template <typename T>
struct GetTransposed<MatrixAccess<T, identity> >
{
	typedef MatrixAccess<T, trans> type;
};

template <typename T>
struct GetTransposed<MatrixAccess<T, trans> >
{
	typedef MatrixAccess<T, identity> type;
};

template <class T, class AccessMode>
class MatrixAccess {
private:
	typedef MatrixAccess<T, AccessMode> This;
	unsigned int rows, cols;
	T* mat;
public:
	typedef T Type;
	DEVICE MatrixAccess() {}
	DEVICE MatrixAccess(T* mat, const unsigned int cols, const unsigned int rows) : 
		rows(rows),
		cols(cols),
		mat(mat)
	{}
	DEVICE MatrixAccess(const unsigned int cols, const unsigned int rows) : 
		rows(rows),
		cols(cols),
		mat(new T[cols*rows])
	{}
	DEVICE void finalize() {
		delete mat;
	}
	DEVICE typename GetTransposed<This>::type transpose() {
		return typename GetTransposed<This>::type(mat, cols, rows);
	}
	DEVICE T& operator[](const uint2 pos) {
		const uint2 pos2 = AccessMode()(pos);		
		return mat[pos2.y*cols+pos2.x];
	}
	DEVICE const T& operator[](const uint2 pos) const {
		const uint2 pos2 = AccessMode()(pos);		
		return mat[pos2.y*cols+pos2.x];
	}
	DEVICE unsigned int getRows() const {
		uint2 dim = make_uint2(cols, rows);
		dim = AccessMode()(dim);
		return dim.y;
	}
	DEVICE unsigned int getCols() const {
		uint2 dim = make_uint2(cols, rows);
		dim = AccessMode()(dim);
		return dim.x;
	}
	
	DEVICE T* getRawPointer() const {
		return mat;
	}
	#if DEBUG_ENABLED
	DEVICE void print() const {
		if(threadIdx.x == 0) {
			for(int x = 0; x < cols; x++) {
				for(int y = 0; y < rows; y++) {
					const uint2 pos = make_uint2(x,y);
					pos = AccessMode()(pos);
					printf("%f ", mat[pos.y*cols+pos.x]);
				}
				printf("\n");
			}
		}
	}
	#endif
};

template<unsigned int blockSize, class MatrixAccess1, class MatrixAccess2, class MatrixAccess3>
DEVICE void matProdKernel(MatrixAccess3& result, const MatrixAccess1& left, const MatrixAccess2& right, typename MatrixAccess1::Type* sleft) {	
	#ifdef DEBUG_ENABLED
	if(left.getCols() != right.getRows() && right.getCols() != result.getCols() && left.getRows() != result.getRows()) {
		printf("Can not multiply %ux%u with %ux%u to %ux%u matrix.\n", 
			left.getCols(),    left.getRows(), 
			right.getCols(),    right.getRows(), 
			result.getCols(), result.getRows());
	}
	#endif
	
	for(int xr = 0; xr < right.getCols(); xr++) {
		for(int y = 0; y < left.getRows(); y++) {
			typename MatrixAccess1::Type sright;
			sright = 0;
			for(int blocks = 0; blockSize*4*blocks < left.getCols(); blocks++) {
				const int x = threadIdx.x+blockSize*4*blocks;

				//Spalte der rechten Matrix in shared Mem laden
				for(int i = 0; i < 4; ++i) {
					const int j = x+i*blockSize;
					const int valid = j < right.getRows();
					const int address = j*valid;
					sright += valid*right[make_uint2(xr,address)]*left[make_uint2(address,y)];	
				}
				
				//Teilergebniss in Register speichern
				//if(threadIdx.x +threadIdx.y == 0 && blockIdx.x+blockIdx.y == 0) printf("%i += %i\n", res, sleft[0][threadIdx.y]);
			}
			sleft[threadIdx.x] = sright;
			//Vektor folden
			for(int i = blockSize/2; i >= 1; i/=2) {
				if(threadIdx.x < i) sleft[threadIdx.x] += sleft[threadIdx.x+i];
				__syncthreads();
			}
			//Register in Ergebniss Matrix schreiben
			if(threadIdx.x == 0) {
				result[make_uint2(xr, y)] = sleft[0];
			}
		}
	}
	__syncthreads();
}

#ifdef DEBUG_ENABLED
template <typename T, class AccessMode>
DEVICE void printMat(MatrixAccess<T, AccessMode>& mat) {
	printf("{");
	for(int j = 0; j < mat.getRows(); j++) {
		printf("{");
		for(int i = 0; i < mat.getCols(); i++) {
			printf("%f",mat[make_uint2(i,j)]);
			if(i<mat.getCols()-1) printf(", ");
		}
		printf("}");
		if(j<mat.getRows()-1) printf(",\n");
	}
	printf("}\n");
}


template <typename T, class AccessMode>
DEVICE void printIntMat(MatrixAccess<T, AccessMode>& mat) {
	for(int j = 0; j < mat.getRows(); j++) {
		for(int i = 0; i < mat.getCols(); i++) {
			printf("%i ",mat[make_uint2(i,j)]);
		}
		printf("\n");
	}
	printf("\n");
}
#endif

#endif
