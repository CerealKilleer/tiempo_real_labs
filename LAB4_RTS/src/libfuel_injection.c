#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include "libfuel_injection.h"
#include "synchronization_communication.h"

struct injector_data injector_data = {
        .fuel = UINT32_MAX,
        .fuel_inyected = 0
};


void injection(void)
{
        if (injector_data.fuel == 0) {
                PRINT_DATA(stdout, "[Inyector de combustible]: No hay combustible disponible\n");
                return;
        }
        
        pthread_mutex_lock(&data_mutex);
        while (!data.speed_sensor_ready) {
                pthread_cond_wait(&speed_sensor_cond, &data_mutex);
        }

        if (data.stoped) {
                PRINT_DATA(stdout, "[Inyector de combustible]: Vehiculo detenido\n");
                pthread_mutex_unlock(&data_mutex);
                return;
        }

        injector_data.fuel_inyected += 1000000;
        injector_data.fuel -= 1000000;

        if (injector_data.fuel == 0) {
                data.stoped = true;
                data.avg_vehicle_speed = 0.0;
                memset(data.wheel_speed, 0, WHEELS_NUM*sizeof(float));
                PRINT_DATA(stdout, "[Inyector de combustible]: Se ha terminado el combustible\n");
        } else {
                PRINT_DATA(stdout, "[Inyector de combustible]: Combustible inyectado\n");
        }
        pthread_mutex_unlock(&data_mutex);
}