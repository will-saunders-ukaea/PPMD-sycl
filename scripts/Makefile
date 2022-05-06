
MPI_CFLAGS:= -Wl,-Bsymbolic-functions -Wl,-z,relro -I/usr/include/x86_64-linux-gnu/mpich -L/usr/lib/x86_64-linux-gnu -lmpich

HIPSYCL:=syclcc --hipsycl-targets=omp
DPCPP:=dpcpp


SYCL:=$(HIPSYCL)
CFLAGS:=-I../include $(MPI_CFLAGS)

main:
	$(SYCL) main.cpp $(CFLAGS) -o main

main_dpcpp:
	$(DPCPP) main.cpp $(CFLAGS) -o main

main_hipsycl:
	$(HIPSYCL) main.cpp $(CFLAGS) -o main
