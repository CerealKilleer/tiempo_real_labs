/**
 * @brief App principal de la practica 4.
 * Crea los hilos para las 4 tareas.
 * Sensor de velocidad: periodo 20ms y offset de 1s
 * Control ABS : periodo 40ms y offset de 5s
 * Inyeccion de combustible: periodo 12.5 ms y offset de 1s
 * Sensor de posicion: periodo de 200ms y offset de 1s
 * Los primeros dos hilos usan relojes POSIX y los restantes señales POSIX.
 * Se simula un auto que va a aproximadamente 80km/h y a los 5 segundos cominza a frenar hasta detenerse
 */
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "app.h"
#include "libsense_vel.h"
#include "libABS_breaks.h"
#include "librepeating_timers_clocks.h"
#include "libfuel_injection.h"
#include "libgps.h"
#include "synchronization_communication.h"

pthread_mutex_t data_mutex; //Mutex para acceder a la información de la velocidad
pthread_mutex_t stdout_mutex; //Mutex para la salida estándar
pthread_cond_t abs_cond; //Variable condicoinal para el control abs
pthread_cond_t speed_sensor_cond; //Variable condicional para el sensor de velocidad

/* La intención de las banderas es garantizar que el control abs, la inyección o el sensor de posición solo 
   actúen cuando hay medidas validas del sensor de velocidad. Por otro lado,
   el sensor de velocidad debe actúar únicamente cuando el abs establezca la presión de los frenos
   en un valor válido.
*/
struct speed_data data = { //Se crea la struct de los datos del sensor de velocidad
        .stoped = false, //El auto inicia en movimiento
        .abs_ready = true, //Se asume que el ABS está listo. Inicialmente no se pisan los frenos
        .speed_sensor_ready = false //El sensor de velocidad no ha hecho la primera medición.
};

/**
 * @brief Función de entrada para el hilo de cálculo de posición. Se hace uso señales POSIX.
 * El sensor de posición tiene un offset de 1 segundo y un periodo de 200ms.
 * Se considera una tarea soft pues pequeñas variaciones en el tiempo de atención no deben variar
 * el resultado de la posición.
 * @param args referencia a los argumentos que se pasan a la función al crear el hilo
 */

void *position_sensor(void *args)
{
        //Este hilo usará la señal SIGRTMIN + 1 (35)
        int32_t siginfo = SIGRTMIN + 1;
        //Se crea la señal periódica
        struct periodic_signal *p = create_periodic_signal(MS_TO_US(OFFSET_POSITION_SENSOR_MS), 
                                                        MS_TO_US(PERIOD_POSITION_SENSOR_MS), siginfo);
        struct timespec start, stop; //Para medir el periodo
        float diff;
        //Se valida la creación de la señal periódica
        if (p == NULL) {
                PRINT_DATA(stderr, "[position_sensor thread]: No se pudo crear la tarea periodica\n");
                return NULL; 
        }
        //Los PRINT_DATA y los PRINT_DATA_ARGS son utilidades thread safe para escribir en la salida estándar
        PRINT_DATA_ARGS(stdout, "******* En %d s inicia %s *******\n", 
                       SECS_PER_MS(OFFSET_POSITION_SENSOR_MS), (char *)args);

        //Cuerpo principal del hilo
        while (1) {
                clock_gettime(CLOCK_REALTIME, &start);
                wait_periodic_signal(p);
                clock_gettime(CLOCK_REALTIME, &stop); //Aquí debe haber aproximadamente un periodo
                diff = TIMESPEC_DIFF_MS(start, stop);
                show_position(MS_TO_S(diff)); //Se muestra la nueva posición del auto
                
                PRINT_DATA_ARGS(stdout, "[position_sensor] desde la última ejecución: %.3f ms\n", diff);
        }
}

/**
 * @brief Función de entrada para el hilo de inyeccion de combustible. Se hace uso señales POSIX.
 * La inyección de combustible tiene un offset de 1 segundo y un periodo de 12.5ms.
 * @param args referencia a los argumentos que se pasan a la función al crear el hilo
 */
