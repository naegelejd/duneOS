#ifndef DUNE_ASSERT_H
#define DUNE_ASSERT_H

#include "dune.h"

#ifndef NDEBUG

#define KASSERT(cond)                                   \
do {                                                    \
    if (!(cond)) {                                        \
        kprintf("Failed Assertion: %s at %s:%s:%d\n",   \
                #cond, __FILE__, __func__, __LINE__);   \
        khalt();                                        \
    }                                                   \
} while (0)

#else /* NDEBUG */

#define KASSERT(cond)

#endif /* NDEBUG */

#endif /* DUNE_ASSERT_H */
