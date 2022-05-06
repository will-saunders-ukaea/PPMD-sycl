#ifndef _PPMD_ACCESS
#define _PPMD_ACCESS

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

#endif
