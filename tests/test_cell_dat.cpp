#include <CL/sycl.hpp>
#include <catch2/catch.hpp>
#include <ppmd.h>
using namespace PPMD;

TEST_CASE("test_cell_dat_const_1") {

    SYCLTarget sycl_target{GPU_SELECTOR, MPI_COMM_WORLD};

    const int cell_count = 4;
    const int nrow = 3;
    const int ncol = 5;
    Mesh mesh(cell_count);
    Domain domain(mesh);

    CellDatConst<PPMD::INT> cdc(sycl_target, mesh, nrow, ncol);

    REQUIRE(cdc.nrow == nrow);
    REQUIRE(cdc.ncol == ncol);

    // should have malloced a cell_count * nrow * ncol sized block
    std::vector<PPMD::INT> test_data(cell_count * nrow * ncol);

    // create some data and manually place in dat
    const int stride = nrow * ncol;
    int index = 0;
    for (int cellx = 0; cellx < cell_count; cellx++) {
        for (int rowx = 0; rowx < nrow; rowx++) {
            for (int colx = 0; colx < ncol; colx++) {
                test_data[cellx * stride + colx * nrow + rowx] = ++index;
            }
        }
    }
    sycl_target.queue
        .memcpy(cdc.device_ptr(), test_data.data(),
                test_data.size() * sizeof(PPMD::INT))
        .wait();

    index = 0;
    for (int cellx = 0; cellx < cell_count; cellx++) {
        CellData<PPMD::INT> cell_data = cdc.get_cell(cellx);
        for (int rowx = 0; rowx < nrow; rowx++) {
            for (int colx = 0; colx < ncol; colx++) {
                // check data returned from dat using user interface
                REQUIRE(cell_data->data[colx][rowx] == ++index);
                // change the data
                cell_data->data[colx][rowx] = 2 * cell_data->data[colx][rowx];
            }
        }
        // write the changed data back to the celldat through the user interface
        cdc.set_cell(cellx, cell_data);
    }

    // check celldat data is correct
    sycl_target.queue
        .memcpy(test_data.data(), cdc.device_ptr(),
                test_data.size() * sizeof(PPMD::INT))
        .wait();
    index = 0;
    for (int cellx = 0; cellx < cell_count; cellx++) {
        for (int rowx = 0; rowx < nrow; rowx++) {
            for (int colx = 0; colx < ncol; colx++) {
                REQUIRE(test_data[cellx * stride + colx * nrow + rowx] ==
                        2 * (++index));
            }
        }
    }
}
