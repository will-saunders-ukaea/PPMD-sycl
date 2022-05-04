SYCL:=syclcc --hipsycl-targets=omp
CFLAGS:=-I../include -Wl,-Bsymbolic-functions -Wl,-z,relro -I/usr/include/x86_64-linux-gnu/mpich -L/usr/lib/x86_64-linux-gnu -lmpich


main:
	$(SYCL) main.cpp $(CFLAGS) -o main



