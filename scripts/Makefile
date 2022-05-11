
MPI_CFLAGS : = -Wl, -Bsymbolic - functions - Wl, -z,
           relro - I / usr / include / x86_64 - linux - gnu / mpich -
               L / usr / lib / x86_64 - linux - gnu - lmpich -
               g

                   HIPSYCL : = syclcc-- hipsycl - targets =
                                   omp - DGPU_SELECTOR = 0 DPCPP
    : = dpcpp - DGPU_SELECTOR = 0 HIPSYCL_CUDA
    : = syclcc - DRESTRICT = __restrict --hipsycl - platform =
                                 cuda-- hipsycl - gpu - arch =
                                     sm_60-- cuda - path =
                                         $(CUDA_HOME) - DGPU_SELECTOR = 1

#SYCL : = $(HIPSYCL)
      SYCL = $(HIPSYCL_CUDA)

          CFLAGS : = -I../ include $(MPI_CFLAGS)

                               main : $(SYCL) main.cpp $(CFLAGS) -
                     o main

                         main_dpcpp : $(DPCPP) main.cpp $(CFLAGS) -
                     o main

                         main_hipsycl : $(HIPSYCL) main.cpp $(CFLAGS) -
                     o main
