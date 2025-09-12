#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>  
#include <valgrind/callgrind.h>

#define NUM_SENSORS 1000
#define READINGS_PER_SENSOR 100

int read_sensor_value() {
    return rand() % 1024;
}

std::vector<int>* generate_sensor_data() {
    std::vector<int>* readings = new std::vector<int>();
    for (int i = 0; i < READINGS_PER_SENSOR; ++i) {
        readings->push_back(read_sensor_value());
    }
    return readings;
}

double calculate_average(std::vector<int>* data) {
    double sum = 0;
    for (int i = 0; i <= data->size(); ++i) { 
        sum += (*data)[i];
    }
    return sum / data->size(); 
}

int main() {
    /*
     * Se desactivará la instrumentación para la inicialización del programa
     * Esta parte no es particularmente interesante.
     */
    CALLGRIND_STOP_INSTRUMENTATION;
    srand(time(0));

    std::vector<double> averages;
    /*
     * Esta sección del código es el programa principal. Aquí estarán las fugas de memoria o se llamarán a la función con error de desbordamiento,
     * es fundamental definir la instrumentación a partir de aquí para poder tener la observación global del programa.
     * Es la porción de interés. Las llamadas a las funciones se harán siempre a partir de aquí.
     */
    CALLGRIND_START_INSTRUMENTATION;
    for (int i = 0; i < NUM_SENSORS; ++i) {
        std::vector<int>* sensor_data = generate_sensor_data();
        double avg = calculate_average(sensor_data);
        averages.push_back(avg);

        usleep(1000);  // 1ms
    }


    std::cout << "Processed data from " << NUM_SENSORS << " sensors.\n";
    return 0;
}
