#ifndef _PPMD
#define _PPMD

#include <cstdint>
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl;
using namespace std;

namespace PPMD {

    #define PPMDASSERT(expr, msg) PPMD::ppmd_assert(#expr, expr, __FILE__, __LINE__, msg)

    void ppmd_assert(
        const char* expr_str, 
        bool expr, 
        const char* file, 
        int line, 
        const char* msg
    ){
    if (!expr)
    {
        std::cerr << "PPMD Assertion error:\t" << msg << "\n"
            << "Expected value:\t" << expr_str << "\n"
            << "Source location:\t\t" << file << ", line " << line << "\n";
        abort();
    }
}



    typedef double REAL;
    typedef int64_t INT;
    
    #include "domain.h"
    #include "compute_target.h"
    #include "particle_dat.h"
    #include "particle_group.h"

}

#endif
