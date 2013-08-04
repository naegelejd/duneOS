#ifndef DATE_H
#define DATE_H

#include <stdint.h>


struct tm {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t mday;
    uint8_t month;
    uint8_t wday;   /* days since Monday */
    uint16_t yday;  /* days since January 1 */
    uint16_t year;  /* year since 0 AD */
    uint16_t isdst; /* is daylight savings flag */
};

/* expects day number 0-6 (days from Sunday */
char* day_name(uint8_t wday);

/* expects month number 1-12 */
char* month_name(uint8_t mon);

void datetime(struct tm *tm_out);

#endif
