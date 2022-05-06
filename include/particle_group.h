#ifndef _PPMD_PARTICLE_GROUP
#define _PPMD_PARTICLE_GROUP

#include <cstdint>
#include <map>
#include <mpi.h>
#include <string>

namespace PPMD {

class ParticleGroup {
  private:
    template <typename T>
    void set_particle_dat_info(ParticleDatShPtr<T> &particle_dat) {
        particle_dat->set_compute_target(this->compute_target);
    }

  public:
    Domain domain;
    ComputeTarget compute_target;

    std::map<PPMD::Sym<REAL>, ParticleDatShPtr<PPMD::REAL>>
        particle_dats_real{};
    std::map<PPMD::Sym<INT>, ParticleDatShPtr<PPMD::INT>> particle_dats_int{};

    ParticleGroup(Domain domain, ParticleSpec &particle_spec,
                  ComputeTarget compute_target)
        : domain(domain), compute_target(compute_target) {
        for (auto const &property : particle_spec.properties_real) {
            add_particle_dat(ParticleDat(property));
        }
        for (auto const &property : particle_spec.properties_int) {
            add_particle_dat(ParticleDat(property));
        }
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
    for (auto const &dat : this->particle_dats_real) {
        dat.second->append_particle_data(particle_data.get(dat.first));
    }
}

} // namespace PPMD

#endif
