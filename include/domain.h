#ifndef _PPMD_DOMAIN
#define _PPMD_DOMAIN

#include "typedefs.h"

namespace PPMD {

class Domain {};

class DummyDomain : public Domain {
  private:
  public:
    DummyDomain() {}
    ~DummyDomain() {}
};

} // namespace PPMD

#endif
