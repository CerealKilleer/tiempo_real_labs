#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include "SharedResources.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <cstdlib>

/**
 * task del sensor de humedad, se generarán los datos
 * y se agregarán a la cola cuando esta esté libre, posteriormente
 * se notificará a cualquier hilo que esté esperando el Mutex para leer los datos
 */
void humidity_sensor_task() {
    std::string msg;
    while (true) {
        int hum = 40 + rand() % 20;  // Simulated humidity
        /*
            Se crean Scopes para cada seccion critica.
            La idea es delimitarla claramente y aprovechar el hecho
            de que los mutex se liberan automaticamente al salir de ellas
        */
        {
            /*
            * Se controla el acceso a la cola de humedad
            */
            std::scoped_lock<std::mutex> lock(hum_mutex);
            if (humidity_queue.buffer.size() < humidity_queue.max_size) { //Pa respetar el tamaño maximo
                humidity_queue.buffer.push(hum);
                msg = "[Humidity Sensor] Produced: " + std::to_string(hum) + "%";
            } else {
                msg = "[Humidity Sensor] Humidity queue: FULL";
            }
            hum_cv.notify_one();
        }
        
        print_data(msg);

        std::this_thread::sleep_for(std::chrono::milliseconds(700));
    }
}

#endif // HUMIDITY_SENSOR_H
