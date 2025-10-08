#include <stdint.h>
#include <stdio.h>
#include "libsense_vel.h"
#include "libABS_breaks.h"
#include "synchronization_communication.h"

/**
 * @brief funcion que simula a un controlador de frenos ABS
 * verifica si alguna de las ruedas está en riesgo de bloquearse.
 * En caso de estarlo libera un poco el freno
 */
void control_abs_breaks(void)
{
        pthread_mutex_lock(&data_mutex);

        while (!data.speed_sensor_ready) {
                pthread_cond_wait(&speed_sensor_cond, &data_mutex);
        }
        data.abs_ready = false;

        if (data.stoped) {
                PRINT_DATA(stdout, "[control_abs_breaks]: Vehiculo detenido\n");
                data.abs_ready = true;
                pthread_cond_signal(&abs_cond);
                pthread_mutex_unlock(&data_mutex);
                return;
        }
        /*
        Implementacion de la funcionalidad basica del ABS
        Si alguna de las ruedas está en riesgo de bloquearse,
        se libera lentamente el freno
        */
        for (uint8_t i = 0; i < WHEELS_NUM; i++) {
                float slip = (data.avg_vehicle_speed - data.wheel_speed[i]) / data.avg_vehicle_speed; //¿Cuanto difiere la velocidad de cada rueda respecto a la del vehiculo?
                if (slip < SLIP_TH) {
                        //La rueda no está en riesgo de bloquearse
                        data.break_pressure[i] += 0.05;
                        PRINT_DATA_ARGS(stdout, "[control_abs_breaks]: La rueda %d no está en riesgo de bloquearse. Aumentando presión de frenado.\n", i + 1);
                } else {
                        //Si se está en riesgo de bloquearse hay que disminuir la presión
                        data.break_pressure[i] -= 0.05;
                        PRINT_DATA_ARGS(stdout, "[control_abs_breaks]: La rueda %d está en riesgo de bloquearse. Disminuyendo presión de frenado\n", i + 1);
                }
        }
        data.abs_ready = true;
        pthread_cond_signal(&abs_cond);
        pthread_mutex_unlock(&data_mutex);
}