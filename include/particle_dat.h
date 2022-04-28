#ifndef _PPMD_PARTICLE_DAT
#define _PPMD_PARTICLE_DAT



template<typename T>
class ParticleDat {
  private:

  public:
    const int ncomp;
    const bool positions;

    ParticleDat(int ncomp, bool positions): ncomp(ncomp), positions(positions){}
    ParticleDat(int ncomp): ncomp(ncomp), positions(false){}
    ~ParticleDat(){}

};



#endif
