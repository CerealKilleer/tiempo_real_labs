#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct {
    int id;
    unsigned int period_ticks;
    int priority;
    const char *name;
} task_info_t;

static volatile unsigned long tick_count = 0;
static pthread_mutex_t tick_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t tick_cond = PTHREAD_COND_INITIALIZER;
static int running = 1;

/* Hilo que genera el clock periódico basado en el RTC */
static void *rtc_periodic_thread(void *arg) {
    
    const char *dev = "/dev/rtc0";     // Cambiar a rtc1 si aplica
    unsigned int freq = *(unsigned int *)arg;
    unsigned long data;

    int fd = open(dev, O_RDONLY);
    if (fd < 0) {
        perror("Error abriendo el RTC");
        exit(EXIT_FAILURE);
    }    

    if (ioctl(fd, RTC_IRQP_SET, freq) < 0) {
        perror("Error al configurar frecuencia del RTC");
        exit(EXIT_FAILURE);
    }
    if (ioctl(fd, RTC_PIE_ON, 0) < 0) {
        perror("Error al habilitar interrupciones periódicas RTC");
        exit(EXIT_FAILURE);
    }

    while (running) {
        read(fd, &data, sizeof(data));

        pthread_mutex_lock(&tick_mutex);
        tick_count++;
        pthread_cond_broadcast(&tick_cond);
        pthread_mutex_unlock(&tick_mutex);
    }

    ioctl(fd, RTC_PIE_OFF, 0);
    close(fd);
    return NULL;
}

/* Hilo de tarea periódica controlada por ticks */
static void *periodic_task(void *arg) {
    task_info_t *info = (task_info_t *)arg;
    unsigned long next_release;

    pthread_mutex_lock(&tick_mutex);
    next_release = tick_count + info->period_ticks;
    pthread_mutex_unlock(&tick_mutex);

    while (running) {

        pthread_mutex_lock(&tick_mutex);
        while (tick_count < next_release)
            pthread_cond_wait(&tick_cond, &tick_mutex);
        next_release += info->period_ticks;
        pthread_mutex_unlock(&tick_mutex);

        printf("[tick %lu] Ejecutando %s (periodo: %u ticks)\n",
               tick_count, info->name, info->period_ticks);
        usleep(50000);  // Tiempo simulado de ejecución de la tarea
    }
    return NULL;
}

int main(void) {
    mlockall(MCL_CURRENT | MCL_FUTURE);  // Evita fallos de página
    unsigned int freq = 64;  // Frecuencia del RTC en Hz → cada tick ≈ 15.6ms

    task_info_t tasks[3] = {
        {1, 16, 80, "Tarea 1"},  // Periodo: 250 ms, prioridad más alta
        {2, 32, 70, "Tarea 2"},  // Periodo: 500 ms
        {3, 80, 60, "Tarea 3"}   // Periodo: 1250 ms, prioridad más baja
    };

    pthread_t rtc_thread;
    if (pthread_create(&rtc_thread, NULL, rtc_periodic_thread, &freq) != 0) {
        perror("Error al crear hilo RTC");
        exit(EXIT_FAILURE);
    }

    pthread_t th[3];
    pthread_attr_t attr;
    struct sched_param sp;

    for (int i = 0; i < 3; i++) {
        pthread_attr_init(&attr);

        // Establecer política de planificación SCHED_FIFO
        if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO) != 0) {
            perror("Error al establecer política SCHED_FIFO");
            exit(EXIT_FAILURE);
        }

        // Establecer prioridad RMS
        sp.sched_priority = tasks[i].priority;
        if (pthread_attr_setschedparam(&attr, &sp) != 0) {
            perror("Error al establecer prioridad del hilo");
            exit(EXIT_FAILURE);
        }

        // Fijar que los atributos de planificación sean utilizados
        if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
            perror("Error al aplicar atributos explícitos");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&th[i], &attr, periodic_task, &tasks[i]) != 0) {
            fprintf(stderr, "Error al crear hilo %d\n", i + 1);
            exit(EXIT_FAILURE);
        }

        pthread_attr_destroy(&attr);
    }

    sleep(20);  // Simulación por 20 segundos

    running = 0;

    pthread_mutex_lock(&tick_mutex);
    pthread_cond_broadcast(&tick_cond);
    pthread_mutex_unlock(&tick_mutex);

    pthread_join(rtc_thread, NULL);
    for (int i = 0; i < 3; i++) pthread_join(th[i], NULL);

    return 0;
}
