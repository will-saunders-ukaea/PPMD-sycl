#ifndef _PPMD_COMPUTE_TARGET
#define _PPMD_COMPUTE_TARGET

#include <CL/sycl.hpp>
#include <mpi.h>

#include "typedefs.h"

using namespace cl;

namespace PPMD {

class SYCLTarget {
  private:
  public:
    sycl::device device;
    sycl::queue queue;
    MPI_Comm comm;

    SYCLTarget(){};
    SYCLTarget(const int gpu_device, MPI_Comm comm) {
        if (gpu_device > 0) {
            try {
                this->device = sycl::device(sycl::gpu_selector());
            } catch (sycl::exception const &e) {
                std::cout << "Cannot select a GPU\n" << e.what() << "\n";
                std::cout << "Using a CPU device\n";
                this->device = sycl::device(sycl::cpu_selector());
            }
        } else {
            this->device = sycl::device(sycl::cpu_selector());
        }

        std::cout << "Using "
                  << this->device.get_info<sycl::info::device::name>()
                  << std::endl;

        this->queue = sycl::queue(this->device);
        this->comm = comm;
    }
    ~SYCLTarget() {}
};

} // namespace PPMD

#endif
