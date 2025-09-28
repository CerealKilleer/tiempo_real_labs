#include "TemperatureSensor.h"
#include "HumiditySensor.h"
#include "DataLogger.h"
#include "SharedResources.h"
#include <thread>

/*
    Estas son todas variables globales.
    La intención es que puedan utilizadas desde todos los archivos
*/

SharedQueue temp_queue;
SharedQueue humidity_queue;
std::mutex temp_mutex;
std::mutex hum_mutex;
std::mutex std_mutex;
std::condition_variable temp_cv;
std::condition_variable hum_cv;

int main() {
    //Se crean los tres hilos adicionales al main
    std::thread temperature(temperature_sensor_task);
    std::thread humidity(humidity_sensor_task);
    std::thread Logger(data_logger_task);

    temperature.join();
    humidity.join();
    Logger.join();

    return 0;
}
