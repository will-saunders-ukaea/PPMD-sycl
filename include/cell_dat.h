#ifndef _PPMD_CELL_DAT
#define _PPMD_CELL_DAT

#include <CL/sycl.hpp>
#include <memory>
#include <vector>

#include "access.h"
#include "compute_target.h"
#include "domain.h"
#include "typedefs.h"

namespace PPMD {

template <typename T> class CellDataT {
  private:
  public:
    SYCLTarget &sycl_target;
    const int nrow;
    const int ncol;
    std::vector<std::vector<T>> data;
    CellDataT(SYCLTarget &sycl_target, const int nrow, const int ncol)
        : sycl_target(sycl_target), nrow(nrow), ncol(ncol) {
        this->data = std::vector<std::vector<T>>(ncol);
        for (int colx = 0; colx < ncol; colx++) {
            this->data[colx] = std::vector<T>(nrow);
        }
    }

    std::vector<T> &operator[](int col) { return this->data[col]; }
};

template <typename T> using CellData = std::shared_ptr<CellDataT<T>>;

template <typename T> class CellDatConst {
  private:
    T *d_ptr;
    const int stride;

  public:
    SYCLTarget &sycl_target;
    const int ncells;
    const int nrow;
    const int ncol;
    ~CellDatConst() { sycl::free(this->d_ptr, sycl_target.queue); };
    CellDatConst(SYCLTarget &sycl_target, const int ncells, const int nrow,
                 const int ncol)
        : sycl_target(sycl_target), ncells(ncells), nrow(nrow), ncol(ncol),
          stride(nrow * ncol) {
        this->d_ptr =
            sycl::malloc_device<T>(ncells * nrow * ncol, sycl_target.queue);
        this->sycl_target.queue.fill(this->d_ptr, ((T)0), ncells * nrow * ncol);
        this->sycl_target.queue.wait();
    };
    inline int idx(const int cell, const int row, const int col) {
        return (this->stride * cell) + (this->nrow * col + row);
    };

    T *device_ptr() { return this->d_ptr; };

    CellData<T> get_cell(const int cell) {
        auto cell_data = std::make_shared<CellDataT<T>>(this->sycl_target,
                                                        this->nrow, this->ncol);
        for (int colx = 0; colx < this->ncol; colx++) {
            this->sycl_target.queue.memcpy(
                cell_data->data[colx].data(),
                &this->d_ptr[cell * this->stride + colx * this->nrow],
                this->nrow * sizeof(T));
        }
        this->sycl_target.queue.wait();
        return cell_data;
    }
    void set_cell(const int cell, CellData<T> cell_data) {
        PPMDASSERT(cell_data->nrow >= this->nrow,
                   "CellData as insuffient row count.");
        PPMDASSERT(cell_data->ncol >= this->ncol,
                   "CellData as insuffient column count.");

        for (int colx = 0; colx < this->ncol; colx++) {
            this->sycl_target.queue.memcpy(
                &this->d_ptr[cell * this->stride + colx * this->nrow],
                cell_data->data[colx].data(), this->nrow * sizeof(T));
        }
        this->sycl_target.queue.wait();
    }
};

template <typename T> class CellDat {
  private:
    T ***d_ptr;
    std::vector<T **> h_ptr_cells;
    std::vector<T *> h_ptr_cols;

  public:
    SYCLTarget &sycl_target;
    const int ncells;
    std::vector<PPMD::INT> nrow;
    const int ncol;
    std::vector<PPMD::INT> nrow_alloc;
    ~CellDat() {
        // issues on cuda backend w/o this NULL check.
        for (int cellx = 0; cellx < ncells; cellx++) {
            if ((this->nrow_alloc[cellx] != 0) &&
                (this->h_ptr_cells[cellx] != NULL)) {
                sycl::free(this->h_ptr_cells[cellx], sycl_target.queue);
            }
        }
        for (int colx = 0; colx < ncells * this->ncol; colx++) {
            if (this->h_ptr_cols[colx] != NULL) {
                sycl::free(this->h_ptr_cols[colx], sycl_target.queue);
            }
        }
        sycl::free(this->d_ptr, sycl_target.queue);
    };
    CellDat(SYCLTarget &sycl_target, const int ncells, const int ncol)
        : sycl_target(sycl_target), ncells(ncells), ncol(ncol) {

        this->nrow = std::vector<PPMD::INT>(ncells);
        this->d_ptr = sycl::malloc_device<T **>(ncells, sycl_target.queue);
        this->h_ptr_cells = std::vector<T **>(ncells);
        this->h_ptr_cols = std::vector<T *>(ncells * ncol);
        this->nrow_alloc = std::vector<PPMD::INT>(ncells);

        for (int cellx = 0; cellx < ncells; cellx++) {
            this->nrow_alloc[cellx] = 0;
            this->nrow[cellx] = 0;
            this->h_ptr_cells[cellx] =
                sycl::malloc_device<T *>(ncol, sycl_target.queue);
            for (int colx = 0; colx < ncol; colx++) {
                this->h_ptr_cols[cellx * ncol + colx] = NULL;
            }
        }

        sycl_target.queue.memcpy(d_ptr, this->h_ptr_cells.data(),
                                 ncells * sizeof(T *));

        this->sycl_target.queue.wait();
    };

    void set_nrow(const PPMD::INT cell, const PPMD::INT nrow_required) {
        PPMDASSERT(cell >= 0, "Cell index is negative");
        PPMDASSERT(cell < this->ncells, "Cell index is >= ncells");
        PPMDASSERT(nrow_required >= 0, "Requested number of rows is negative");
        const PPMD::INT nrow_alloced = this->nrow_alloc[cell];
        const PPMD::INT nrow_existing = this->nrow[cell];

        if (nrow_required != nrow_existing) {
            if (nrow_required > nrow_alloced) {
                for (int colx = 0; colx < this->ncol; colx++) {
                    T *col_ptr_old = this->h_ptr_cols[cell * this->ncol + colx];
                    T *col_ptr_new = sycl::malloc_device<T>(
                        nrow_required, this->sycl_target.queue);

                    if (nrow_alloced > 0) {
                        this->sycl_target.queue
                            .memcpy(col_ptr_new, col_ptr_old,
                                    nrow_existing * sizeof(T))
                            .wait();
                    }
                    if (col_ptr_old != NULL) {
                        sycl::free(col_ptr_old, this->sycl_target.queue);
                    }

                    this->h_ptr_cols[cell * this->ncol + colx] = col_ptr_new;
                }
                this->nrow_alloc[cell] = nrow_required;
                sycl_target.queue.memcpy(this->h_ptr_cells[cell],
                                         &this->h_ptr_cols[cell * this->ncol],
                                         this->ncol * sizeof(T *));

                sycl_target.queue.wait();
            }
            this->nrow[cell] = nrow_required;
        }
    }

    CellData<T> get_cell(const int cell) {

        auto cell_data = std::make_shared<CellDataT<T>>(
            this->sycl_target, this->nrow[cell], this->ncol);
        for (int colx = 0; colx < this->ncol; colx++) {
            this->sycl_target.queue.memcpy(
                cell_data->data[colx].data(),
                this->h_ptr_cols[cell * this->ncol + colx],
                this->nrow[cell] * sizeof(T));
        }
        this->sycl_target.queue.wait();
        return cell_data;
    }
    void set_cell(const int cell, CellData<T> cell_data) {
        PPMDASSERT(cell_data->nrow >= this->nrow[cell],
                   "CellData as insuffient row count.");
        PPMDASSERT(cell_data->ncol >= this->ncol,
                   "CellData as insuffient column count.");

        if (this->nrow[cell] > 0) {
            for (int colx = 0; colx < this->ncol; colx++) {

                this->sycl_target.queue.memcpy(
                    this->h_ptr_cols[cell * this->ncol + colx],
                    cell_data->data[colx].data(), this->nrow[cell] * sizeof(T));
            }
            this->sycl_target.queue.wait();
        }
    }
    T ***device_ptr() { return this->d_ptr; };
};

} // namespace PPMD

#endif
