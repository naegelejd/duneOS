#include <stddef.h>
#include "io.h"
#include "date.h"

#define CURRENT_YEAR    2013

#define CMOS_ADDR_REG   0x70
#define CMOS_DATA_REG   0x71
#define CMOS_STATUS_REGA    0x0A
#define CMOS_STATUS_REGB    0x0B
#define NMI_DISABLE     0x1F
#define NMI_ENABLE      0x0F

#define BCD2BIN(bcd)    (((bcd) & 0xF) + ((bcd) >> 4) * 10)

static char* months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char* days[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

uint8_t read_cmos(uint8_t addr)
{
    outportb(CMOS_ADDR_REG, addr);
    return inportb(CMOS_DATA_REG);
}

void write_cmos(uint8_t addr, uint8_t val)
{
    outportb(CMOS_ADDR_REG, addr);
    outportb(CMOS_DATA_REG, NMI_DISABLE | val);
}

char* day_name(uint8_t wday)
{
    if (wday > 6) {
        return NULL;
    }
    return days[wday];
}

char* month_name(uint8_t mon)
{
    if (mon > 12) {
        return NULL;
    }
    return months[mon - 1];
}

void datetime(struct tm *tm_out)
{
    uint8_t update_in_progress = 1;
    while (update_in_progress) {
        update_in_progress = read_cmos(0x0A) & 0x80;
    }

    uint8_t sec = read_cmos(0x0);
    uint8_t min = read_cmos(0x02);
    uint8_t hour = read_cmos(0x04);
    uint8_t wday = read_cmos(0x06);
    uint8_t mday = read_cmos(0x07);
    uint8_t month = read_cmos(0x08);
    uint16_t year = (uint8_t)read_cmos(0x09);
    /* There is an "RTC century register" at offset 108 in ACPI's
     * "Fixed ACPI Description Table". If this byte is 0, then the RTC
     * does not have a century register, otherwise, it is the number
     * of the RTC register to use for the century...
     *
     * if it existed:
     * uint8_t century = read_cmos(CMOS_CENTURY_REG);
     *
     * For now, I'm going to 'deduce' the century based on the year
     */

    /* determine if values are packed BCD or Binary */
    uint8_t regB = read_cmos(CMOS_STATUS_REGB);
    if (!(regB & 0x04)) {
        sec = BCD2BIN(sec);
        min = BCD2BIN(min);
        hour = BCD2BIN(hour);
        wday = BCD2BIN(wday);
        mday = BCD2BIN(mday);
        month = BCD2BIN(month);
        year = BCD2BIN(year);
    }

    /* convert hour from 12-hour to 24-hour if necessary */
    if (!(regB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    /* fix year depending on century */
    year += (CURRENT_YEAR / 100) * 100;
    if (year < CURRENT_YEAR) {
        year += 100;
    }

    tm_out->sec = sec;
    tm_out->min = min;
    tm_out->hour = hour;
    tm_out->wday = wday;
    tm_out->mday = mday;
    tm_out->month = month;
    tm_out->year = year;
    tm_out->yday = 0;
    tm_out->isdst = 0;
}