void *fuel_injection(void *args)
{
        //Esta primera tarea usará la señal número SIGRTMIN (34) en la maquina donde se desarrlló
        int32_t siginfo = SIGRTMIN;
        struct periodic_signal *p = create_periodic_signal(MS_TO_US(OFFSET_FUEL_INJECTION_MS), 
                                                        PERIOD_FUEL_INJECTION_US, siginfo);
        //Se creó la señal periódica
        struct timespec start, stop; //Para saber el periodo de ejecución del job
        float diff;

        //Validar la creación del periodic signal
        if (p == NULL) {
                PRINT_DATA(stderr, "[fuel_injection thread]: No se pudo crear la tarea periodica\n");
                return NULL; 
        }

        PRINT_DATA_ARGS(stdout, "******* En %d s inicia %s *******\n", 
                         SECS_PER_MS(OFFSET_FUEL_INJECTION_MS),  (char *)args);
        
        while (1) {
                clock_gettime(CLOCK_REALTIME, &start); //Inicia la medición del tiempo
                wait_periodic_signal(p); //Se espera a que el timer active la señal
                clock_gettime(CLOCK_REALTIME, &stop); //Finaliza la medición del tiempo
                injection(); //En este punto debe haber pasado más o menos un periodo o un offset si es el inicio
        
                diff = TIMESPEC_DIFF_MS(start, stop); //Debe imprimir un valor muy cercano al periodo o al offset

                PRINT_DATA_ARGS(stdout, "[fuel_injection thread] desde la última ejecución: %.3f ms\n", diff);
        }
}

/**
 * @brief Función de entrada para el hilo del control ABS. Se hace uso del reloj POSIX.
 * El control ABS tiene un offset de 5 segundos y un periodo de 40ms.
 * @param args referencia a los argumentos que se pasan a la función al crear el hilo
 */
void *abs_control(void *args)
{
        //Se crea la estructura
        struct periodic_task *p = create_periodic_task(MS_TO_US(OFFSET_ABS_CONTROL_MS), 
                                                        MS_TO_US(PERIOD_ABS_CONTROL_MS));
        struct timespec start, stop; //Contar el tiempo entre ejecuciones del job
        float diff;
        //Validar la creacion del periodic_task
        if (p == NULL) {
                PRINT_DATA(stderr, "[abs_control thread]: No se pudo crear la tarea periodica\n");
                return NULL; 
        }
        //Imprime información de manera thread safe
        PRINT_DATA_ARGS(stdout, "******* En %d s inicia %s *******\n", 
                        SECS_PER_MS(OFFSET_ABS_CONTROL_MS),  (char *)args);
        //Cuerpo principal del thread
        while (1) {
                clock_gettime(CLOCK_REALTIME, &start); //Cuando inicia la espera para el job
                wait_clock(p);
                clock_gettime(CLOCK_REALTIME, &stop); //Cuando el job está listo
                control_abs_breaks();
                
                diff = TIMESPEC_DIFF_MS(start, stop); //Esto debe ser aproximadamente igual al periodo o al offset

                PRINT_DATA_ARGS(stdout, "[abs_control thread] desde la última ejecución: %.3f ms\n", diff);
        }
}

/**
 * @brief Función de entrada para el hilo 1 del sensor de velocidad.
 * Crea el periodic_task, suspende el hilo mientras no se cumpla la condición de tiempo
 * e imprime en pantalla la información del sensor y del tiempo entre periodos
 * @param args referencia a los parámetros de inicio que puede recibir la función del hilo
 */
