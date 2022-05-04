#ifndef _PPMD_PARTICLE_DAT
#define _PPMD_PARTICLE_DAT

#include <memory>


template<typename T>
class ParticleDatT {
  private:
    T* d_ptr;
    int npart_local;
    int npart_alloc;

  public:
    const int ncomp;
    const bool positions;

    ComputeTarget compute_target;

    ParticleDatT(int ncomp, bool positions = false):
        ncomp(ncomp), positions(positions)
    {
        this->npart_alloc = 0;
    }
    ~ParticleDatT(){}

    void set_compute_target(ComputeTarget& compute_target){
        this->compute_target = compute_target;
    }
    void set_npart_local(const int npart_local){
        PPMDASSERT(npart_local >= 0, "npart_local is negative");
        
        if (npart_local > this->npart_alloc){
            this->d_ptr = sycl::malloc_device<T>(
                npart_local * this->ncomp, 
                this->compute_target->queue
            );
            this->npart_alloc = npart_local;
        }
        this->npart_local = npart_local;
    }
    int get_npart_local(const int npart_local){
        return this->npart_local;
    }

};


template <typename T>
using ParticleDatShPtr = std::shared_ptr<ParticleDatT<T>>;

template <typename T>
ParticleDatShPtr<T> ParticleDat(int ncomp, bool positions = false){
    return std::make_shared<ParticleDatT<T>>(ncomp, positions);
}



#endif