#ifndef _PPMD_PARTICLE_DAT
#define _PPMD_PARTICLE_DAT

#include <string>

class ParticleDatBase {};

template<typename T>
class ParticleDat : public ParticleDatBase {
  private:

  public:
    const int ncomp;
    const bool positions;

    ParticleDat(int ncomp, bool positions): ncomp(ncomp), positions(positions){}
    ParticleDat(int ncomp): ncomp(ncomp), positions(false){}
    ~ParticleDat(){}

};



#endif
