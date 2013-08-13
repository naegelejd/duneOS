#ifndef ASSERT_H
#define ASSERT_H

#ifndef NDEBUG

#define KASSERT(cond)                                   \
do {                                                    \
    if (!cond) {                                        \
        kprintf("Failed Assertion: %s at %s:%s:%d\n",   \
                #cond, __FILE__, __func__, __LINE__);   \
        khalt();                                        \
    }                                                   \
} while (0)

#else

#define KASSERT(cond)

#endif
