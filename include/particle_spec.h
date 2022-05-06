#ifndef _PPMD_PARTICLE_SPEC
#define _PPMD_PARTICLE_SPEC

#include <cstdint>
#include <map>
#include <mpi.h>
#include <string>
#include <vector>

namespace PPMD {

template <typename U> class Sym {
  private:
  public:
    const std::string name;
    Sym(const std::string name) : name(name) {}

    // std::map uses std::less as default comparison operator
    bool operator<(const Sym &sym) const { return this->name < sym.name; }
};

template <typename T> class ParticleProp {
  private:
  public:
    const Sym<T> sym;
    const std::string name;
    const int ncomp;
    const bool positions;
    ParticleProp(const Sym<T> sym, int ncomp, bool positions = false)
        : sym(sym), name(sym.name), ncomp(ncomp), positions(positions) {}
};

class ParticleSpec {
  private:
    template <typename... T> void push(T... args) { this->push(args...); }
    void push(ParticleProp<PPMD::REAL> pp) {
        this->properties_real.push_back(pp);
    }
    void push(ParticleProp<PPMD::INT> pp) {
        this->properties_int.push_back(pp);
    }
    template <typename... T> void push(ParticleProp<PPMD::REAL> pp, T... args) {
        this->properties_real.push_back(pp);
        this->push(args...);
    }
    template <typename... T> void push(ParticleProp<PPMD::INT> pp, T... args) {
        this->properties_int.push_back(pp);
        this->push(args...);
    }

  public:
    std::vector<ParticleProp<PPMD::REAL>> properties_real;
    std::vector<ParticleProp<PPMD::INT>> properties_int;

    template <typename... T> ParticleSpec(T... args) { this->push(args...); };

    ~ParticleSpec(){};
};

} // namespace PPMD

#endif
