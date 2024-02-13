#ifndef SPRASE_UTILS_UTILS_IMPL_H
#define SPRASE_UTILS_UTILS_IMPL_H

#include "sparse_utils/matrix_utils.h"

namespace SparseUtils {

// Compute y = alpha A * x + beta * y
template <typename T>
void CSRMatVec(double alpha, int nrows, const int *rowp, const int *cols,
               const T *Avals, const T *x, double beta, T *y) {
  if (alpha == 1.0 && beta == 0.0) {
    for (int i = 0; i < nrows; i++) {
      int jp_end = rowp[i + 1];
      T value = 0.0;
      for (int jp = rowp[i]; jp < jp_end; jp++) {
        int j = cols[jp];
        value += Avals[jp] * x[j];
      }
      y[i] = value;
    }
  } else if (alpha == -1.0 && beta == 0.0) {
    for (int i = 0; i < nrows; i++) {
      int jp_end = rowp[i + 1];
      T value = 0.0;
      for (int jp = rowp[i]; jp < jp_end; jp++) {
        int j = cols[jp];
        value += Avals[jp] * x[j];
      }
      y[i] = -value;
    }
  } else {
    if (beta == 0.0) {
      for (int i = 0; i < nrows; i++) {
        int jp_end = rowp[i + 1];
        T value = 0.0;
        for (int jp = rowp[i]; jp < jp_end; jp++) {
          int j = cols[jp];
          value += Avals[jp] * x[j];
        }
        y[i] = alpha * value;
      }
    } else {
      for (int i = 0; i < nrows; i++) {
        int jp_end = rowp[i + 1];
        T value = 0.0;
        for (int jp = rowp[i]; jp < jp_end; jp++) {
          int j = cols[jp];
          value += Avals[jp] * x[j];
        }
        y[i] = beta * y[i] + alpha * value;
      }
    }
  }
}

// Compute y = alpha A * x + beta * y
template <typename T>
void CSCMatVec(double alpha, int nrows, int ncols, const int *colp,
               const int *rows, const T *Avals, const T *x, double beta, T *y) {
  if (beta == 0.0) {
    for (int i = 0; i < nrows; i++) {
      y[i] = 0.0;
    }
  } else {
    for (int i = 0; i < nrows; i++) {
      y[i] = beta * y[i];
    }
  }

  if (alpha == 1.0) {
    for (int i = 0; i < ncols; i++) {
      int jp_end = colp[i + 1];
      T xi = x[i];
      for (int jp = colp[i]; jp < jp_end; jp++) {
        int j = rows[jp];
        y[j] += Avals[jp] * xi;
      }
    }
  } else if (alpha == -1.0) {
    for (int i = 0; i < ncols; i++) {
      int jp_end = colp[i + 1];
      T xi = x[i];
      for (int jp = colp[i]; jp < jp_end; jp++) {
        int j = rows[jp];
        y[j] -= Avals[jp] * xi;
      }
    }
  } else {
    for (int i = 0; i < ncols; i++) {
      int jp_end = colp[i + 1];
      T xi = alpha * x[i];
      for (int jp = colp[i]; jp < jp_end; jp++) {
        int j = rows[jp];
        y[j] += Avals[jp] * xi;
      }
    }
  }
}

template <typename T>
void SparseTranspose(int nrows, int ncols, const int *rowp, const int *cols,
                     const T *Avals, int *colp, int *rows, T *ATvals) {
  for (int j = 0; j < ncols + 1; j++) {
    colp[j] = 0;
  }

  for (int i = 0; i < nrows; i++) {
    int jp_end = rowp[i + 1];
    for (int jp = rowp[i]; jp < jp_end; jp++) {
      int j = cols[jp];
      colp[j + 1]++;
    }
  }

  // Set the colp array to be a pointer into each row
  for (int j = 0; j < ncols; j++) {
    colp[j + 1] += colp[j];
  }

  // Now, add the rows indices
  for (int i = 0; i < nrows; i++) {
    int jp_end = rowp[i + 1];
    for (int jp = rowp[i]; jp < jp_end; jp++) {
      int j = cols[jp];
      rows[colp[j]] = i;
      if (Avals) {
        ATvals[colp[j]] = Avals[jp];
      }
      colp[j]++;
    }
  }

  // Reset the colp array
  for (int j = ncols - 1; j >= 0; j--) {
    colp[j + 1] = colp[j];
  }
  colp[0] = 0;
}

// Compute the number of entries in the matrix product A * A^{T}
template <typename T>
void MatMatTransNumeric(int nrows, int ncols, const int *rowp, const int *cols,
                        const T *Avals, const int *colp, const int *rows,
                        const T *ATvals, const int *Bcolp, int *Brows, T *Bvals,
                        int *flag, T *tmp) {
  for (int i = 0; i < nrows; i++) {
    flag[i] = -1;
  }

  // P_{*j} = A_{*k} * A_{jk}
  for (int j = 0; j < nrows; j++) {
    int nz = 0;

    // Loop over the non-zero columns
    int kp_end = rowp[j + 1];
    for (int kp = rowp[j]; kp < kp_end; kp++) {
      T Ajk = Avals[kp];
      int k = cols[kp];

      // Add the non-zero pattern from column k
      int ip_end = colp[k + 1];
      for (int ip = colp[k]; ip < ip_end; ip++) {
        int i = rows[ip];

        if (flag[i] != j) {
          flag[i] = j;
          tmp[i] = ATvals[ip] * Ajk;
          Brows[Bcolp[j] + nz] = i;
          nz++;
        } else {
          tmp[i] += ATvals[ip] * Ajk;
        }
      }
    }

    // Copy the values from the temporary column
    for (int k = 0; k < nz; k++) {
      Bvals[Bcolp[j] + k] = tmp[Brows[Bcolp[j] + k]];
    }
  }
}

// Compute the matrix C + A * D * A^{T}
template <typename T>
void MatMatTransNumeric(int nrows, int ncols, const T *cvals, const int *rowp,
                        const int *cols, const T *Avals, const T *dvals,
                        const int *colp, const int *rows, const T *ATvals,
                        const int *Bcolp, int *Brows, T *Bvals, int *flag,
                        T *tmp) {
  for (int i = 0; i < nrows; i++) {
    flag[i] = -1;
  }

  // P_{*j} = A_{*k} * A_{jk}
  for (int j = 0; j < nrows; j++) {
    int nz = 0;

    // Loop over the non-zero columns
    int kp_end = rowp[j + 1];
    for (int kp = rowp[j]; kp < kp_end; kp++) {
      int k = cols[kp];
      T dAjk = dvals[k] * Avals[kp];

      // Add the non-zero pattern from column k
      int ip_end = colp[k + 1];
      for (int ip = colp[k]; ip < ip_end; ip++) {
        int i = rows[ip];

        if (flag[i] != j) {
          flag[i] = j;
          if (i == j) {
            tmp[i] = cvals[i] + ATvals[ip] * dAjk;
          } else {
            tmp[i] = ATvals[ip] * dAjk;
          }
          Brows[Bcolp[j] + nz] = i;
          nz++;
        } else {
          tmp[i] += ATvals[ip] * dAjk;
        }
      }
    }

    // Copy the values from the temporary column
    for (int k = 0; k < nz; k++) {
      Bvals[Bcolp[j] + k] = tmp[Brows[Bcolp[j] + k]];
    }
  }
}

// Compute the number of entries in the matrix product A * A^{T}
inline int MatMatTransSymbolic(int nrows, int ncols, const int *rowp,
                               const int *cols, const int *colp,
                               const int *rows, int *Bcolp, int *flag) {
  for (int i = 0; i < nrows; i++) {
    Bcolp[i] = 0;
    flag[i] = -1;
  }

  // P_{*j} = A_{*k} * A_{jk}
  for (int j = 0; j < nrows; j++) {
    int nz = 0;

    // Loop over the non-zero columns
    int kp_end = rowp[j + 1];
    for (int kp = rowp[j]; kp < kp_end; kp++) {
      int k = cols[kp];

      // Add the non-zero pattern from column k
      int ip_end = colp[k + 1];
      for (int ip = colp[k]; ip < ip_end; ip++) {
        int i = rows[ip];

        if (flag[i] != j) {
          flag[i] = j;
          nz++;
        }
      }
    }

    Bcolp[j] = nz;
  }

  int nnz = 0;
  for (int j = 0; j < nrows; j++) {
    int tmp = Bcolp[j];
    Bcolp[j] = nnz;
    nnz += tmp;
  }
  Bcolp[nrows] = nnz;

  return nnz;
}

/*
  Sort an array of length len, then remove duplicate entries and
  entries with values -1.
*/
inline int RemoveDuplicates(int *array, int len, int exclude) {
  std::sort(array, array + len);

  // Remove any negative numbers
  int i = 0;  // location to take entries from
  int j = 0;  // location to place entries

  while (i < len && array[i] < 0) i++;

  if (exclude >= 0) {
    for (; i < len; i++, j++) {
      while ((i < len - 1) && (array[i] == array[i + 1])) i++;

      if (array[i] == exclude) {
        j--;
      } else if (i != j) {
        array[j] = array[i];
      }
    }
  } else {
    for (; i < len; i++, j++) {
      while ((i < len - 1) && (array[i] == array[i + 1])) i++;

      if (i != j) {
        array[j] = array[i];
      }
    }
  }

  return j;  // The new length of the array
}

// Sort and make the data structure unique - remove diagonal
inline void SortAndRemoveDuplicates(int nvars, int *rowp, int *cols,
                                    int remove_diagonal) {
  int begin = rowp[0];
  for (int i = 0; i < nvars; i++) {
    int len = rowp[i + 1] - begin;
    int new_len = -1;
    if (remove_diagonal) {
      new_len = RemoveDuplicates(&cols[begin], len, i);
    } else {
      new_len = RemoveDuplicates(&cols[begin], len);
    }

    if (begin != rowp[i]) {
      for (int k = 0; k < new_len; k++) {
        cols[rowp[i] + k] = cols[begin + k];
      }
    }

    begin = rowp[i + 1];
    rowp[i + 1] = rowp[i] + new_len;
  }
}

// Convert BSRMat to an unblocked, CSR format
template <typename T, int M, int N>
CSRMat<T> *bsr_to_csr(BSRMat<T, M, N> *bsr_mat) {
  int nbrows = bsr_mat->nbrows;
  int nbcols = bsr_mat->nbcols;
  int nnz = bsr_mat->nnz;

  int nrows = nbrows * M;
  int ncols = nbcols * N;
  int gnnz = bsr_mat->nnz * M * N;

  CSRMat<T> *csr_mat = new CSRMat<T>(nrows, ncols, gnnz);

  int i;            // row index for this entry
  int n;            // entry index if entries are continuous in rows
  int nb;           // block index for this block
  int num_entries;  // number of entries in this row
  int MN = M * N;   // block size
  int index;        // aux entry index within the row

  for (int ib = 0; ib < nbrows; ib++) {
    num_entries = (bsr_mat->rowp[ib + 1] - bsr_mat->rowp[ib]) * N;
    for (int ii = 0; ii < M; ii++) {
      i = M * ib + ii;
      csr_mat->rowp[i] = MN * bsr_mat->rowp[ib] + ii * num_entries;
      for (nb = bsr_mat->rowp[ib], index = 0; nb < bsr_mat->rowp[ib + 1];
           nb++, index++) {
        for (int jj = 0; jj < N; jj++) {
          n = csr_mat->rowp[i] + index * N + jj;
          csr_mat->vals[n] = bsr_mat->vals[MN * nb + N * ii + jj];
          csr_mat->cols[n] = N * bsr_mat->cols[nb] + jj;
        }
      }
    }
  }
  csr_mat->rowp[nrows] = gnnz;

  return csr_mat;
}

// Convert BSRMat to an unblocked, CSC format
template <typename T, int M, int N>
CSCMat<T> *bsr_to_csc(BSRMat<T, M, N> *bsr_mat) {
  int nbrows = bsr_mat->nbrows;
  int nbcols = bsr_mat->nbcols;
  int nnz = bsr_mat->nnz;

  int nrows = nbrows * M;
  int ncols = nbcols * N;
  int gnnz = bsr_mat->nnz * M * N;

  CSCMat<T> *csc_mat = new CSCMat<T>(nrows, ncols, gnnz);
  for (int i = 0; i < ncols + 1; i++) {
    csc_mat->colp[i] = 0;
  }

  int i, j;  // global row, column index for this entry
  int n;     // entry index if entries are continuous in rows
  int nb;    // block index if blocks are continuous in rows
  int m;     // entry index if entries are continuous in columns

  for (nb = 0; nb < nnz; nb++) {
    for (int jj = 0; jj < N; jj++) {
      // Count entries for each column, need a prefix sum later
      csc_mat->colp[N * bsr_mat->cols[nb] + jj] += M;
    }
  }

  // Perform the prefix sum across columns to get real colp
  int presum = 0;  // prefix sum, number of entries before this column
  int temp;
  for (int jb = 0; jb < nbcols; jb++) {
    for (int jj = 0; jj < N; jj++) {
      j = N * jb + jj;
      temp = csc_mat->colp[j];
      csc_mat->colp[j] = presum;
      presum += temp;
    }
  }
  csc_mat->colp[ncols] = gnnz;

  // Now populate rows and vals, note that we use colp to track each column,
  // this means we need to set colp back later on
  for (int ib = 0; ib < nbrows; ib++) {
    for (int ii = 0; ii < M; ii++) {
      i = M * ib + ii;
      for (nb = bsr_mat->rowp[ib]; nb < bsr_mat->rowp[ib + 1]; nb++) {
        for (int jj = 0; jj < N; jj++) {
          j = N * bsr_mat->cols[nb] + jj;
          m = csc_mat->colp[j];

          csc_mat->rows[m] = i;
          csc_mat->vals[m] = bsr_mat->vals[M * N * nb + N * ii + jj];
          csc_mat->colp[j]++;
        }
      }
    }
  }

  // Now we reset colp
  int last = 0;
  for (int jb = 0; jb < nbcols; jb++) {
    for (int jj = 0; jj < N; jj++) {
      j = N * jb + jj;
      temp = csc_mat->colp[j];
      csc_mat->colp[j] = last;
      last = temp;
    }
  }

  return csc_mat;
}

}  // namespace SparseUtils

#endif  // SPRASE_UTILS_UTILS_IMPL_H