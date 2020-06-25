#ifndef EVENT_TIME_H
#define EVENT_TIME_H
#include <stdlib.h>

typedef enum
{
TIMER_SINGLE_SHOT = 0,
TIMER_PERIODIC
} tmr_types;

typedef void (*time_handler)(size_t timer_id, void * user_data);

int     initialize_timers();
size_t  start_timer_long(unsigned int interval, time_handler handler, tmr_types type, void * user_data);
size_t  start_timer(unsigned int interval, time_handler handler, tmr_types type, void * user_data);
void    stop_timer(size_t timer_id);
void    close_timers();

#endif
