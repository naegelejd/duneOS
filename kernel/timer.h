#ifndef DUNE_TIMER_H
#define DUNE_TIMER_H

uint32_t get_ticks(void);
void timer_install();
void delay(unsigned int ticks);
void set_timer_frequency(unsigned int hz);

#endif /* DUNE_TIMER_H */
