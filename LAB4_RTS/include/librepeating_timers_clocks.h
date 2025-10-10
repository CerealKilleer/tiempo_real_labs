#ifndef __LIBREPEATING_TIMERS_CLOCKS_H__
#define __LIBREPEATING_TIMERS_CLOCKS_H__

        #include <stdint.h>
        #include <time.h>
        #include <signal.h>
        #include <sys/types.h>
        #include "utilities.h"        

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