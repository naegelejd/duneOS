#include "string.h"

#define FAIL 0
#define PASS 1
#define ASSERT(x)   do { if(!(x)) return FAIL; } while (0)

int test_strcmp()
{
    char* s0 = "";
    char* s1 = "Hello";
    char* s2 = "hello";
    char* s3 = "Hell0";
    char* s4 = "Hello\n";

    ASSERT(strcmp(s0, s0) == 0);
    ASSERT(strcmp(s0, s1) < 0);
    ASSERT(strcmp(s0, s2) < 0);
    ASSERT(strcmp(s0, s3) < 0);
    ASSERT(strcmp(s0, s4) < 0); /* s4 longer */

    ASSERT(strcmp(s1, s1) == 0);
    ASSERT(strcmp(s1, s2) < 0); /* h > H */
    ASSERT(strcmp(s1, s3) > 0); /* o > 0 */
    ASSERT(strcmp(s1, s4) > 0); /* o > 0 */

    ASSERT(strcmp(s2, s2) == 0);
    ASSERT(strcmp(s2, s3) > 0);
    ASSERT(strcmp(s2, s4) > 0);

    ASSERT(strcmp(s3, s3) == 0);
    ASSERT(strcmp(s3, s4) > 0); /* o > 0 */

    ASSERT(strcmp(s4, s4) == 0);

    return PASS;
}

int test_strncmp()
{
    char* s0 = "";
    char* s1 = "Hello";
    char* s2 = "hello";
    char* s3 = "Hell0";
    char* s4 = "Hello\n";

    ASSERT(strcmp(s0, s0) == 0);
    ASSERT(strcmp(s0, s1) < 0);
    ASSERT(strcmp(s0, s2) < 0);
    ASSERT(strcmp(s0, s3) < 0);
    ASSERT(strcmp(s0, s4) < 0); /* s4 longer */

    ASSERT(strcmp(s1, s1) == 0);
    ASSERT(strcmp(s1, s2) < 0); /* h > H */
    ASSERT(strcmp(s1, s3) > 0); /* o > 0 */
    ASSERT(strcmp(s1, s4) > 0); /* o > 0 */

    ASSERT(strcmp(s2, s2) == 0);
    ASSERT(strcmp(s2, s3) > 0);
    ASSERT(strcmp(s2, s4) > 0);

    ASSERT(strcmp(s3, s3) == 0);
    ASSERT(strcmp(s3, s4) > 0); /* o > 0 */

    ASSERT(strcmp(s4, s4) == 0);

    return PASS;

    return PASS;
}

int test_strcpy()
{
    char buf[10] = "x";
    char *dst = buf;

    dst = strcpy(dst, "");
    ASSERT(dst[0] == 0);

    dst = strcpy(dst, "Hello");
    ASSERT(dst[0] == 'H');
    ASSERT(dst[1] == 'e');
    ASSERT(dst[2] == 'l');
    ASSERT(dst[3] == 'l');
    ASSERT(dst[4] == 'o');
    ASSERT(dst[5] == '\0');

    dst = strcpy(dst, "Good\0bye");
    ASSERT(dst[0] == 'G');
    ASSERT(dst[1] == 'o');
    ASSERT(dst[2] == 'o');
    ASSERT(dst[3] == 'd');
    ASSERT(dst[4] == '\0');

    dst = strcpy(dst, "");
    ASSERT(dst[0] == '\0');

    return PASS;
}

int test_strncpy()
{
    char buf[10] = "\0";
    char *dst = buf;

    dst = strncpy(dst, "Hello", 10);

    return PASS;
}

int main(void)
{
    if (test_strcmp() != PASS) {
        return 1;
    }

    if (test_strncmp() != PASS) {
        return 1;
    }

    if (test_strcpy() != PASS) {
        return 1;
    }

    if (test_strncpy() != PASS) {
        return 1;
    }

    return 0;
}
