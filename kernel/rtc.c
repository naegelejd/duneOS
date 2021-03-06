#include "irq.h"
#include "io.h"
#include "rtc.h"


static char* months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char* days[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static struct tm g_datetime;
static bool CMOS_BCD_VALUES = 0;


uint8_t read_cmos(uint8_t addr)
{
    outportb(CMOS_ADDR_REG, addr | NMI_DISABLE);
    return inportb(CMOS_DATA_REG);
}

void write_cmos(uint8_t addr, uint8_t val)
{
    outportb(CMOS_ADDR_REG, addr | NMI_DISABLE);
    outportb(CMOS_DATA_REG, val);
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
    if (mon < 1 || mon > 12) {
        return "?";
    }
    return months[mon - 1];
}

void datetime(struct tm *tm_out)
{
    tm_out->sec = g_datetime.sec;
    tm_out->min = g_datetime.min;
    tm_out->hour = g_datetime.hour;
    tm_out->wday = g_datetime.wday;
    tm_out->mday = g_datetime.mday;
    tm_out->month = g_datetime.month;
    tm_out->year = g_datetime.year;
    tm_out->yday = 0;
    tm_out->isdst = 0;
}

void rtc_handler(struct regs *r)
{
    (void)r;    /* prevent 'unused parameter' warning */
    static unsigned int ticks = 0;

    /* only read CMOS registers once a second (RTC @ 1024Hz)
     * in testing using QEMU, the RTC was very finnicky regarding
     * CMOS reads immediately after the RTC periodic interrupt is fired.
     * There's no reason to read the CMOS values at 1024Hz anyway, but I like
     * using an IRQ handler to populate a static date-time struct regularly
     * rather than performing the CMOS reads on demand.
     */
    ticks++;
    if (ticks / 1024 == 1) {
        ticks = 0;

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

        /* convert values if they're packed BCD */
        if (CMOS_BCD_VALUES) {
            sec = BCD2BIN(sec);
            min = BCD2BIN(min);
            hour = BCD2BIN(hour);
            wday = BCD2BIN(wday);
            mday = BCD2BIN(mday);
            month = BCD2BIN(month);
            year = BCD2BIN(year);
        }

        /* convert hour from 12-hour to 24-hour if necessary */
        uint8_t regB = read_cmos(CMOS_STATUS_REGB);
        if (!(regB & 0x02) && (hour & 0x80)) {
            hour = ((hour & 0x7F) + 12) % 24;
        }

        /* fix year depending on century */
        year += (CURRENT_YEAR / 100) * 100;
        if (year < CURRENT_YEAR) {
            year += 100;
        }

        g_datetime.sec = sec;
        g_datetime.min = min;
        g_datetime.hour = hour;
        g_datetime.wday = wday;
        g_datetime.mday = mday;
        g_datetime.month = month;
        g_datetime.year = year;
        g_datetime.yday = 0;
        g_datetime.isdst = 0;
    }

    /* register C must be read for another interrupt to occur
     * select register C, read, throw it away */
    outportb(CMOS_ADDR_REG, CMOS_STATUS_REGC);
    inportb(CMOS_DATA_REG);
}

void rtc_install(void)
{
    irq_install_handler(IRQ_RTC, rtc_handler);

    /* determine if values are packed BCD or Binary */
    uint8_t regB = read_cmos(CMOS_STATUS_REGB);
    if (!(regB & 0x04)) {
        CMOS_BCD_VALUES = true;
    }

    /* enable IRQ8 (RTC) - interrupts must be disabled! */
    /* read CMOS register B */
    uint8_t prev = read_cmos(CMOS_STATUS_REGB);
    /* write previous value with bit 6 turned on */
    write_cmos(CMOS_STATUS_REGB, prev | 0x40);

    enable_irq(IRQ_RTC);
}
