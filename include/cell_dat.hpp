#ifndef _PPMD_CELL_DAT
#define _PPMD_CELL_DAT

#include <CL/sycl.hpp>
#include <memory>
#include <vector>

#include "access.hpp"
#include "compute_target.hpp"
#include "domain.hpp"
#include "typedefs.hpp"

namespace PPMD {

/*
 * Container for the data within a single cell stored on the host. Data is
 * store column wise.
 */
template <typename T> class CellDataT {
  private:
  public:
    SYCLTarget &sycl_target;
    const int nrow;
    const int ncol;
    std::vector<std::vector<T>> data;
    inline CellDataT(SYCLTarget &sycl_target, const int nrow, const int ncol)
        : sycl_target(sycl_target), nrow(nrow), ncol(ncol) {
        this->data = std::vector<std::vector<T>>(ncol);
        for (int colx = 0; colx < ncol; colx++) {
            this->data[colx] = std::vector<T>(nrow);
        }
    }

    /*
     *  Subscript operator for cell data. Data should be indexed by column then
     * row. e.g. CellData cell_data; T value = *cell_data[col][row];
     */
    inline std::vector<T> &operator[](int col) { return this->data[col]; }
};

template <typename T> using CellData = std::shared_ptr<CellDataT<T>>;

/*
 *  Container that allocates on the device a matrix of fixed size nrow X ncol
 *  for N cells. Data stored in column major format. i.e. Data order from
 *  slowest to fastest is: cell, column, row.
 */
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

    /*
     * Helper function to index into the stored data. Note column major format.
     */
    inline int idx(const int cell, const int row, const int col) {
        return (this->stride * cell) + (this->nrow * col + row);
    };

    /*
     * Get the device pointer for the underlying data. Only accessible on the
     * device.
     */
    T *device_ptr() { return this->d_ptr; };

    /*
     * Get the data stored in a provided cell on the host as a CellData
     * instance.
     */
    inline CellData<T> get_cell(const int cell) {
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
    /*
     *  Set the data in a cell using a CellData instance.
     */
    inline void set_cell(const int cell, CellData<T> cell_data) {
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

/*
 * Store data on each cell where the number of columns required per cell is
 * constant but the number of rows is variable. Data is stored in a column
 * major manner with a new device pointer per column.
 */
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
    inline CellDat(SYCLTarget &sycl_target, const int ncells, const int ncol)
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

    /*
     * Set the number of rows required in a provided cell. This will realloc if
     * needed and copy the existing data into the new space. May not shrink the
     * array if the requested size is smaller than the existing size.
     */
    inline void set_nrow(const PPMD::INT cell, const PPMD::INT nrow_required) {
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

    /*
     * Get the contents of a provided cell on the host as a CellData instance.
     */
    inline CellData<T> get_cell(const int cell) {

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

    /*
     * Set the contents of a cell on the device using a CellData instance.
     */
    inline void set_cell(const int cell, CellData<T> cell_data) {
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

    /*
     * Get the root device pointer for the data storage. Data can be accessed
     * on the device in SYCL kernels with access like:
     *      d[cell_index][column_index][row_index]
     */
    T ***device_ptr() { return this->d_ptr; };
};

} // namespace PPMD

#endif
