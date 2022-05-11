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

        auto d = this->sycl_target.queue.get_device();

        this->npart_local = 0;
        this->npart_alloc = 0;
        this->d_ptr = NULL;
    }
    ~ParticleDatT() {
        if (this->d_ptr != NULL) {
            sycl::free(this->d_ptr, this->sycl_target.queue);
        }
        this->d_ptr = NULL;
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

template <typename T>
void ParticleDatT<T>::append_particle_data(const int npart_new,
                                           const bool new_data_exists,
                                           std::vector<PPMD::INT> &cells,
                                           std::vector<T> &data) {

    if (new_data_exists) {
        PPMDASSERT(data.size() >= npart_new * this->ncomp,
                   "Source vector too small");

    } else {
    }

    this->sycl_target.queue.wait();
}

} // namespace PPMD

#endif
