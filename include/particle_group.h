#ifndef _PPMD_PARTICLE_GROUP
#define _PPMD_PARTICLE_GROUP


class ParticleGroup {
  private:

  public:
    Domain domain;
    ComputeTarget compute_target;

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


}


#endif
