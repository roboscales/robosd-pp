#ifndef robo_matrix_hpp
#define  robo_matrix_hpp
namespace robo {
	template <typename math> class matrix_t {
        void clear_(void) {
            if (memo != nullptr) {
                for (int i = 0; i < cols; ++i) {
                    delete[] memo[i];
                }
                delete[] memo;
                memo = nullptr;
            }

        }
        void init_(int _rows, int _cols) {
            rows = _rows;
            cols = _cols;
            if (rows > 0 && _cols > 0) {
                memo = new  real *[rows];
                for (int i = 0; i < rows; ++i) {
                    memo[i] = new real[cols];
                }
            }
        }
       public:
		typedef  typename math::real real;
		real** memo;
		int rows;
		int cols;

		matrix_t(int _rows, int _cols) {
            memo = nullptr;
            init_(_rows, _cols);
		}
        matrix_t(void) {
            memo = nullptr;
        }
        matrix_t(const matrix_t & _src) {
            (*this) = _src;
        }
        ~matrix_t(void) {
            clear_();
		}
        void reinit(int _rows, int _cols) {
            if (rows != _rows || cols != _cols) {
                clear_();
                init_(_rows, _cols);
            }
        }
        void eye(void) {
            for (int i = 0; i < cols; ++i) {
                math::fill(memo[i], rows, 0.);
                memo[i][i] = 1.;
            }
        }
        void zeros(void) {
            for (int i = 0; i < cols; ++i) {
                math::fill(IM.[i], rows, 0.);
            }
        }
        void invto(matrix_t & IM) {
            math::assert(cols==rows);
            IM.reinit(cols, cols);
            int i, j, k;
            int I, K; ///ÀÀÀÀÀÀ
            real l;
            IM.eye();


            //_________________________________________________

            for (I = 0; I < cols; I++) {
                if (memo[I][I] == 0) {
                    K = I;
                    while (memo[K][I] == 0) {
                        K++;
                    }
                    for (i = 0; i < cols; i++) {
                        for (j = 0; j < cols; j++) {
                            real tmp;
                            tmp = memo[i][j];
                            memo[i][j] = memo[K][j];
                            memo[K][j] = tmp;
                            tmp = IM.memo[i][j];
                            IM.memo[i][j] = IM.memo[K][j];
                            IM.memo[K][j] = tmp;
                        }
                    }
                }
                l = memo[I][I];
                for (k = 0; k < cols; ++k) {
                    memo[I][k] /= l;
                    IM.memo[I][k] /= l;
                }
                for (k = 0; k < I; ++k) {
                    real r = memo[k][I];
                    for (j = I; j < cols; ++j) {
                        memo[k][j] -= memo[I][j] * r;
                    }
                    for (j = 0; j < cols; ++j) {
                        IM.memo[k][j] -= IM.memo[I][j] * r;
                    }
                }
                for (k = I + 1; k < cols; ++k) {
                    real r = memo[k][I];
                    for (j = I; j < cols; ++j) {
                        memo[k][j] -= memo[I][j] * r;
                    }
                    for (j = 0; j < cols; ++j) {
                        IM.memo[k][j] -= IM.memo[I][j] * r;
                    }
                }
            }
		}
        void multto( const matrix_t & M2, matrix_t& R) {
            int rows1 = rows;
            int cols1 = cols; 
            int cols2 = M2.cols;
            math::assert( cols == M2.rows );
            R.reinit(rows1, cols2);

            int i, j, k;
            for (i = 0; i < rows1; i++)
                for (j = 0; j < cols2; j++) {
                    real tmp = 0;
                    for (k = 0; k < cols1; k++)
                        tmp += (memo[i][k] * M2.memo[k][j]);
                    R.memo[i][j] = tmp;
                }
        }

        void trans_sqrto(matrix_t & R) {
            R.reinit(rows, rows);
            int i, j, k;
            for (i = 0; i < rows; i++)
                for (j = 0; j < rows; j++) {
                    real tmp = 0;
                    for (k = 0; k < cols; k++)
                        tmp += memo[i][k] * memo[j][k];
                    R.memo[i][j] = tmp;
                }
        }

        void pinvto(matrix_t& IM, matrix_t& TS, matrix_t& ITS) {
            int i, j, k;
            trans_sqrto(TS);
            TS.invto(ITS);
            for (i = 0; i < cols; i++)
                for (j = 0; j < rows; j++) {
                    real tmp = 0;
                    for (k = 0; k < rows; k++)
                        tmp += memo[k][i] * ITS.memo[k][j];
                    IM.memo[i][j] = tmp;
                }
        }

        matrix_t& operator = (const matrix_t & SRC) {
            reinit(SRC.rows, SRC.cols);
            int i, j;
            for (i = 0; i < rows; i++)
                for (j = 0; j < cols; j++)
                    memo[i][j] = SRC.memo[i][j];
            return *this;
        }

	};
	
}

#endif
