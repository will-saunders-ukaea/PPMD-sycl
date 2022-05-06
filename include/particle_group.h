#ifndef _PPMD_PARTICLE_GROUP
#define _PPMD_PARTICLE_GROUP

#include <cstdint>
#include <map>
#include <mpi.h>
#include <string>

#include "access.h"
#include "compute_target.h"
#include "domain.h"
#include "particle_dat.h"
#include "particle_set.h"
#include "particle_spec.h"
#include "typedefs.h"

namespace PPMD {

class ParticleGroup {
  private:
    template <typename T>
    void set_particle_dat_info(ParticleDatShPtr<T> &particle_dat) {
        particle_dat->set_compute_target(this->sycl_target);
    }
    int npart_local;

  public:
    Domain domain;
    SYCLTarget sycl_target;

    std::map<PPMD::Sym<REAL>, ParticleDatShPtr<PPMD::REAL>>
        particle_dats_real{};
    std::map<PPMD::Sym<INT>, ParticleDatShPtr<PPMD::INT>> particle_dats_int{};

    ParticleGroup(Domain domain, ParticleSpec &particle_spec,
                  SYCLTarget sycl_target)
        : domain(domain), sycl_target(sycl_target) {
        for (auto const &property : particle_spec.properties_real) {
            add_particle_dat(ParticleDat(property));
        }
        for (auto const &property : particle_spec.properties_int) {
            add_particle_dat(ParticleDat(property));
        }
        this->npart_local = 0;
    }
    ~ParticleGroup() {}

    void add_particle_dat(ParticleDatShPtr<PPMD::REAL> particle_dat);
    void add_particle_dat(ParticleDatShPtr<PPMD::INT> particle_dat);

    void add_particles();
    template <typename U> void add_particles(U particle_data);
    void add_particles_local(ParticleSet &particle_data);
};

void ParticleGroup::add_particle_dat(
    ParticleDatShPtr<PPMD::REAL> particle_dat) {
    this->particle_dats_real[particle_dat->sym] = particle_dat;
    set_particle_dat_info(particle_dat);
}
void ParticleGroup::add_particle_dat(ParticleDatShPtr<PPMD::INT> particle_dat) {
    this->particle_dats_int[particle_dat->sym] = particle_dat;
    set_particle_dat_info(particle_dat);
}

void ParticleGroup::add_particles(){};
template <typename U>
void ParticleGroup::add_particles(U particle_data){

};

void ParticleGroup::add_particles_local(ParticleSet &particle_data) {
    const int npart = particle_data.npart;
    const int npart_new = this->npart_local + npart;

    for (auto const &dat : this->particle_dats_real) {
        dat.second->append_particle_data(npart,
                                         particle_data.contains(dat.first),
                                         particle_data.get(dat.first));
        PPMDASSERT(dat.second->get_npart_local() == npart_new,
                   "Appending particles failed.");
    }
    for (auto const &dat : this->particle_dats_int) {
        dat.second->append_particle_data(npart,
                                         particle_data.contains(dat.first),
                                         particle_data.get(dat.first));
        PPMDASSERT(dat.second->get_npart_local() == npart_new,
                   "Appending particles failed.");
    }
    this->npart_local = npart_new;
}

} // namespace PPMD

#endif
