
#include <ppmd.h>

using namespace PPMD;

int main(int argc, char **argv){

    SYCLTarget compute_target{0, MPI_COMM_WORLD};
    DummyDomain domain{};
    

    ParticleSpec particle_spec{
        ParticleProp(Sym<PPMD::REAL>("P"), 2, true),
        ParticleProp(Sym<PPMD::REAL>("V"), 3),
        ParticleProp(Sym<PPMD::INT>("ID"), 1)
    };
    
    ParticleGroup A(domain, particle_spec, compute_target);


    A.add_particle_dat(ParticleDat(Sym<PPMD::REAL>("Q"), 1));


    ParticleSet initial_distribution(10, particle_spec);


    initial_distribution[Sym<PPMD::REAL>("P")][0][0] = 1.0;
    std::cout << initial_distribution[Sym<PPMD::REAL>("P")][0][0] << std::endl;

    initial_distribution[Sym<PPMD::INT>("ID")][0][0] = 100;
    std::cout << initial_distribution[Sym<PPMD::INT>("ID")][0][0] << std::endl;

    A.add_particles_local(initial_distribution);


    Accessor<PPMD::REAL> P = A[Sym<PPMD::REAL>("P")]->access(READ());
    Accessor<PPMD::REAL> V = A[Sym<PPMD::REAL>("V")]->access(WRITE());


    compute_target.queue.submit(
        [&](sycl::handler& cgh) {

            cgh.parallel_for<class addkernel>(
                sycl::range<1>(A.get_npart_local()), [=](sycl::id<1> idx
            ) {
                
                V[idx] = P[idx];

            });


        }
    );

    compute_target.queue.wait();


    return 0;
}


