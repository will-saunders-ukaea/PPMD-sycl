#ifndef _PPMD_DOMAIN
#define _PPMD_DOMAIN

#include "typedefs.hpp"

namespace PPMD {

class Mesh {
  private:
    int cell_count;

  public:
    Mesh(int cell_count) : cell_count(cell_count){};
    inline int get_cell_count() { return this->cell_count; };
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
