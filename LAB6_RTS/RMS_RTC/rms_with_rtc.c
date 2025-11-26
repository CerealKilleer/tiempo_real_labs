#define _GNU_SOURCE   // Extensiones GNU
#include <errno.h>    // Manejo de códigos de error.
#include <fcntl.h>    // open(), O_RDONLY, O_NONBLOCK, etc.
#include <linux/rtc.h>// Constantes e ioctls para controlar el RTC.
#include <pthread.h>  // Hilos, mutex, variables condicionales.
#include <sched.h>    // Políticas y prioridades del scheduler.
#include <stdio.h>    // Entrada y salida estandar.
#include <stdlib.h>   // exit, funciones generales
#include <string.h>   // Para manejar cadenas de texto
#include <sys/ioctl.h>// ioctl() para configurar el RTC.
#include <sys/mman.h> // mlockall
#include <unistd.h>   // sleep, close, funciones POSIX básicas.


//La estructura de información para los tasks
typedef struct {
    int id;
    unsigned int period_ticks;
    int priority;
    const char *name;
} task_info_t;

static volatile unsigned long tick_count = 0;
//Se inicializan las variables, estos macros permiten inicializar el mutex y la variable condicional de manera estatica sin el llamado a las funciones correspondientes.
static pthread_mutex_t tick_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t tick_cond = PTHREAD_COND_INITIALIZER;
static int running = 1;

/* Hilo que genera el clock periódico basado en el RTC */
static void *rtc_periodic_thread(void *arg) {
    //Esto identifica al RTC, en nuestro caso el punto de montaje sigue siendo /dev/rtc0
    const char *dev = "/dev/rtc0";
    //Castea la frecuencia que se envía como parámetro a este hilo
    unsigned int freq = *(unsigned int *)arg;
    //Sirve para almacenar la data de la interrupción.
    unsigned long data;

    //Lo dicho en el caso anterior: en unix todo es un archivo y una forma de accederlo es por medio de un descriptor
    int fd = open(dev, O_RDONLY);
    if (fd < 0) {
        perror("Error abriendo el RTC");
        exit(EXIT_FAILURE);
    }    

    //Esta línea configura la frecuencia (freq) de generación de interrupciones. Sin embargo, el DS3231 nunca conecta sus pines SQW y 32K a la raspberry.
    //Con lo cual, estas interrupciones son generadas por software (el kernel)
    if (ioctl(fd, RTC_IRQP_SET, freq) < 0) {
        perror("Error al configurar frecuencia del RTC");
        exit(EXIT_FAILURE);
    }
    //Esto habilita las interrupciones a la frecuencia que fueron configuradas
    if (ioctl(fd, RTC_PIE_ON, 0) < 0) {
        perror("Error al habilitar interrupciones periódicas RTC");
        exit(EXIT_FAILURE);
    }
    //Esto se ejecuta durante unos 20 segundos
    while (running) {
        //Se trata de leer el controlador. Esta operación es bloqueante, así que este hilo se suspenderá hasta que se haya generado una interrupción.
        //Cuando haya una interrupción se leerá el archivo (desbloqueando a este hilo) que representa al dispositivo.
        read(fd, &data, sizeof(data));
        //Como los otros hilos podrían estar tratando de leer a la vez el tick_count, hay que serializar su acceso para actualizarlo.
        pthread_mutex_lock(&tick_mutex);
        //Luego de tomado el mutex, se aumenta el contador de ticks.
        tick_count++;
        //Y como se contó un tick, hay que avisarle a toooodos los hilos que puedan estar suspendidos por una variable condicional para que evalúen si luego de este tick deben ejecutarse.
        //Por eso el broadcast
        pthread_cond_broadcast(&tick_cond);
        //Luego de notificados todos los demás, entonces se libera el mutex
        pthread_mutex_unlock(&tick_mutex);
    }
    //Luego de que pasen esos 20 segundos hay que deshabilitar las interrupciones para no dejar al kernel generando eventos.
    ioctl(fd, RTC_PIE_OFF, 0);
    //Se cierra el archivo.
    close(fd);
    return NULL;
}

