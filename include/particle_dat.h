#ifndef _PPMD_PARTICLE_DAT
#define _PPMD_PARTICLE_DAT

#include <CL/sycl.hpp>
#include <memory>

#include "access.h"
#include "compute_target.h"
#include "particle_set.h"
#include "particle_spec.h"
#include "typedefs.h"

namespace PPMD {

template <typename T> class ParticleDatT {
  private:
    int npart_local;
    int npart_alloc;

  public:
    // TODO make this pointer private
    T *d_ptr;
    int *s_npart_cell;
    const PPMD::Sym<T> sym;
    CellDat<T> cell_dat;
    const int ncomp;
    const int ncell;
    const bool positions;
    const std::string name;

    SYCLTarget &sycl_target;

    ParticleDatT(SYCLTarget &sycl_target, const Sym<T> sym, int ncomp,
                 int ncell, bool positions = false)
        : sycl_target(sycl_target), sym(sym), name(sym.name), ncomp(ncomp),
          ncell(ncell), positions(positions),
          cell_dat(CellDat<T>(sycl_target, ncell, ncomp)) {

        this->npart_local = 0;
        this->npart_alloc = 0;
        this->d_ptr = NULL;

        this->s_npart_cell =
            sycl::malloc_shared<int>(this->ncell, this->sycl_target.queue);
        for (int cellx = 0; cellx < this->ncell; cellx++) {
            this->s_npart_cell[cellx] = 0;
        }
    }
    ~ParticleDatT() {
        if (this->d_ptr != NULL) {
            sycl::free(this->d_ptr, this->sycl_target.queue);
        }
        this->d_ptr = NULL;
        sycl::free(this->s_npart_cell, this->sycl_target.queue);
    }

    void set_compute_target(SYCLTarget &sycl_target) {
        this->sycl_target = sycl_target;
    }
    void set_npart_local(const int npart_local) {
        PPMDASSERT(npart_local >= 0, "npart_local is negative");
        this->realloc(npart_local);
        this->npart_local = npart_local;
    }
    int get_npart_local(const int npart_local) { return this->npart_local; }
    void append_particle_data(const int npart_new, const bool new_data_exists,
                              std::vector<T> &data);
    void append_particle_data(const int npart_new, const bool new_data_exists,
                              std::vector<PPMD::INT> &cells,
                              std::vector<T> &data);
    void realloc(const int npart_new);
    void realloc(std::vector<PPMD::INT> &npart_cell_new);
    int get_npart_local() { return this->npart_local; }

    Accessor<T> access(AccessMode mode) {
        return Accessor<T>(this->d_ptr, mode, this->npart_alloc);
    };
};

template <typename T> using ParticleDatShPtr = std::shared_ptr<ParticleDatT<T>>;

template <typename T>
ParticleDatShPtr<T> ParticleDat(SYCLTarget &sycl_target, const PPMD::Sym<T> sym,
                                int ncomp, int ncell, bool positions = false) {
    return std::make_shared<ParticleDatT<T>>(sycl_target, sym, ncomp, ncell,
                                             positions);
}
template <typename T>
ParticleDatShPtr<T> ParticleDat(SYCLTarget &sycl_target, ParticleProp<T> prop,
                                int ncell) {
    return std::make_shared<ParticleDatT<T>>(sycl_target, prop.sym, prop.ncomp,
                                             ncell, prop.positions);
}

template <typename T> void ParticleDatT<T>::realloc(const int npart_new) {
    if (npart_new > npart_alloc) {
        T *d_ptr_new = sycl::malloc_device<T>(npart_new * this->ncomp,
                                              this->sycl_target.queue);

        for (int cx = 0; cx < this->ncomp; cx++) {
            this->sycl_target.queue.memcpy(&d_ptr_new[cx * npart_new],
                                           &d_ptr[cx * this->npart_local],
                                           this->npart_local * sizeof(T));
        }
        this->sycl_target.queue.wait();
        if (this->d_ptr != NULL) {
            sycl::free(this->d_ptr, this->sycl_target.queue);
        }
        this->d_ptr = d_ptr_new;
        this->npart_alloc = npart_new;
    }
}

template <typename T>
void ParticleDatT<T>::realloc(std::vector<PPMD::INT> &npart_cell_new) {
    PPMDASSERT(npart_cell_new.size() >= this->ncell,
               "Insufficent new cell counts");
    for (int cellx = 0; cellx < this->ncell; cellx++) {
        this->cell_dat.set_nrow(cellx, npart_cell_new[cellx]);
    }
}

template <typename T>
void ParticleDatT<T>::append_particle_data(const int npart_new,
                                           const bool new_data_exists,
                                           std::vector<T> &data) {

    this->realloc(npart_new + this->npart_local);
    if (new_data_exists) {
        PPMDASSERT(data.size() >= npart_new * this->ncomp,
                   "Source vector too small");
        for (int cx = 0; cx < this->ncomp; cx++) {
            this->sycl_target.queue.memcpy(&this->d_ptr[cx * this->npart_alloc],
                                           &data.data()[cx * npart_new],
                                           npart_new * sizeof(T));
        }
    } else {
        for (int cx = 0; cx < this->ncomp; cx++) {
            this->sycl_target.queue.fill(&this->d_ptr[cx * this->npart_alloc],
                                         ((T)0), npart_new);
        }
    }
    this->sycl_target.queue.wait();
    this->npart_local += npart_new;
}

/*
 *  Append particle data to the ParticleDat. wait() must be called on the queue
 *  before use of the data.
 *
 */
template <typename T>
void ParticleDatT<T>::append_particle_data(const int npart_new,
                                           const bool new_data_exists,
                                           std::vector<PPMD::INT> &cells,
                                           std::vector<T> &data) {

    PPMDASSERT(npart_new <= cells.size(), "incorrect number of cells");

    const size_t size_npart_new = static_cast<size_t>(npart_new);
    int *s_npart_cell = this->s_npart_cell;
    const int ncell = this->ncell;
    const int ncomp = this->ncomp;
    T ***d_cell_dat_ptr = this->cell_dat.device_ptr();

    sycl::buffer<PPMD::INT, 1> b_cells(cells.data(),
                                       sycl::range<1>{size_npart_new});

    if (new_data_exists) {
        sycl::buffer<T, 1> b_data(data.data(),
                                  sycl::range<1>{size_npart_new * this->ncomp});
        this->sycl_target.queue.submit([&](sycl::handler &cgh) {
            auto a_cells = b_cells.get_access<sycl::access::mode::read>(cgh);
            auto a_data =
                b_data.template get_access<sycl::access::mode::read>(cgh);

            cgh.parallel_for<class _ppmd_zero_append_data>(
                sycl::range<1>(npart_new), [=](sycl::id<1> idx) {
                    PPMD::INT cellx = a_cells[idx];
                    // atomically get the layer
                    sycl::atomic_ref<int, sycl::memory_order::relaxed,
                                     sycl::memory_scope::device>
                        element_atomic(s_npart_cell[cellx]);
                    const int layerx = element_atomic.fetch_add(1);

                    for (int cx = 0; cx < ncomp; cx++) {
                        d_cell_dat_ptr[cellx][cx][layerx] =
                            a_data[cx * npart_new + idx];
                    }
                });
        });
    } else {
        this->sycl_target.queue.submit([&](sycl::handler &cgh) {
            auto a_cells = b_cells.get_access<sycl::access::mode::read>(cgh);

            cgh.parallel_for<class _ppmd_zero_append_data>(
                sycl::range<1>(npart_new), [=](sycl::id<1> idx) {
                    PPMD::INT cellx = a_cells[idx];
                    // atomically get the layer
                    sycl::atomic_ref<int, sycl::memory_order::relaxed,
                                     sycl::memory_scope::device>
                        element_atomic(s_npart_cell[cellx]);
                    const int layerx = element_atomic.fetch_add(1);

                    for (int cx = 0; cx < ncomp; cx++) {
                        d_cell_dat_ptr[cellx][cx][layerx] = ((T)0);
                    }
                });
        });
    }
}

} // namespace PPMD

#endif
