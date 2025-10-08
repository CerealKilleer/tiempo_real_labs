#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "libsense_vel.h"
#include "synchronization_communication.h"

/**
 * @brief Simula la velocidad cuando se aplica una presion de frenado y modela una suerte de desbalance
 * de velocidad entre las ruedas. Cuando se llega a una velocidad promedio cercana a cero se asume que
 * el vehiculo está completamente detenido
 * @param break_pressure un valor flotante que representa la presion de frenado.
 * @return la velocidad simulada de una rueda
 */
float simulate_speed(float break_pressure)
{
        //Si el vehiculo esta detenido asi se queda
        if ((data.stoped)) {
                return 0.0;
        }

        //Se modela la velocidad de una llanta con cierto desnivel entre las ruedas
        float speed = 80.0 - (break_pressure * 20) + ((rand() % 100) / 10);

        if (speed < 0.0) {
                speed = 0.0;
        }

        return speed;
}

/**
 * @brief Implementa el sensor basico de velocidad. Lee el buffer de velocidades para cada rueda
 * y determina cuando el auto está detenido
 */
void sense_speed(void)
{
        float speed;
        
        pthread_mutex_lock(&data_mutex);

        while (!data.abs_ready) {
                pthread_cond_wait(&abs_cond, &data_mutex);
        }

        data.speed_sensor_ready = false;
        data.avg_vehicle_speed = 0.0;
        for (uint8_t i = 0; i < WHEELS_NUM; i++) {
                speed = simulate_speed(data.break_pressure[i]);
                data.wheel_speed[i] = speed;
                data.avg_vehicle_speed += speed;
                PRINT_DATA_ARGS(stdout, "[sense_speed] rueda #%d: %.2f km/h\n", i+1, speed);
        }

        data.avg_vehicle_speed /= WHEELS_NUM;

        if (data.avg_vehicle_speed < 0.01) {
                data.stoped = true;
        }

        data.speed_sensor_ready = true;
        pthread_cond_signal(&speed_sensor_cond);
        pthread_mutex_unlock(&data_mutex);
}