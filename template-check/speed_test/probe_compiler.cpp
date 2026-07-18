#include <cfloat>
#include <cassert>
// pb_ds / rope 是否可用：能编过即 OK
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/rope>

// long double 是不是真扩展精度（CE = 退化成 double 或编译器异常）
static_assert(sizeof(long double) > sizeof(double), "ld == double");
static_assert(LDBL_MANT_DIG >= 64, "ld mantissa < 64 bits");

int main() {
    __int128 x = (__int128)1 << 100; assert(x > 0);
    __float128 y = (__float128)1; assert(y == 1);
}
