

MPI_CFLAGS:= -Wl,-Bsymbolic-functions -Wl,-z,relro -I/usr/include/x86_64-linux-gnu/mpich -L/usr/lib/x86_64-linux-gnu -lmpich
LIBS:=
CFLAGS = -Wall -Wno-unused-function -g -I../src -DDEBUGGING
CC:= mpic++

HIPSYCL:=syclcc --hipsycl-targets=omp
DPCPP:=dpcpp


SYCL:=$(HIPSYCL)
CFLAGS:=-I../include $(MPI_CFLAGS)

# Avoid making main.o so we can link the test objects against *.o in this
# directory.
TEST_SRCS:=$(wildcard *.cpp)
TEST_OBJS:=$(TEST_SRCS:.cpp=.o)

HEADERS:=$(wildcard ../include/*.h) $(wildcard ./*.h)

.PRECIOUS: %.o

test: test_runner
	./test_runner

test_runner: $(TEST_OBJS)
	$(CC) -o $@ $(TEST_OBJS)  $(CFLAGS)  $(LIBS)

%.o: %.cpp %.h $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cpp $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm *.o test_runner
