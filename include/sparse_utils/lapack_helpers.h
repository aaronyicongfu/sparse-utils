#ifndef SPARSE_UTILS_LAPACK_H
#define SPARSE_UTILS_LAPACK_H
#include <complex.h>

#ifdef __APPLE__
#define ACCELERATE_NEW_LAPACK
#include <Accelerate/Accelerate.h>
#else
#include <cblas.h>
#include <lapack.h>
#endif

#include <complex>

namespace SparseUtils {

/**
 * To add more BLAS wrappers, refer to the official API at
 * https://www.netlib.org/blas/cblas.h
 */

/* BLAS/LAPACK functions for double precision real datatype */

// Level 1 BLAS routines

inline double BLASdot(int n, double *x, int incx, double *y, int incy) {
  return cblas_ddot(n, x, incx, y, incy);
}
inline double BLASnrm2(int n, double *x, int incx) {
  return cblas_dnrm2(n, x, incx);
}
inline void BLASaxpy(int n, double a, double *x, int incx, double *y,
                     int incy) {
  return cblas_daxpy(n, a, x, incx, y, incy);
}
inline void BLASscal(int n, double a, double *x, int incx) {
  return cblas_dscal(n, a, x, incx);
}

// Compute C := alpha*A*A**double + beta*C or C := alpha*A**double*A + beta*C
inline void BLASsyrk(enum CBLAS_ORDER order, enum CBLAS_UPLO uplo,
                     enum CBLAS_TRANSPOSE trans, int n, int k, double alpha,
                     double *a, int lda, double beta, double *c, int ldc) {
  return cblas_dsyrk(order, uplo, trans, n, k, alpha, a, lda, beta, c, ldc);
}

// Solve A*x = b or A^double*x = b where A is in packed format
inline void BLAStpsv(enum CBLAS_ORDER order, enum CBLAS_UPLO uplo,
                     enum CBLAS_TRANSPOSE transa, enum CBLAS_DIAG diag, int n,
                     double *a, double *x, int incx) {
  return cblas_dtpsv(order, uplo, transa, diag, n, a, x, incx);
}

// Level 2 BLAS routines
// y = alpha * A * x + beta * y, for a general matrix
inline void BLASgemv(enum CBLAS_ORDER order, enum CBLAS_TRANSPOSE transa, int m,
                     int n, double alpha, double *a, int lda, double *x,
                     int incx, double beta, double *y, int incy) {
  return cblas_dgemv(order, transa, m, n, alpha, a, lda, x, incx, beta, y,
                     incy);
}

// Level 3 BLAS routines
// C := alpha*op( A )*op( B ) + beta*C,
inline void BLASgemm(const enum CBLAS_ORDER Order,
                     const enum CBLAS_TRANSPOSE TransA,
                     const enum CBLAS_TRANSPOSE TransB, const int M,
                     const int N, const int K, const double alpha,
                     const double *A, const int lda, const double *B,
                     const int ldb, const double beta, double *C,
                     const int ldc) {
  return cblas_dgemm(Order, TransA, TransB, M, N, K, alpha, A, lda, B, ldb,
                     beta, C, ldc);
}

// General factorization routines
inline void LAPACKgetrf(int *m, int *n, double *a, int *lda, int *ipiv,
                        int *info) {
  return dgetrf_(m, n, a, lda, ipiv, info);
}
inline void LAPACKgetrs(const char *c, int *n, int *nrhs, double *a, int *lda,
                        int *ipiv, double *b, int *ldb, int *info) {
  return dgetrs_(c, n, nrhs, a, lda, ipiv, b, ldb, info);
}
inline void LAPACKgetri(int *n, double *a, int *lda, int *ipiv, double *work,
                        int *lwork, int *info) {
  return dgetri_(n, a, lda, ipiv, work, lwork, info);
}

// Factorization of packed-storage matrices
inline void LAPACKpptrf(const char *c, int *n, double *ap, int *info) {
  return dpptrf_(c, n, ap, info);
}
inline void LAPACKpptrs(const char *c, int *n, int *nrhs, double *ap,
                        double *rhs, int *ldrhs, int *info) {
  return dpptrs_(c, n, nrhs, ap, rhs, ldrhs, info);
}

/* BLAS/LAPACK functions for double precision complex datatype */

// Level 1 BLAS routines
inline std::complex<double> BLASdot(int n, std::complex<double> *x, int incx,
                                    std::complex<double> *y, int incy) {
  std::complex<double> ret;
  cblas_zdotu_sub(n, x, incx, y, incy, &ret);
  return ret;
}
inline double BLASnrm2(int n, std::complex<double> *x, int incx) {
  return cblas_dznrm2(n, x, incx);
}
inline void BLASaxpy(int n, std::complex<double> *a, std::complex<double> *x,
                     int incx, std::complex<double> *y, int incy) {
  return cblas_zaxpy(n, a, x, incx, y, incy);
}
inline void BLASscal(int n, std::complex<double> *a, std::complex<double> *x,
                     int incx) {
  return cblas_zscal(n, a, x, incx);
}

// Compute C := alpha*A*A**std::complex<double> + beta*C or C :=
// alpha*A**std::complex<double>*A + beta*C
inline void BLASsyrk(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
                     const enum CBLAS_TRANSPOSE Trans, int n, int k,
                     std::complex<double> *alpha, std::complex<double> *a,
                     int lda, std::complex<double> *beta,
                     std::complex<double> *c, int ldc) {
  return cblas_zsyrk(Order, Uplo, Trans, n, k, alpha, a, lda, beta, c, ldc);
}

// Solve A*x = b or A^std::complex<double>*x = b where A is in packed format
inline void BLAStpsv(const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
                     const enum CBLAS_TRANSPOSE TransA,
                     const enum CBLAS_DIAG Diag, int n, std::complex<double> *a,
                     std::complex<double> *x, int incx) {
  return cblas_ztpsv(order, Uplo, TransA, Diag, n, a, x, incx);
}

// Level 2 BLAS routines
// y = alpha * A * x + beta * y, for a general matrix
inline void BLASgemv(const enum CBLAS_ORDER order,
                     const enum CBLAS_TRANSPOSE TransA, int m, int n,
                     std::complex<double> *alpha, std::complex<double> *a,
                     int lda, std::complex<double> *x, int incx,
                     std::complex<double> *beta, std::complex<double> *y,
                     int incy) {
  return cblas_zgemv(order, TransA, m, n, alpha, a, lda, x, incx, beta, y,
                     incy);
}

// Level 3 BLAS routines
// C := alpha*op( A )*op( B ) + beta*C,
inline void BLASgemm(const enum CBLAS_ORDER Order,
                     const enum CBLAS_TRANSPOSE TransA,
                     const enum CBLAS_TRANSPOSE TransB, int m, int n, int k,
                     std::complex<double> *alpha, std::complex<double> *a,
                     int lda, std::complex<double> *b, int ldb,
                     std::complex<double> *beta, std::complex<double> *c,
                     int ldc) {
  return cblas_zgemm(Order, TransA, TransB, m, n, k, alpha, a, lda, b, ldb,
                     beta, c, ldc);
}

// General factorization routines
inline void LAPACKgetrf(int *m, int *n, std::complex<double> *a, int *lda,
                        int *ipiv, int *info) {
  return zgetrf_(m, n, a, lda, ipiv, info);
}
inline void LAPACKgetrs(const char *c, int *n, int *nrhs,
                        std::complex<double> *a, int *lda, int *ipiv,
                        std::complex<double> *b, int *ldb, int *info) {
  return zgetrs_(c, n, nrhs, a, lda, ipiv, b, ldb, info);
}
inline void LAPACKgetri(int *n, std::complex<double> *a, int *lda, int *ipiv,
                        std::complex<double> *work, int *lwork, int *info) {
  return zgetri_(n, a, lda, ipiv, work, lwork, info);
}

// Factorization of packed-storage matrices
inline void LAPACKpptrf(const char *c, int *n, std::complex<double> *ap,
                        int *info) {
  return zpptrf_(c, n, ap, info);
}
inline void LAPACKpptrs(const char *c, int *n, int *nrhs,
                        std::complex<double> *ap, std::complex<double> *rhs,
                        int *ldrhs, int *info) {
  return zpptrs_(c, n, nrhs, ap, rhs, ldrhs, info);
}

// Solve a real eigenvalue problem
inline void LAPACKdspev(const char *job, const char *uplo, int *n, double *a,
                        double *w, double *z, int *ldz, double *work,
                        int *info) {
  dspev_(job, uplo, n, a, w, z, ldz, work, info);
}

}  // namespace SparseUtils

#endif  // SPARSE_UTILS_LAPACK_H