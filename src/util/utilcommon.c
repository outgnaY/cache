#include "utilcommon.h"

ulint ut_max(ulint n1, ulint n2) {
    return ((n1 <= n2) ? n2 : n1);
}

/* upper limit */
ulint ut_2_log(ulint n) {
    ulint res;
    res = 0;
    assert(n > 0);
    n--;
    for (;;) {
        n >>= 1;
        if (n == 0) {
            break;
        }
        res++;
    }
    return res + 1;
}


ulint ut_2_exp(ulint n) {
    return 1 << n;
}