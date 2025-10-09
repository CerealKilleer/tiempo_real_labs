#ifndef __LIBREPEATING_TIMERS_CLOCKS_H__
#define __LIBREPEATING_TIMERS_CLOCKS_H__

        #include <stdint.h>
        #include <time.h>
        #include <signal.h>
        #include <sys/types.h>

        #define NS_PER_MS (1000000)
        #define SEC_TO_NS(x) ((x) * NS_PER_SEC)
        #define NS_TO_MS(x) ((x) / NS_PER_MS)
        #define NS_PER_SEC (1*1000*1000000)
        #define US_TO_NS(x) ((x) * 1000)
        #define SECS_PER_NS(x) ((x) / NS_PER_SEC)
        #define REMAIN_NS_FROM_SECS(x) ((x) % NS_PER_SEC)
        #define US_PER_SEC (1000000)
        #define US_TO_SECS(x) ((x) / US_PER_SEC)
        #define NS_PER_US (1000)
        #define REMAIN_NS_FROM_US(x) (((x) % US_PER_SEC) * NS_PER_US)

        struct periodic_task {
                struct timespec next_activation;
                uint32_t period;
        };

        struct periodic_signal {
                sigset_t sigset;
                struct itimerspec t;
                struct sigevent sigev;
                timer_t timer;
        };

        int wait_clock(struct periodic_task *p);
        void add_us(struct timespec *clock, uint64_t offset);
        struct periodic_task *create_periodic_task(uint64_t offset, uint32_t period);
        struct periodic_signal *create_periodic_signal(uint64_t offset, uint32_t period, const int32_t signotif);
        int wait_periodic_signal(struct periodic_signal *p);
#endif