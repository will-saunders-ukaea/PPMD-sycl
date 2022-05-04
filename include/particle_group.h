#ifndef _PPMD_PARTICLE_GROUP
#define _PPMD_PARTICLE_GROUP

#include <map>
#include <string>

class ParticleGroup {
  private:

  public:
    Domain domain;
    ComputeTarget compute_target;
    std::map<std::string, ParticleDatBase> particle_dats{};

    ParticleGroup(
        Domain domain, 
        ComputeTarget compute_target
    ):
    domain(domain), compute_target(compute_target)
    {}
    ~ParticleGroup(){}
    
    template<typename T>
    void add_particle_dat(std::string name, ParticleDat<T> particle_dat);


};


template<typename T>
void ParticleGroup::add_particle_dat(
    std::string name,
    ParticleDat<T> particle_dat
){
    
    this->particle_dats[name] = particle_dat;

}


#endif
