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
    Mesh &mesh;
    T *d_ptr;
    const int stride;

  public:
    SYCLTarget &sycl_target;
    const int nrow;
    const int ncol;
    ~CellDatConst() { sycl::free(this->d_ptr, sycl_target.queue); };
    CellDatConst(SYCLTarget &sycl_target, Mesh &mesh, const int nrow,
                 const int ncol)
        : sycl_target(sycl_target), mesh(mesh), nrow(nrow), ncol(ncol),
          stride(nrow * ncol) {
        const int cell_count = mesh.get_cell_count();
        this->d_ptr =
            sycl::malloc_device<T>(cell_count * nrow * ncol, sycl_target.queue);
        this->sycl_target.queue.fill(this->d_ptr, ((T)0),
                                     cell_count * nrow * ncol);
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
    Mesh &mesh;
    T ***d_ptr;
    std::vector<T **> h_ptr_cells;
    std::vector<T *> h_ptr_cols;
    std::vector<PPMD::INT> nrow_alloc;

  public:
    SYCLTarget &sycl_target;
    CellDatConst<PPMD::INT> &nrow;
    const int ncol;
    ~CellDat() {
        const int cell_count = mesh.get_cell_count();
        for (int cellx = 0; cellx < cell_count; cellx++) {
            sycl::free(this->h_ptr_cells[cellx], sycl_target.queue);
        }
        for (int colx = 0; colx < cell_count * this->ncol; colx++) {
            sycl::free(this->h_ptr_cols[colx], sycl_target.queue);
        }
        sycl::free(this->d_ptr, sycl_target.queue);
    };
    CellDat(SYCLTarget &sycl_target, Mesh &mesh, CellDatConst<PPMD::INT> &nrow,
            const int ncol)
        : sycl_target(sycl_target), mesh(mesh), nrow(nrow), ncol(ncol) {
        const int cell_count = mesh.get_cell_count();

        this->d_ptr = sycl::malloc_device<T **>(cell_count, sycl_target.queue);
        this->h_ptr_cells = std::vector<T **>(cell_count);
        this->h_ptr_cols = std::vector<T *>(cell_count * ncol);
        this->nrow_alloc = std::vector<PPMD::INT>(cell_count);

        for (int cellx = 0; cellx < cell_count; cellx++) {
            this->nrow_alloc[cellx] = 0;
            this->h_ptr_cells[cellx] =
                sycl::malloc_device<T *>(ncol, sycl_target.queue);
            for (int colx = 0; colx < ncol; colx++) {
                this->h_ptr_cols[cellx * ncol + colx] = NULL;
            }
        }
        sycl_target.queue.memcpy(d_ptr, this->h_ptr_cells.data(),
                                 cell_count * sizeof(T *));
        this->sycl_target.queue.wait();
    };

    void realloc() {

        const int cell_count = mesh.get_cell_count();
        for (int cellx = 0; cellx < cell_count; cellx++) {
            CellData<PPMD::INT> nrow_cell = this->nrow.get_cell(cellx);
            const int nrow_required = nrow_cell->data[0][0];
            const int nrow_existing = this->nrow_alloc[cellx];
            if (nrow_required > nrow_existing) {
                for (int colx = 0; colx < this->ncol; colx++) {
                    T *col_ptr_old =
                        this->h_ptr_cols[cellx * this->ncol + colx];
                    T *col_ptr_new = sycl::malloc_device<T>(nrow_required,
                                                            sycl_target.queue);
                    this->sycl_target.queue
                        .memcpy(col_ptr_new, col_ptr_old,
                                nrow_existing * sizeof(T))
                        .wait();
                    sycl::free(col_ptr_old, this->sycl_target.queue);
                    this->h_ptr_cols[cellx * this->ncol + colx] = col_ptr_new;
                }
                this->nrow_alloc[cellx] = nrow_required;
                sycl_target.queue.memcpy(this->h_ptr_cells[cellx],
                                         &this->h_ptr_cols[cellx * this->ncol],
                                         this->ncol * sizeof(T *));
            }
        }
        sycl_target.queue.wait();

        // TODO check above
    }
};

} // namespace PPMD

#endif
