#ifndef DUNE_RTC_H
#define DUNE_RTC_H

#include "dune.h"

#define BCD2BIN(bcd)    (((bcd) & 0xF) + ((bcd) >> 4) * 10)

enum {
    CMOS_STATUS_REGA = 0x0A,
    CMOS_STATUS_REGB = 0x0B,
    CMOS_STATUS_REGC = 0x0C,
    NMI_ENABLE = 0x0F,
    CMOS_ADDR_REG = 0x70,
    CMOS_DATA_REG = 0x71,
    NMI_DISABLE = 0x80,
    CURRENT_YEAR = 2013
};

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

void rtc_install(void);


#endif /* DUNE_RTC_H */
