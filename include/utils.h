#ifndef SPARSE_UTILS_UTILS_H
#define SPARSE_UTILS_UTILS_H

#include <algorithm>

#include "defs.h"
#include "sparse_matrix.h"

namespace SparseUtils {

// Compute y = alpha * A * x + beta * y
template <typename T>
void CSRMatVec(double alpha, int nrows, const int *rowp,
               const int *cols, const T *Avals, const T *x, double beta,
               T *y);

// Compute A * x -> y
template <typename T>
void CSCMatVec(double alpha, int nrows, int ncols, const int *colp,
               const int *rows, const T *Avals, const T *x, double beta,
               T *y);

// Based on the pattern of A, compute A^{T}. The numerical values are optional
template <typename T>
void SparseTranspose(int nrows, int ncols, const int *rowp,
                     const int *cols, const T *Avals, int *colp,
                     int *rows, T *ATvals);

// Compute the matrix-matrix product A * A^{T}
template <typename T>
void MatMatTransNumeric(int nrows, int ncols, const int *rowp,
                        const int *cols, const T *Avals,
                        const int *colp, const int *rows,
                        const T *ATvals, const int *Bcolp, int *Brows,
                        T *Bvals, int *flag, T *tmp);

// Compute the result C + A * D * A^{T}, where C and D are diagonal
template <typename T>
void MatMatTransNumeric(int nrows, int ncols, const T *cvals,
                        const int *rowp, const int *cols,
                        const T *Avals, const T *dvals, const int *colp,
                        const int *rows, const T *ATvals,
                        const int *Bcolp, int *Brows, T *Bvals,
                        int *flag, T *tmp);

// Compute the number of entries in the matrix product A * A^{T}
inline int MatMatTransSymbolic(int nrows, int ncols,
                                   const int *rowp, const int *cols,
                                   const int *colp, const int *rows,
                                   int *Bcolp, int *flag);

// Sort an array of length len, then remove duplicate entries and
// entries with values -1.
inline int RemoveDuplicates(int *array, int len,
                                int exclude = -1);

// Sort and make the data structure unique - remove diagonal
inline void SortAndRemoveDuplicates(int nvars, int *rowp, int *cols,
                                    int remove_diagonal = 0);

// Convert BSRMat to an unblocked, CSR format
template <typename T, int M, int N>
CSRMat<T> *bsr_to_csr(BSRMat<T, M, N> *bsr_mat);

// Convert BSRMat to an unblocked, CSC format
template <typename T, int M, int N>
CSCMat<T> *bsr_to_csc(BSRMat<T, M, N> *bsr_mat);

}  // namespace SparseUtils

#include "detail/utils_impl.h"

#endif  // SPARSE_UTILS_UTILS_H