#include "librepeating_timers_clocks.h"
#include "synchronization_communication.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int wait_periodic_signal(struct periodic_signal *p) 
{
        int sig;
        if (sigwait(&p->sigset, &sig) != 0) {
                PRINT_DATA(stdout, "[wait_periodic_signal]: sigwait no debió retornar\n");
                return 1;
        }
        return 0;
}

int wait_clock(struct periodic_task *p)
{
        if (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &p->next_activation, NULL) > 0) {
                pthread_mutex_lock(&data_mutex);
                fprintf(stderr, "[wait_clock]: El reloj no se despertó adecuadamente\n");
                pthread_mutex_unlock(&data_mutex);
                return 1;
        }

        add_us(&p->next_activation, p->period);
        return 0;
}

void add_us(struct timespec *clock, uint64_t offset)
{
        uint64_t actual_time_ns = clock->tv_nsec + US_TO_NS(offset);
        clock->tv_sec += SECS_PER_NS(actual_time_ns);
        clock->tv_nsec = REMAIN_NS_FROM_SECS(actual_time_ns);
}

struct periodic_signal *create_periodic_signal(uint64_t offset, uint32_t period, const int32_t signotif)
{
        struct periodic_signal *p = malloc(sizeof(struct periodic_signal));

        if (p == NULL) {
                return NULL;
        }

        memset(&p->t, 0, sizeof(p->t));
        memset(&p->sigev, 0, sizeof(p->sigev));

        sigemptyset(&p->sigset);
        sigaddset(&p->sigset, signotif);
        p->sigev.sigev_notify = SIGEV_SIGNAL;
        p->sigev.sigev_signo = signotif;

        if (timer_create(CLOCK_REALTIME, &p->sigev, &p->timer) != 0) {
                free(p);
                PRINT_DATA(stderr, "[create_periodic_signal]: No pudo crearse el timer\n");
                return NULL;
        }

        p->t.it_value.tv_sec = US_TO_SECS(offset);
        p->t.it_value.tv_nsec = REMAIN_NS_FROM_US(offset);
        p->t.it_interval.tv_sec = US_TO_SECS(period);
        p->t.it_interval.tv_nsec = REMAIN_NS_FROM_US(period);

        if (timer_settime(p->timer, 0, &p->t, NULL) != 0) {
                free(p);
                PRINT_DATA(stderr, "[create_periodic_signal]: No pudo programarse el timer\n");
                return NULL;
        }

        return p;
}

struct periodic_task *create_periodic_task(uint64_t offset, uint32_t period) 
{
        struct periodic_task *p = malloc(sizeof(struct periodic_task));

        if (p == NULL) {
                return NULL;
        }

        if (clock_gettime(CLOCK_REALTIME, &p->next_activation) != 0) {
                free(p);
                PRINT_DATA(stderr, "[create_periodic_task]");
                return NULL;
        }

        add_us(&p->next_activation, offset);
        p->period = period;

        return p;
}