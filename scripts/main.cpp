
#include <ppmd.h>

using namespace PPMD;

int main(int argc, char **argv){
    

    DummyTarget compute_target{};
    DummyDomain domain{};
    ParticleDat<double> pd(0);
    
    ParticleGroup A(domain, compute_target);
    
    A.add_particle_dat("P", ParticleDat<double>(2, true));
    A.add_particle_dat("V", ParticleDat<double>(3));
    A.add_particle_dat("Q", ParticleDat<double>(1));


    return 0;
}


