#ifndef SPRASE_UTILS_UTILS_IMPL_H
#define SPRASE_UTILS_UTILS_IMPL_H

#include "utils.h"

namespace SparseUtils {

// Compute y = alpha A * x + beta * y
template <typename T>
void CSRMatVec(double alpha, index_t nrows, const index_t *rowp,
               const index_t *cols, const T *Avals, const T *x, double beta,
               T *y) {
  if (alpha == 1.0 && beta == 0.0) {
    for (index_t i = 0; i < nrows; i++) {
      index_t jp_end = rowp[i + 1];
      T value = 0.0;
      for (index_t jp = rowp[i]; jp < jp_end; jp++) {
        index_t j = cols[jp];
        value += Avals[jp] * x[j];
      }
      y[i] = value;
    }
  } else if (alpha == -1.0 && beta == 0.0) {
    for (index_t i = 0; i < nrows; i++) {
      index_t jp_end = rowp[i + 1];
      T value = 0.0;
      for (index_t jp = rowp[i]; jp < jp_end; jp++) {
        index_t j = cols[jp];
        value += Avals[jp] * x[j];
      }
      y[i] = -value;
    }
  } else {
    if (beta == 0.0) {
      for (index_t i = 0; i < nrows; i++) {
        index_t jp_end = rowp[i + 1];
        T value = 0.0;
        for (index_t jp = rowp[i]; jp < jp_end; jp++) {
          index_t j = cols[jp];
          value += Avals[jp] * x[j];
        }
        y[i] = alpha * value;
      }
    } else {
      for (index_t i = 0; i < nrows; i++) {
        index_t jp_end = rowp[i + 1];
        T value = 0.0;
        for (index_t jp = rowp[i]; jp < jp_end; jp++) {
          index_t j = cols[jp];
          value += Avals[jp] * x[j];
        }
        y[i] = beta * y[i] + alpha * value;
      }
    }
  }
}

// Compute y = alpha A * x + beta * y
template <typename T>
void CSCMatVec(double alpha, index_t nrows, index_t ncols, const index_t *colp,
               const index_t *rows, const T *Avals, const T *x, double beta,
               T *y) {
  if (beta == 0.0) {
    for (index_t i = 0; i < nrows; i++) {
      y[i] = 0.0;
    }
  } else {
    for (index_t i = 0; i < nrows; i++) {
      y[i] = beta * y[i];
    }
  }

  if (alpha == 1.0) {
    for (index_t i = 0; i < ncols; i++) {
      index_t jp_end = colp[i + 1];
      T xi = x[i];
      for (index_t jp = colp[i]; jp < jp_end; jp++) {
        index_t j = rows[jp];
        y[j] += Avals[jp] * xi;
      }
    }
  } else if (alpha == -1.0) {
    for (index_t i = 0; i < ncols; i++) {
      index_t jp_end = colp[i + 1];
      T xi = x[i];
      for (index_t jp = colp[i]; jp < jp_end; jp++) {
        index_t j = rows[jp];
        y[j] -= Avals[jp] * xi;
      }
    }
  } else {
    for (index_t i = 0; i < ncols; i++) {
      index_t jp_end = colp[i + 1];
      T xi = alpha * x[i];
      for (index_t jp = colp[i]; jp < jp_end; jp++) {
        index_t j = rows[jp];
        y[j] += Avals[jp] * xi;
      }
    }
  }
}

template <typename T>
void SparseTranspose(index_t nrows, index_t ncols, const index_t *rowp,
                     const index_t *cols, const T *Avals, index_t *colp,
                     index_t *rows, T *ATvals) {
  for (index_t j = 0; j < ncols + 1; j++) {
    colp[j] = 0;
  }

  for (index_t i = 0; i < nrows; i++) {
    index_t jp_end = rowp[i + 1];
    for (index_t jp = rowp[i]; jp < jp_end; jp++) {
      index_t j = cols[jp];
      colp[j + 1]++;
    }
  }

  // Set the colp array to be a pointer into each row
  for (index_t j = 0; j < ncols; j++) {
    colp[j + 1] += colp[j];
  }

  // Now, add the rows indices
  for (index_t i = 0; i < nrows; i++) {
    index_t jp_end = rowp[i + 1];
    for (index_t jp = rowp[i]; jp < jp_end; jp++) {
      index_t j = cols[jp];
      rows[colp[j]] = i;
      if (Avals) {
        ATvals[colp[j]] = Avals[jp];
      }
      colp[j]++;
    }
  }

  // Reset the colp array
  for (index_t j = ncols - 1; j >= 0; j--) {
    colp[j + 1] = colp[j];
  }
  colp[0] = 0;
}

// Compute the number of entries in the matrix product A * A^{T}
template <typename T>
void MatMatTransNumeric(index_t nrows, index_t ncols, const index_t *rowp,
                        const index_t *cols, const T *Avals,
                        const index_t *colp, const index_t *rows,
                        const T *ATvals, const index_t *Bcolp, index_t *Brows,
                        T *Bvals, index_t *flag, T *tmp) {
  for (index_t i = 0; i < nrows; i++) {
    flag[i] = -1;
  }

  // P_{*j} = A_{*k} * A_{jk}
  for (index_t j = 0; j < nrows; j++) {
    index_t nz = 0;

    // Loop over the non-zero columns
    index_t kp_end = rowp[j + 1];
    for (index_t kp = rowp[j]; kp < kp_end; kp++) {
      T Ajk = Avals[kp];
      index_t k = cols[kp];

      // Add the non-zero pattern from column k
      index_t ip_end = colp[k + 1];
      for (index_t ip = colp[k]; ip < ip_end; ip++) {
        index_t i = rows[ip];

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
    for (index_t k = 0; k < nz; k++) {
      Bvals[Bcolp[j] + k] = tmp[Brows[Bcolp[j] + k]];
    }
  }
}

// Compute the matrix C + A * D * A^{T}
template <typename T>
void MatMatTransNumeric(index_t nrows, index_t ncols, const T *cvals,
                        const index_t *rowp, const index_t *cols,
                        const T *Avals, const T *dvals, const index_t *colp,
                        const index_t *rows, const T *ATvals,
                        const index_t *Bcolp, index_t *Brows, T *Bvals,
                        index_t *flag, T *tmp) {
  for (index_t i = 0; i < nrows; i++) {
    flag[i] = -1;
  }

  // P_{*j} = A_{*k} * A_{jk}
  for (index_t j = 0; j < nrows; j++) {
    index_t nz = 0;

    // Loop over the non-zero columns
    index_t kp_end = rowp[j + 1];
    for (index_t kp = rowp[j]; kp < kp_end; kp++) {
      index_t k = cols[kp];
      T dAjk = dvals[k] * Avals[kp];

      // Add the non-zero pattern from column k
      index_t ip_end = colp[k + 1];
      for (index_t ip = colp[k]; ip < ip_end; ip++) {
        index_t i = rows[ip];

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
    for (index_t k = 0; k < nz; k++) {
      Bvals[Bcolp[j] + k] = tmp[Brows[Bcolp[j] + k]];
    }
  }
}

// Compute the number of entries in the matrix product A * A^{T}
inline index_t MatMatTransSymbolic(index_t nrows, index_t ncols,
                                   const index_t *rowp, const index_t *cols,
                                   const index_t *colp, const index_t *rows,
                                   index_t *Bcolp, index_t *flag) {
  for (index_t i = 0; i < nrows; i++) {
    Bcolp[i] = 0;
    flag[i] = -1;
  }

  // P_{*j} = A_{*k} * A_{jk}
  for (index_t j = 0; j < nrows; j++) {
    index_t nz = 0;

    // Loop over the non-zero columns
    index_t kp_end = rowp[j + 1];
    for (index_t kp = rowp[j]; kp < kp_end; kp++) {
      index_t k = cols[kp];

      // Add the non-zero pattern from column k
      index_t ip_end = colp[k + 1];
      for (index_t ip = colp[k]; ip < ip_end; ip++) {
        index_t i = rows[ip];

        if (flag[i] != j) {
          flag[i] = j;
          nz++;
        }
      }
    }

    Bcolp[j] = nz;
  }

  index_t nnz = 0;
  for (index_t j = 0; j < nrows; j++) {
    index_t tmp = Bcolp[j];
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
inline index_t RemoveDuplicates(index_t *array, index_t len, index_t exclude) {
  std::sort(array, array + len);

  // Remove any negative numbers
  index_t i = 0;  // location to take entries from
  index_t j = 0;  // location to place entries

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
inline void SortAndRemoveDuplicates(index_t nvars, index_t *rowp, index_t *cols,
                                    index_t remove_diagonal) {
  index_t begin = rowp[0];
  for (index_t i = 0; i < nvars; i++) {
    index_t len = rowp[i + 1] - begin;
    index_t new_len = -1;
    if (remove_diagonal) {
      new_len = RemoveDuplicates(&cols[begin], len, i);
    } else {
      new_len = RemoveDuplicates(&cols[begin], len);
    }

    if (begin != rowp[i]) {
      for (index_t k = 0; k < new_len; k++) {
        cols[rowp[i] + k] = cols[begin + k];
      }
    }

    begin = rowp[i + 1];
    rowp[i + 1] = rowp[i] + new_len;
  }
}

// Convert BSRMat to an unblocked, CSR format
template <typename T, index_t M, index_t N>
CSRMat<T> *bsr_to_csr(BSRMat<T, M, N> *bsr_mat) {
  index_t nbrows = bsr_mat->nbrows;
  index_t nbcols = bsr_mat->nbcols;
  index_t nnz = bsr_mat->nnz;

  index_t nrows = nbrows * M;
  index_t ncols = nbcols * N;
  index_t gnnz = bsr_mat->nnz * M * N;

  CSRMat<T> *csr_mat = new CSRMat<T>(nrows, ncols, gnnz);

  index_t i;            // row index for this entry
  index_t n;            // entry index if entries are continuous in rows
  index_t nb;           // block index for this block
  index_t num_entries;  // number of entries in this row
  index_t MN = M * N;   // block size
  index_t index;        // aux entry index within the row

  for (index_t ib = 0; ib < nbrows; ib++) {
    num_entries = (bsr_mat->rowp[ib + 1] - bsr_mat->rowp[ib]) * N;
    for (index_t ii = 0; ii < M; ii++) {
      i = M * ib + ii;
      csr_mat->rowp[i] = MN * bsr_mat->rowp[ib] + ii * num_entries;
      for (nb = bsr_mat->rowp[ib], index = 0; nb < bsr_mat->rowp[ib + 1];
           nb++, index++) {
        for (index_t jj = 0; jj < N; jj++) {
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
template <typename T, index_t M, index_t N>
CSCMat<T> *bsr_to_csc(BSRMat<T, M, N> *bsr_mat) {
  index_t nbrows = bsr_mat->nbrows;
  index_t nbcols = bsr_mat->nbcols;
  index_t nnz = bsr_mat->nnz;

  index_t nrows = nbrows * M;
  index_t ncols = nbcols * N;
  index_t gnnz = bsr_mat->nnz * M * N;

  CSCMat<T> *csc_mat = new CSCMat<T>(nrows, ncols, gnnz);
  for (index_t i = 0; i < ncols + 1; i++) {
    csc_mat->colp[i] = 0;
  }

  index_t i, j;  // global row, column index for this entry
  index_t n;     // entry index if entries are continuous in rows
  index_t nb;    // block index if blocks are continuous in rows
  index_t m;     // entry index if entries are continuous in columns

  for (nb = 0; nb < nnz; nb++) {
    for (index_t jj = 0; jj < N; jj++) {
      // Count entries for each column, need a prefix sum later
      csc_mat->colp[N * bsr_mat->cols[nb] + jj] += M;
    }
  }

  // Perform the prefix sum across columns to get real colp
  index_t presum = 0;  // prefix sum, number of entries before this column
  index_t temp;
  for (index_t jb = 0; jb < nbcols; jb++) {
    for (index_t jj = 0; jj < N; jj++) {
      j = N * jb + jj;
      temp = csc_mat->colp[j];
      csc_mat->colp[j] = presum;
      presum += temp;
    }
  }
  csc_mat->colp[ncols] = gnnz;

  // Now populate rows and vals, note that we use colp to track each column,
  // this means we need to set colp back later on
  for (index_t ib = 0; ib < nbrows; ib++) {
    for (index_t ii = 0; ii < M; ii++) {
      i = M * ib + ii;
      for (nb = bsr_mat->rowp[ib]; nb < bsr_mat->rowp[ib + 1]; nb++) {
        for (index_t jj = 0; jj < N; jj++) {
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
  index_t last = 0;
  for (index_t jb = 0; jb < nbcols; jb++) {
    for (index_t jj = 0; jj < N; jj++) {
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