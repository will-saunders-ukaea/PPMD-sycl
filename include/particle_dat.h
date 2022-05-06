#ifndef _PPMD_PARTICLE_DAT
#define _PPMD_PARTICLE_DAT

#include <CL/sycl.hpp>
#include <memory>

#include "typedefs.h"

#include "access.h"
#include "compute_target.h"
#include "particle_set.h"
#include "particle_spec.h"

namespace PPMD {

template <typename T> class ParticleDatT {
  private:
    T *d_ptr;
    int npart_local;
    int npart_alloc;

  public:
    const PPMD::Sym<T> sym;
    const int ncomp;
    const bool positions;
    const std::string name;

    ComputeTarget compute_target;

    ParticleDatT(const Sym<T> sym, int ncomp, bool positions = false)
        : sym(sym), name(sym.name), ncomp(ncomp), positions(positions) {
        this->npart_alloc = 0;
    }
    ~ParticleDatT() {}

    void set_compute_target(ComputeTarget &compute_target) {
        this->compute_target = compute_target;
    }
    void set_npart_local(const int npart_local) {
        PPMDASSERT(npart_local >= 0, "npart_local is negative");

        if (npart_local > this->npart_alloc) {
            this->d_ptr = sycl::malloc_device<T>(npart_local * this->ncomp,
                                                 this->compute_target->queue);
            this->npart_alloc = npart_local;
        }
        this->npart_local = npart_local;
    }
    int get_npart_local(const int npart_local) { return this->npart_local; }
    void append_particle_data(std::vector<T> &data);
};

template <typename T> using ParticleDatShPtr = std::shared_ptr<ParticleDatT<T>>;

template <typename T>
ParticleDatShPtr<T> ParticleDat(const PPMD::Sym<T> sym, int ncomp,
                                bool positions = false) {
    return std::make_shared<ParticleDatT<T>>(sym.name, ncomp, positions);
}
template <typename T> ParticleDatShPtr<T> ParticleDat(ParticleProp<T> prop) {
    return std::make_shared<ParticleDatT<T>>(prop.sym, prop.ncomp,
                                             prop.positions);
}

template <typename T>
void ParticleDatT<T>::append_particle_data(std::vector<T> &data) {

    // Assume values are in column major format in data
    const int ncomp = data.size() / this->ncomp;

    std::cout << "ncomp: " << ncomp << std::endl;
}

} // namespace PPMD

#endif