void *speed_sensor(void *args)
{       
        //Crea el periodic_task del sensor de velocidad con un offset de 1s y un periodo de 20ms
        struct periodic_task *p = create_periodic_task(MS_TO_US(OFFSET_SPEED_SENSOR_MS), 
                                                        MS_TO_US(PERIOD_SPEED_SENSOR_MS));
        struct timespec start, stop; //Calcular el periodo entre ejecuciones del job
        float diff; //El periodo en ms

        //Se valida que se haya podido crear el periodic_task, en caso contrario el hilo termina
        if (p == NULL) {
                //Se escribe en stderr que es el mismo stdout en la configuración por defecto.
                PRINT_DATA(stderr, "[speed_sensor thread]: No se pudo crear la tarea periodica\n");
                return NULL;
        }

        //Mensaje de inicio, notese el uso del macro variadico
        PRINT_DATA_ARGS(stdout, "******* En %d s inicia %s *******\n", 
                        SECS_PER_MS(OFFSET_SPEED_SENSOR_MS), (char *)args);
       
        while (1) {
                //Se toma el tiempo antes de esperar suspender el hilo
                clock_gettime(CLOCK_REALTIME, &start);
                wait_clock(p); //Se suspende el hilo hasta que se cumpla el offset o el periodo
                clock_gettime(CLOCK_REALTIME, &stop); //Se muestrea el instante de inicio del job
                sense_speed(); // Operaciones asociadas al job del sensado de velocidad
                diff = TIMESPEC_DIFF_MS(start, stop); //Se toma la diferencia de tiempos
                //Se muestra la diferencia, esta debe ser aproximadamente igual al offset o al periodo de ejecución
                PRINT_DATA_ARGS(stdout, "[speed_sensor thread] desde la última ejecución: %.3f ms\n", diff);
        }
}

/**
 * @brief Es el main de la app, podría haber sido directamente main pero
 * se prefirió el desacomple para más modularidad
 */
int init_app(void)
{
        pthread_t threads[MAX_THREADS]; //En lugar de crear cuatro variables, se crea un arreglo de cuatro elementos
        sigset_t alarm_sigset; //Es el conjunto de las dos señales a utilizar
        pthread_mutex_init(&data_mutex, NULL); //Se inicia el mutex de data
        pthread_mutex_init(&stdout_mutex, NULL); //Se inicia el mutex de la salida estandar
        pthread_cond_init(&abs_cond, NULL); //Se inicia la variable condicional del abs
        pthread_cond_init(&speed_sensor_cond, NULL); //Se inicia la variable condicional del sensor de velocidad
        //Todas las inicializaciones se hacen con los atributos por defecto, por eso el NULL

        //Se agrega el conjunto de señales a un conjunto vacío
        sigemptyset(&alarm_sigset);
        //La dos señales a utilizar son las SIGRTMIN y SIGRTMIN + 1
        //¿Es innecesario el for? ¿y si algun dia fueran más señales?
	for (uint8_t i = SIGRTMIN; i <= SIGRTMIN + 1; i++) {
                sigaddset(&alarm_sigset, i); //Se agregan al conjunto
        }
	//Para poder activar las señales de manera periódica se deben bloquear con una mascara, por ejemplo SIG_BLOCK
	sigprocmask(SIG_BLOCK, &alarm_sigset, NULL);

        //Se crean los 4 hilos, para un total de 5 contando el main.
        //Cada hilo recibe un parametro que es su nombre
        if (pthread_create(&threads[0], NULL, speed_sensor, "Speed Sensor") != 0) {
                PRINT_DATA(stderr, "[init_app]: no pudo crearse el hilo Speed Sensor\n");
                return 1;
        }

        if (pthread_create(&threads[1], NULL, abs_control, "ABS Control") != 0) {
                PRINT_DATA(stderr, "[init_app]: no pudo crearse el hilo ABS Control\n");
                return 1;
        }

        if (pthread_create(&threads[2], NULL, fuel_injection, "Fuel Injection") != 0) {
                PRINT_DATA(stderr, "[init_app]: no pudo crearse el hilo Fuel Injection\n");
                return 1;
        }

        if (pthread_create(&threads[3], NULL, position_sensor, "Position sensor") != 0) {
                PRINT_DATA(stderr, "[init_app]: no pudo crearse el hilo Position Sensor\n");
                return 1;
        }

        //Se obliga a que los hilos se esperen entre si
        for (uint8_t i=0; i < MAX_THREADS; i++) {
                pthread_join(threads[i], NULL);
        }

        return 0;
}