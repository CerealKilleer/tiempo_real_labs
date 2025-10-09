#ifndef __LIBREPEATING_TIMERS_CLOCKS_H__
#define __LIBREPEATING_TIMERS_CLOCKS_H__

        #include <stdint.h>
        #include <time.h>
        #include <signal.h>
        #include <sys/types.h>

        //Macro utiles para hacer conversiones de tiempo. Los nombres son suficientemente explicativos
        #define NS_PER_MS (1000000)
        #define NS_PER_SEC (1*1000*1000000)
        #define SEC_TO_NS(x) ((x) * NS_PER_SEC)
        #define NS_TO_MS(x) ((x) / NS_PER_MS)
        #define US_TO_NS(x) ((x) * 1000)
        #define SECS_PER_NS(x) ((x) / NS_PER_SEC)
        #define REMAIN_NS_FROM_SECS(x) ((x) % NS_PER_SEC)
        #define US_PER_SEC (1000000)
        #define US_TO_SECS(x) ((x) / US_PER_SEC)
        #define NS_PER_US (1000)
        #define REMAIN_NS_FROM_US(x) (((x) % US_PER_SEC) * NS_PER_US)

        //Las structuras para los relojes y los timers
        struct periodic_task { //Estructura para el periodic_task con el reloj
                struct timespec next_activation; //Tiempo objetivo
                uint32_t period; //Periodo
        };

        //Estructura para la señal periódica
        struct periodic_signal {
                sigset_t sigset; //Conjunto de señales asociadas al timer repetitivo
                struct itimerspec t; //Tiempo relativo para la próxima ejecución en s y ns
                struct sigevent sigev; //Información asociada a la señal del timer
                timer_t timer; //timer asociado a la señal
        };

        int wait_clock(struct periodic_task *p);
        struct periodic_task *create_periodic_task(uint64_t offset, uint32_t period);
        struct periodic_signal *create_periodic_signal(uint64_t offset, uint32_t period, const int32_t signotif);
        int wait_periodic_signal(struct periodic_signal *p);
#endif