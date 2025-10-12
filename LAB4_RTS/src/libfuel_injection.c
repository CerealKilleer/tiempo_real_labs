#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include "libfuel_injection.h"
#include "synchronization_communication.h"

struct injector_data injector_data = {
        .fuel = MAX_FUEL,
        .fuel_inyected = 0
};

/**
 * @brief Simula un sistema de inyección de combustible básico
 * Inyecta combustible si está disponible, si el combustible se termina levanta banderas
 * indicando que el auto se detenderá. Depende de que hayan mediciones válidas de velocidad en el sistema
 * para saber si inyectar o no combustible y para ello usa mutex y la variable condicional speed_sensor_cond.
 */
void injection(void)
{
        //Se intenta tomar el mutex de los datos de velocidad. Solo se inyectará si el auto está en movimiento
        pthread_mutex_lock(&data_mutex);
        //Se usa la variable condicional para garantizar accesos solo cuando hayan valores válidos del sensor de velocidad
        while (!data.speed_sensor_ready) {
                pthread_cond_wait(&speed_sensor_cond, &data_mutex);
        }
        //Si el auto está detenido no hay nada que inyectar
        if (data.stoped) {
                PRINT_DATA_ARGS(stdout, "[Inyector de combustible]: Vehiculo detenido. Combustible restante %d\n", injector_data.fuel);
                pthread_mutex_unlock(&data_mutex); //Se libera el mutex
                return;
        }
        
        //Antes de intentar inyectar combustible se valida si hay disponible
        if (injector_data.fuel == 0) {
                PRINT_DATA(stdout, "[Inyector de combustible]: No hay combustible disponible\n");
                pthread_mutex_unlock(&data_mutex); //Se libera el mutex
                return;
        }

        //Si el auto está en movimiento se inyecta combustible
        injector_data.fuel_inyected += FUEL_INYECTED;
        injector_data.fuel -= FUEL_INYECTED;

        //Si luego de inyectarse, se acaba el combustible, el carro se detiene.
        //En la vida real seguramente entran a jugar otros elementos, pero es suficiente para esta desmotracion
        if (injector_data.fuel == 0) {
                data.stoped = true; //Se detiene el auto
                data.avg_vehicle_speed = 0.0; //Nueva velocidad promedio
                memset(data.wheel_speed, 0, WHEELS_NUM*sizeof(float)); //Todas las ruedas se ponen en cero
                PRINT_DATA(stdout, "[Inyector de combustible]: Se ha terminado el combustible\n");
        } else {
                PRINT_DATA(stdout, "[Inyector de combustible]: Combustible inyectado\n");
        }
        pthread_mutex_unlock(&data_mutex); //Se libera el mutex
}