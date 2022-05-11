#ifndef _PPMD_DOMAIN
#define _PPMD_DOMAIN

#include "typedefs.h"

namespace PPMD {

class Mesh {
  private:
    int cell_count;

  public:
    Mesh(int cell_count) : cell_count(cell_count){};
    int get_cell_count() { return this->cell_count; };
};

class Domain {
  private:
  public:
    Mesh &mesh;
    Domain(Mesh &mesh) : mesh(mesh) {}
    ~Domain() {}
};

} // namespace PPMD

#endif
