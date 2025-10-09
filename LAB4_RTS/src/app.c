#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "app.h"
#include "libsense_vel.h"
#include "libABS_breaks.h"
#include "librepeating_timers_clocks.h"
#include "libfuel_injection.h"
#include "synchronization_communication.h"

#define MAX_THREADS 4
#define MS_PER_SEC (1000)
#define SECS_PER_MS(x) ((x) / MS_PER_SEC)
#define MS_TO_US(x) ((x) * MS_PER_SEC)

#define OFFSET_SPEED_SENSOR_MS 1000
#define PERIOD_SPEED_SENSOR_MS 20
#define OFFSET_ABS_CONTROL_MS 5000
#define PERIOD_ABS_CONTROL_MS 40
#define OFFSET_FUEL_INJECTION_MS 1000
#define PERIOD_FUEL_INJECTION_US 12500

#define TIMESPEC_DIFF_NS(start, stop) ((stop.tv_nsec) + SEC_TO_NS(stop.tv_sec)) - ((start.tv_nsec) + SEC_TO_NS(start.tv_sec))
#define TIMESPEC_DIFF_MS(start, stop) NS_TO_MS((float)(TIMESPEC_DIFF_NS(start, stop)))

pthread_mutex_t data_mutex;
pthread_mutex_t stdout_mutex;
pthread_cond_t abs_cond;
pthread_cond_t speed_sensor_cond;

struct speed_data data = {
        .stoped = false,
        .abs_ready = true,
        .speed_sensor_ready = false
};

void *fuel_injection(void *args)
{
        int32_t siginfo = SIGRTMIN;
        struct periodic_signal *p = create_periodic_signal(MS_TO_US(OFFSET_FUEL_INJECTION_MS), 
                                                        PERIOD_FUEL_INJECTION_US, siginfo);
        struct timespec start, stop;
        float diff;

        if (p == NULL) {
                PRINT_DATA(stderr, "[fuel_injection thread]: No se pudo crear la tarea periodica\n");
                return NULL; 
        }

        PRINT_DATA_ARGS(stdout, "******* En %d s inicia la inyección de combustible *******\n", SECS_PER_MS(OFFSET_FUEL_INJECTION_MS));

        while (1) {
                clock_gettime(CLOCK_REALTIME, &start);
                wait_periodic_signal(p);
                injection();
                clock_gettime(CLOCK_REALTIME, &stop);
                
                diff = TIMESPEC_DIFF_MS(start, stop);

                PRINT_DATA_ARGS(stdout, "[fuel_injection thread] desde la última ejecución: %.3f ms\n", diff);
        }
}

void *abs_control(void *args)
{
        struct periodic_task *p = create_periodic_task(MS_TO_US(OFFSET_ABS_CONTROL_MS), 
                                                        MS_TO_US(PERIOD_ABS_CONTROL_MS));
        struct timespec start, stop;
        float diff;

        if (p == NULL) {
                PRINT_DATA(stderr, "[abs_control thread]: No se pudo crear la tarea periodica\n");
                return NULL; 
        }

        PRINT_DATA_ARGS(stdout, "******* En %d s inicia el control ABS *******\n", SECS_PER_MS(OFFSET_ABS_CONTROL_MS));

        while (1) {
                clock_gettime(CLOCK_REALTIME, &start);
                wait_clock(p);
                control_abs_breaks();
                clock_gettime(CLOCK_REALTIME, &stop);
                
                diff = TIMESPEC_DIFF_MS(start, stop);

                PRINT_DATA_ARGS(stdout, "[abs_control thread] desde la última ejecución: %.3f ms\n", diff);
        }
}

void *speed_sensor(void *args)
{
        struct periodic_task *p = create_periodic_task(MS_TO_US(OFFSET_SPEED_SENSOR_MS), 
                                                        MS_TO_US(PERIOD_SPEED_SENSOR_MS));
        struct timespec start, stop;
        float diff;

        if (p == NULL) {
                PRINT_DATA(stderr, "[speed_sensor thread]: No se pudo crear la tarea periodica\n");
                return NULL;
        }

        
        PRINT_DATA_ARGS(stdout, "******* En %d s inicia el sensor de velocidad *******\n", SECS_PER_MS(OFFSET_SPEED_SENSOR_MS));
       
        while (1) {
                clock_gettime(CLOCK_REALTIME, &start);
                wait_clock(p);
                sense_speed();
                clock_gettime(CLOCK_REALTIME, &stop);
                
                diff = TIMESPEC_DIFF_MS(start, stop);

                PRINT_DATA_ARGS(stdout, "[speed_sensor thread] desde la última ejecución: %.3f ms\n", diff);
        }
}

int init_app(void)
{
        pthread_t threads[MAX_THREADS];
        sigset_t alarm_sigset;
        pthread_mutex_init(&data_mutex, NULL);
        pthread_mutex_init(&stdout_mutex, NULL);
        pthread_cond_init(&abs_cond, NULL);
        pthread_cond_init(&speed_sensor_cond, NULL);

        sigemptyset(&alarm_sigset);
	for (uint8_t i = SIGRTMIN; i <= SIGRTMIN + 1; i++) {
                sigaddset(&alarm_sigset, i);
        }
		
	sigprocmask(SIG_BLOCK, &alarm_sigset, NULL);

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
        for (uint8_t i=0; i < MAX_THREADS; i++) {
                pthread_join(threads[i], NULL);
        }

        return 0;
}