/* Hilo de tarea periódica controlada por ticks */
static void *periodic_task(void *arg) {
    //Esta es la función de entrada para todas las tareas. Cada tarea recibe como void * a su task_info_t
    task_info_t *info = (task_info_t *)arg;
    //Esta variable almacena los ticks hasta la próxima ejecución de la tarea
    unsigned long next_release;
    //Vamos a leer tick_count para definir el next_release y se intenta tomar el mutex.
    pthread_mutex_lock(&tick_mutex);
    //Luego de tomado el mutex, cabe la posibilidad de que esto se haga luego de que se hayan contado algunos ticks, entonces
    //La primera inicialización de la tarea debe tener en cuenta este desfase.
    next_release = tick_count + info->period_ticks;
    //Se libera el mutex
    pthread_mutex_unlock(&tick_mutex);

    //Y la tarea empieza a correr durante unos 20 segundos
    while (running) {
        //Esto es interesante y creo que nosotros usamos algo parecido en la práctica 4.
        //La tarea se bloquea hasta que ocurra un tick y siempre que no se haya alcanzado su next_release

        //Trata de tomar el mutex del tick
        pthread_mutex_lock(&tick_mutex);
        //Cuando lo logra, valida inmediatamente si es hora de ejecutar su cuerpo. Esto lo hace en caso de que tick_count >= next_release. En tiempo real, esperamos que sea igual.

        while (tick_count < next_release)
            //En caso de que no sea hora de ejecutar el cuerpo de la tarea, se queda esperando con una variable condicional hasta que rtc_periodic_thread notifique que pasó un tick.
            //En ese caso, y si no ha sido relevado por un hilo de mayor prioridad, el hilo se despierta y valida si ese tick significa que es tiempo de ejecutar su cuerpo.
            //Por eso el while, a ese while se le conoce también como el predicado, sirve para evitar activaciones espúreas de los hilos. En caso de que la condición vuelva a cumplirse
            //La tarea se duerme hasta el próximo tick y continuará así hasta que sea hora de ejecutarse.
            pthread_cond_wait(&tick_cond, &tick_mutex);

        //Cuando al final es hora de despertarse se debe actualizar el próximo release de la tarea
        next_release += info->period_ticks;

        //Liberarse el mutex, este mutex podría haberse liberado una línea antes porque next_release e info->period_ticks son locales al hilo.
        pthread_mutex_unlock(&tick_mutex);

        //El cuerpo de la tarea es mostrar este printf
        printf("[tick %lu] Ejecutando %s (periodo: %u ticks)\n",
               tick_count, info->name, info->period_ticks);
        //Y se hace esta espera durante 50ms para simular un tiempo de ejecución adicional de este valor. Todas las tareas tienen un deadline de menos de 50ms.
        usleep(50000);
    }
    return NULL; //Cuando la tarea termine retornará NULL indicando que todo bien
}

int main(void) {
    /*
     * Los sistemas en tiempo real necesitan determinismo en su ejecución. Cuando una pagina no estaba en memoria ram y debe ser buscada en
     * el sistema de archivos se agrega una penalización en el tiempo que altera el WCET.
     * Por eso, estos sistemas usualmente buscan  que no haya intercambio de páginas entre ram y disco.
    */

    //mlockall hace que todos los datos que están y que estarán en el espacio de memoria de este proceso se queden en RAM, evitando penalizaciones al tratar de utilizar un dato que no esté en RAM sino en disco
    //Naturalmente, lo hace evitando que haya swap entre RAM y disco.

    mlockall(MCL_CURRENT | MCL_FUTURE);  // Evita fallos de página
    unsigned int freq = 64;  // Frecuencia del RTC en Hz → cada tick ≈ 15.6ms

    //Los datos de las tareas {id, periodo en ticks, prioridad según RMS y un nombre}
    task_info_t tasks[3] = {
        {1, 16, 80, "Tarea 1"},  // Periodo: 250 ms, prioridad más alta
        {2, 32, 70, "Tarea 2"},  // Periodo: 500 ms
        {3, 80, 60, "Tarea 3"}   // Periodo: 1250 ms, prioridad más baja
    };

    //Se crea el primer hilo que controla los ticks del RTC y se valida correctamente.
    pthread_t rtc_thread;
    if (pthread_create(&\, NULL, rtc_periodic_thread, &freq) != 0) {
        perror("Error al crear hilo RTC");
        exit(EXIT_FAILURE);
    }

    //Ahora se crean los tres hilos para las tares tareas anteriores.
    pthread_t th[3];
    pthread_attr_t attr;                                            //Permite definir los atributos propios de los hilos
    struct sched_param sp;                                          //Permite definir las características del scheduler asociado a los hilos

    for (int i = 0; i < 3; i++) {
        //Se reserva memoria para la estructura de los atributos de los hilos
        pthread_attr_init(&attr);

        // Se establece la política a seguir por el hilo. En este caso se ejecutará libremente hasta que se bloquee, termine o sea relevado por otro hilo de mayor prioridad.
        if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO) != 0) {
            perror("Error al establecer política SCHED_FIFO");
            exit(EXIT_FAILURE);
        }

        // Establecer prioridad según RMS para el hilo. Nótese que lo que lo hace RMS es la unión entre SCHED_FIFO y las prioridades asignadas inicialmente.
        sp.sched_priority = tasks[i].priority;
        //Se asignan estos parámetros básicos de scheduling al hilo
        if (pthread_attr_setschedparam(&attr, &sp) != 0) {
            perror("Error al establecer prioridad del hilo");
            exit(EXIT_FAILURE);
        }

        // Importante: por defecto los hilos heredan la política de planificación del padre, así que si esto no se hace las configuraciones anteriores no tienen efecto.
        if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
            perror("Error al aplicar atributos explícitos");
            exit(EXIT_FAILURE);
        }
        //Se crea cada hilo.
        if (pthread_create(&th[i], &attr, periodic_task, &tasks[i]) != 0) {
            fprintf(stderr, "Error al crear hilo %d\n", i + 1);
            exit(EXIT_FAILURE);
        }
        //Se liberan los recursos anteriormente utilizados
        pthread_attr_destroy(&attr);
    }

    sleep(20);  // Simulación por 20 segundos
    //Esta variable se hace cero y rompe todos los ciclos que dependen de ella
    running = 0;

    //Si todavía queda por ahí algún hilo bloqueado esperando el boardcast se despierta para su última ejecución antes de terminar ejecución.
    pthread_mutex_lock(&tick_mutex);
    pthread_cond_broadcast(&tick_cond);
    pthread_mutex_unlock(&tick_mutex);

    //Se hace join con todos los hilos para que el main no termine sin que los demás hilos hayan finalizado su trabajo
    pthread_join(rtc_thread, NULL);
    for (int i = 0; i < 3; i++) pthread_join(th[i], NULL);

    return 0;
}
