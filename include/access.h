#ifndef _PPMD_ACCESS
#define _PPMD_ACCESS

#include "typedefs.h"

namespace PPMD{

template <template <typename...> class T, typename U>
class ColumnMajorColumnAccessor {
  private:
    T<U> &base;
    const int &stride;
    const int &rowx;

  public:
    ColumnMajorColumnAccessor(T<U> &base, const int &stride, const int &rowx)
        : base(base), stride(stride), rowx(rowx){};

    U &operator[](const int colx) {
        return this->base[colx * this->stride + rowx];
    };
};

template <template <typename...> class T, typename U>
class ColumnMajorRowAccessor {
  private:
    T<U> &base;
    const int &stride;

  public:
    ColumnMajorRowAccessor(T<U> &base, const int &stride)
        : base(base), stride(stride){};

    ColumnMajorColumnAccessor<T, U> operator[](const int rowx) {
        return ColumnMajorColumnAccessor<T, U>{this->base, this->stride, rowx};
    };
};

class AccessMode {
};

class READ : public AccessMode {

};

class WRITE : public AccessMode {

};

template<typename T>
class Accessor {
  private:
    T* d_ptr;
  public:

    AccessMode mode;
    Accessor(T* d_ptr, AccessMode mode) : d_ptr(d_ptr), mode(mode){}


    T& operator[](int index) {
        return this->d_ptr[index];
    };

};
}

#endif
