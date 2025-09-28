#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "SharedResources.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <cstdlib>

/**
 * task del sensor de temperatura, se generarán los datos
 * y se agregarán a la cola cuando esta esté libre, posteriormente
 * se notificará a cualquier hilo que esté esperando el Mutex para leer los datos
 */
void temperature_sensor_task() {
    std::string msg;
    while (true) {
        int temp = 20 + rand() % 10;  // Simulated temperature
        //Se crearán scopes para cada sección crítica
        {
            /* Seccion critica para la cola. 
            Hay que evitar accesos invalidos y sincronizar al productor y al consumidor*/
            std::scoped_lock<std::mutex> lock(temp_mutex);
            if (temp_queue.buffer.size() < temp_queue.max_size) { //Vamos a respetar el maximo tamaño definido en el struct
                temp_queue.buffer.push(temp);
                msg = "[Temp Sensor] Produced: " + std::to_string(temp) + "°C";
            } else {
                msg = "[Temp Sensor] Temp queue: FULL";
            }
            temp_cv.notify_one(); //Como solo hay un consumidor, vamos a despertarlo si está bloqueado
        }
    
        print_data(msg);

        //Aqui ya todos los mutex deberan estar liberados
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

#endif // TEMPERATURE_SENSOR_H
