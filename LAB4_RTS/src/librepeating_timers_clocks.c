/**
 * @brief Modulo para crear las señales y relojes POSIX
 */
#include "librepeating_timers_clocks.h"
#include "synchronization_communication.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static void add_us(struct timespec *clock, uint64_t offset);
/**
 * @brief Suspende al hilo actual hasta que la señal asociada
 * a la sigset de la estructura pase al estado pendiente
 * @param p: referencia a la estructura periodic_signal
 * @return 0 si todo salio bien, 1 si el hilo pasó al estado pendiente por un evento distinto a la señal
 */
int wait_periodic_signal(struct periodic_signal *p) 
{
        int sig; //Senal que retorna el sigwait
        //Se espera a que la señal haga que el hilo esté en estado pendiente 
        if (sigwait(&p->sigset, &sig) != 0) {
                PRINT_DATA(stdout, "[wait_periodic_signal]: sigwait no debió retornar\n");
                return 1;
        }
        return 0;
}

/**
 * @brief Suspende al hilo actual hasta que se cumpla el tiempo objetivo
 * y actualiza el tiempo de la proxima actualizacion
 * @param p: referencia a la estructura periodic_task
 * @return 0 si todo salio bien, 1 si el hilo despertó por un motivo distinto a alcanzar el tiempo
 */
int wait_clock(struct periodic_task *p)
{
        //Hace que el hilo duerma con una resolucion de nanosegundos, la fuente de reloj es el CLOCK_REALTIME
        if (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &p->next_activation, NULL) > 0) {
                PRINT_DATA(stderr, "[wait_clock]: El reloj no se despertó adecuadamente\n");
                return 1;
        }
        //Actualiza la proxima ejecucion de la tarea del hilo
        add_us(&p->next_activation, p->period);
        return 0;
}

/**
 * @brief Agrega microsegundos a un timespec determinado
 * @param timespec donde se almacerá la nueva marca de tiempo
 * @param offset cantidad de US a agregar
 */
static void add_us(struct timespec *clock, uint64_t offset)
{
        //Primero se deben juntar los nanosegundos de la marca con el offset
        uint64_t actual_time_ns = clock->tv_nsec + US_TO_NS(offset);
        //Se adicional el nuevo tiempo. Se debe dividir entre la cantidad de segundos que se agregan
        clock->tv_sec += SECS_PER_NS(actual_time_ns);
        //Y los nanosegundos restantes para la marca de tiempo.
        //Los nanosegundos nunca deben superar 999.999.999ns porque eso implicaría que hay uno o varios segundos adicionales
        clock->tv_nsec = REMAIN_NS_FROM_SECS(actual_time_ns);
}

/**
 * @brief Crea una senal POSIX periodica con un offset y un periodo
 * @param offset us que se deben esperar antes de la primera ejecucion
 * @param period cada cuantos us debe activarse la señal luego de ser activada por primera vez
 * @param signotif a que numero se señal se asignará el timer
 * @return puntero en el heap con la información de la señal creada, NULL si algo falló.
 */
struct periodic_signal *create_periodic_signal(uint64_t offset, uint32_t period, const int32_t signotif)
{
        //Tratar de reservar heap y validar que se haya podido hacer
        struct periodic_signal *p = malloc(sizeof(struct periodic_signal));

        if (p == NULL) {
                return NULL;
        }
        /*
        Se inicializa el timer y la estructura sigevent.
        Esto no es obligatorio pero evita que las estructuras y los tipos
        estén en valores aleatorios.
        */
        memset(&p->t, 0, sizeof(p->t));
        memset(&p->sigev, 0, sizeof(p->sigev));

        /*
                Primero se agrega el conjunto de señales del periodic signal a un grupo vacio
                Luego se agrega el número de la señal a dicho conjunto.
        */
        sigemptyset(&p->sigset);
        sigaddset(&p->sigset, signotif);

        p->sigev.sigev_notify = SIGEV_SIGNAL; //La señal notificará al proceso
        p->sigev.sigev_signo = signotif; //Con este número de señal

        //Se crea el timer a la señal asociada al sigev. El reloj de referencia es el reloj de pared
        if (timer_create(CLOCK_REALTIME, &p->sigev, &p->timer) != 0) {
                free(p); //Se libera el heap por si algo evita crear el timer
                PRINT_DATA(stderr, "[create_periodic_signal]: No pudo crearse el timer\n");
                return NULL;
        }

        //Ahora, hay que definir cuantos segundos y us debe esperar el timer hasta activar a la señal
        //Este timespec controla el offset en segundos y nanosegundos
        p->t.it_value.tv_sec = US_TO_SECS(offset);
        p->t.it_value.tv_nsec = REMAIN_NS_FROM_US(offset);
        //Este timespec controla el periodo en segundos y nanosegundos
        p->t.it_interval.tv_sec = US_TO_SECS(period);
        p->t.it_interval.tv_nsec = REMAIN_NS_FROM_US(period);

        //Se establece el timer de manera periódica. Ahora la señal se ejecutará por primera vez cada offset us
        //Y volverá a ejecutarse cada period us 
        if (timer_settime(p->timer, 0, &p->t, NULL) != 0) {
                free(p); //Si ocurre un error se libera heap
                PRINT_DATA(stderr, "[create_periodic_signal]: No pudo programarse el timer\n");
                return NULL;
        }

        return p;
}
/**
* @brief crea la estructura asociada al periodic_task usando relojes POSIX a partir de un offset y un periodo
* @param offset cuantos us espera la tarea antes de ejecutarse por primera vez
* @param period cada cuantos us se ejecuta la tarea luego de la primera ejecución
* @return Si todo sale bien, se retorna un puntero en el heap con los datos de la tarea creada, si no se retorna NULL
*/
struct periodic_task *create_periodic_task(uint64_t offset, uint32_t period) 
{       
        //Se trata de reservar y se valida la reserva en el heap
        struct periodic_task *p = malloc(sizeof(struct periodic_task));

        if (p == NULL) {
                return NULL;
        }
        //Se toma la referencia de tiempo actual, para, a partir de ella definir el reloj
        //La fuente es el reloj de pared
        if (clock_gettime(CLOCK_REALTIME, &p->next_activation) != 0) {
                free(p); //Si ocurre un error se libera el heap
                PRINT_DATA(stderr, "[create_periodic_task]");
                return NULL;
        }

        //Ahora que se tiene la referencia temporal inicial, se define el tiempo objetivo para el reloj
        add_us(&p->next_activation, offset);
        //Se almacena el periodo para las activaciones sucesivas
        p->period = period;

        return p;
}