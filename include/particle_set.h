#ifndef _PPMD_PARTICLE_SET
#define _PPMD_PARTICLE_SET

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "typedefs.h"
#include "access.h"
#include "compute_target.h"
#include "particle_spec.h"

namespace PPMD {

class ParticleSet {

  private:
    std::map<Sym<PPMD::REAL>, std::vector<PPMD::REAL>> values_real;
    std::map<Sym<PPMD::INT>, std::vector<PPMD::INT>> values_int;

  public:
    const int npart;

    ParticleSet(const int npart, ParticleSpec particle_spec) : npart(npart) {

        for (auto const &spec : particle_spec.properties_real) {
            values_real[spec.sym] = std::vector<PPMD::REAL>(npart * spec.ncomp);
            std::fill(values_real[spec.sym].begin(),
                      values_real[spec.sym].end(), 0.0);
        }
        for (auto const &spec : particle_spec.properties_int) {
            values_int[spec.sym] = std::vector<PPMD::INT>(npart * spec.ncomp);
            std::fill(values_int[spec.sym].begin(), values_int[spec.sym].end(),
                      0);
        }
    };

    ColumnMajorRowAccessor<std::vector, PPMD::REAL>
    operator[](Sym<PPMD::REAL> sym) {
        return ColumnMajorRowAccessor<std::vector, PPMD::REAL>{values_real[sym],
                                                               this->npart};
    };
    ColumnMajorRowAccessor<std::vector, PPMD::INT>
    operator[](Sym<PPMD::INT> sym) {
        return ColumnMajorRowAccessor<std::vector, PPMD::INT>{values_int[sym],
                                                              this->npart};
    };

    std::vector<PPMD::REAL> &get(Sym<PPMD::REAL> const &sym) {
        return values_real[sym];
    };
    std::vector<PPMD::INT> &get(Sym<PPMD::INT> const &sym) {
        return values_int[sym];
    };
};

} // namespace PPMD

#endif
