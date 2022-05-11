
#include <ppmd.h>

using namespace PPMD;

int main(int argc, char **argv) {

    SYCLTarget sycl_target{GPU_SELECTOR, MPI_COMM_WORLD};

    const int cell_count = 5;
    Mesh mesh(cell_count);
    Domain domain(mesh);

    CellDatConst<PPMD::INT> c_occupancy(sycl_target, cell_count, 1, 1);
    CellDat<PPMD::REAL> c_particle_pos(sycl_target, cell_count, 2);

    ParticleSpec particle_spec{ParticleProp(Sym<PPMD::REAL>("P"), 2, true),
                               ParticleProp(Sym<PPMD::REAL>("V"), 3),
                               ParticleProp(Sym<PPMD::INT>("CELL_ID"), 1, true),
                               ParticleProp(Sym<PPMD::INT>("ID"), 1)};

    ParticleGroup A(domain, particle_spec, sycl_target);

    const int N = 10;
    ParticleSet initial_distribution(N, particle_spec);
    for (int px = 0; px < N; px++) {
        for (int dimx = 0; dimx < 2; dimx++) {
            initial_distribution[Sym<PPMD::REAL>("P")][px][dimx] =
                (double)((px * 2) + (dimx));
            std::cout << (double)((px * 2) + (dimx)) << std::endl;
        }
    }

    std::cout << "---------------" << std::endl;

    for (int px = 0; px < N; px++) {
        for (int dimx = 0; dimx < 2; dimx++) {
            std::cout << initial_distribution[Sym<PPMD::REAL>("P")][px][dimx]
                      << std::endl;
        }
    }

    std::cout << "---------------" << std::endl;

    A.add_particles_local(initial_distribution);

    return 0;
    // ParticleLoop(
    //    A[Sym<PPMD::REAL>("P")]->access(READ()),
    //    A[Sym<PPMD::REAL>("V")]->access(WRITE()),
    //    [](
    //        const int cell_index,
    //        const int particle index,
    //        ParticleDatAccess<PPMD::REAL> P,
    //        ParticleDatAccess<PPMD::INT> V
    //    ) {
    //            V[idx][0] = P[idx][0];
    //            V[idx][1] = P[idx][1];
    //            V[idx][2] = 3.0;
    //    }
    //)

    /*
    Accessor<PPMD::REAL> P = A[Sym<PPMD::REAL>("P")]->access(READ());
    Accessor<PPMD::REAL> V = A[Sym<PPMD::REAL>("V")]->access(WRITE());

                V[0][0] = P[0][0];
                V[0][1] = P[0][1];
                V[0][2] = 3.0;


    sycl_target.queue.submit(
        [&](sycl::handler& cgh) {

                sycl::stream out(1024, 256, cgh);
            cgh.parallel_for<class addkernel>(
                sycl::range<1>(A.get_npart_local()), [=](sycl::id<1> idx
            ) {

                V[idx][0] = P[idx][0];
                V[idx][1] = P[idx][1];
                V[idx][2] = 3.0;

            });


        }
    );

    sycl_target.queue.wait();

    */

    std::cout << "---------------" << std::endl;

    for (int px = 0; px < N; px++) {
        for (int dimx = 0; dimx < 2; dimx++) {
            std::cout << initial_distribution[Sym<PPMD::REAL>("P")][px][dimx]
                      << std::endl;
        }
    }

    std::cout << "---------------" << std::endl;

    for (int px = 0; px < A.get_npart_local(); px++) {
        std::cout << A[Sym<PPMD::REAL>("V")]->d_ptr[px] << std::endl;
    }

    return 0;
}
