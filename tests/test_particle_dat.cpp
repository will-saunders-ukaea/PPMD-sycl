#include <CL/sycl.hpp>
#include <catch2/catch.hpp>
#include <ppmd.h>
using namespace PPMD;

TEST_CASE("test_particle_dat_1") {

    SYCLTarget sycl_target{GPU_SELECTOR, MPI_COMM_WORLD};

    const int cell_count = 4;
    const int ncomp = 3;

    auto A = ParticleDat(
        sycl_target, ParticleProp(Sym<PPMD::INT>("FOO"), ncomp), cell_count);

    std::vector<PPMD::INT> counts(cell_count);
    for (int cellx = 0; cellx < cell_count; cellx++) {
        REQUIRE(A->s_npart_cell[cellx] == 0);
        counts[cellx] = 0;
    }

    const int N = 42;

    std::vector<PPMD::INT> cells0(N);
    std::vector<PPMD::INT> data0(N * ncomp);

    PPMD::INT index = 0;
    for (int px = 0; px < N; px++) {
        cells0[px] = 1;
        counts[1]++;
        for (int cx = 0; cx < ncomp; cx++) {
            data0[cx * N + px] = ++index;
        }
    }

    A->realloc(counts);
    A->append_particle_data(N, true, cells0, data0);
}
