
#include <ppmd.h>

using namespace PPMD;

int main(int argc, char **argv){

    SYCLTarget compute_target{0, MPI_COMM_WORLD};
    DummyDomain domain{};
    
    ParticleGroup A(domain, compute_target);
    
    A.add_particle_dat("P", ParticleDat<PPMD::REAL>(2, true));
    A.add_particle_dat("V", ParticleDat<PPMD::REAL>(3));
    A.add_particle_dat("Q", ParticleDat<PPMD::REAL>(1));
    A.add_particle_dat("ID", ParticleDat<PPMD::INT>(1));


    return 0;
}


