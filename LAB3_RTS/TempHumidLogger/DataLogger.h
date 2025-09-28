#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "SharedResources.h"
#include <thread>
#include <chrono>
#include <iostream>

/**
 * Este es el task del data logger, la idea es leer unicamente
 * cuando hayan elementos en las colas y ellas estén libres
 */
void data_logger_task() {
    std::string msg;
    while (true) {
        int temp = -1;
        int hum = -1;

        {
            //Antes de leer intentemos tomar el mutex
            std::unique_lock<std::mutex> lock(temp_mutex);
            /*
              Si lo podemos tomar, igual quedemos esperando si no hay datos en la cola.
              El predicado sirve para bloquear unicamente cuando la cola este vacia
              y desbloquear solamente cuando nos despierten y hayan elementos en la cola
            */
            temp_cv.wait(lock, []{ return !temp_queue.buffer.empty(); });

            temp = temp_queue.buffer.front();
            temp_queue.buffer.pop();
        }

        {
            //Lo mismo para aca pero con la humedad
            std::unique_lock<std::mutex> lock(hum_mutex);
            hum_cv.wait(lock, []{ return !humidity_queue.buffer.empty(); });

            hum = humidity_queue.buffer.front();
            humidity_queue.buffer.pop();
        }

        msg = "[Data Logger] Logged: Temp=" + std::to_string(temp) + "°C, Hum=" + std::to_string(hum) + "%";
        print_data(msg);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

#endif // DATA_LOGGER_H
