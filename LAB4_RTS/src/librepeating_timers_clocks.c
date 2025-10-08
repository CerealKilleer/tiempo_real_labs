#include "librepeating_timers_clocks.h"
#include "synchronization_communication.h"
#include <stdlib.h>
#include <stdio.h>

void wait_clock(struct periodic_task *p)
{
        if (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &p->next_activation, NULL) > 0) {
                pthread_mutex_lock(&data_mutex);
                fprintf(stderr, "[wait_clock]: El reloj no se despertó adecuadamente\n");
                pthread_mutex_unlock(&data_mutex);

                return;
        }

        add_us(&p->next_activation, p->period);
}

void add_us(struct timespec *clock, uint64_t offset)
{
        uint64_t actual_time_ns = clock->tv_nsec + US_TO_NS(offset);
        clock->tv_sec += SECS_PER_NS(actual_time_ns);
        clock->tv_nsec = REMAIN_NS_FROM_SECS(actual_time_ns);
}

struct periodic_task *create_periodic_task(uint64_t offset, uint32_t period) 
{
        struct periodic_task *p = malloc(sizeof(struct periodic_task));

        if (p == NULL) {
                return NULL;
        }

        if (clock_gettime(CLOCK_REALTIME, &p->next_activation) != 0) {
                pthread_mutex_lock(&data_mutex);
                perror("[create_periodic_task]");
                pthread_mutex_unlock(&data_mutex);

                return NULL;
        }

        add_us(&p->next_activation, offset);
        p->period = period;

        return p;
}