#ifndef _PPMD_PARTICLE_GROUP
#define _PPMD_PARTICLE_GROUP

#include <map>
#include <string>
#include <cstdint>
#include <mpi.h>

class ParticleGroup {
  private:

    template<typename T>
    void set_particle_dat_info(ParticleDatShPtr<T>& particle_dat){
        particle_dat->set_compute_target(this->compute_target);
    }

  public:
    Domain domain;
    ComputeTarget compute_target;

    std::map<std::string, ParticleDatShPtr<PPMD::REAL>> particle_dats_real{};
    std::map<std::string, ParticleDatShPtr<PPMD::INT>> particle_dats_int{};

    ParticleGroup(
        Domain domain, 
        ComputeTarget compute_target
    ):
    domain(domain), compute_target(compute_target)
    {}
    ~ParticleGroup(){}
    
    void add_particle_dat(std::string name, ParticleDatShPtr<PPMD::REAL> particle_dat);
    void add_particle_dat(std::string name, ParticleDatShPtr<PPMD::INT> particle_dat);

    void add_particles();
    template<typename U>
    void add_particles(U particle_data);

};


void ParticleGroup::add_particle_dat(
    std::string name,
    ParticleDatShPtr<PPMD::REAL> particle_dat
){
    this->particle_dats_real[name] = particle_dat;
    set_particle_dat_info(particle_dat);
}


void ParticleGroup::add_particle_dat(
    std::string name,
    ParticleDatShPtr<PPMD::INT> particle_dat
){
    this->particle_dats_int[name] = particle_dat;
    set_particle_dat_info(particle_dat);
}

void ParticleGroup::add_particles(){
};
template<typename U>
void ParticleGroup::add_particles(U particle_data){

};



#endif
