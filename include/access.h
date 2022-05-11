#ifndef _PPMD_ACCESS
#define _PPMD_ACCESS

#include "typedefs.h"

namespace PPMD {

template <template <typename...> class T, typename U>
class ColumnMajorColumnAccessor {
  private:
    T<U> &base;
    const int &stride;
    const int &rowx;

  public:
    ColumnMajorColumnAccessor(T<U> &base, const int &stride, const int &rowx)
        : base(base), stride(stride), rowx(rowx){};

    U &operator[](const int &colx) {
        return this->base[colx * this->stride + this->rowx];
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

    ColumnMajorColumnAccessor<T, U> operator[](const int &rowx) {
        return ColumnMajorColumnAccessor<T, U>{this->base, this->stride, rowx};
    };
};

template <typename T> class RawPointerColumnMajorColumnAccessor {
  private:
    T *d_ptr;
    const int stride;
    const int rowx;

  public:
    RawPointerColumnMajorColumnAccessor(T *d_ptr, const int stride,
                                        const int rowx)
        : d_ptr(d_ptr), stride(stride), rowx(rowx){};

    T &operator[](const int &colx) {
        return d_ptr[colx * this->stride + this->rowx];
    };
};

template <typename T> class RawPointerColumnMajorRowAccessor {
  private:
    T *d_ptr;
    const int stride;

  public:
    RawPointerColumnMajorRowAccessor(T *d_ptr, const int stride)
        : d_ptr(d_ptr), stride(stride){};

    RawPointerColumnMajorColumnAccessor<T> operator[](const int rowx) {
        return RawPointerColumnMajorColumnAccessor<T>{this->d_ptr, this->stride,
                                                      rowx};
    };
};

class AccessMode {};

class READ : public AccessMode {};

class WRITE : public AccessMode {};

template <typename T> class Accessor {
  private:
    T *d_ptr;
    const int stride;

  public:
    AccessMode mode;
    Accessor(T *d_ptr, AccessMode mode, const int stride)
        : d_ptr(d_ptr), mode(mode), stride(stride) {}

    // T &operator[](int index) const { return this->d_ptr[index]; };
    RawPointerColumnMajorRowAccessor<T> operator[](const int rowx) const {
        return RawPointerColumnMajorRowAccessor<T>(this->d_ptr, this->stride,
                                                   rowx);
    };
};
} // namespace PPMD

#endif
