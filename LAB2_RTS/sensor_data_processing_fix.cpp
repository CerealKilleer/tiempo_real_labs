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
    std::vector<int>* readings = new std::vector<int>(); //Aquí se crea explícitamente en le heap
    for (int i = 0; i < READINGS_PER_SENSOR; ++i) {
        readings->push_back(read_sensor_value());
    }
    return readings;
}

double calculate_average(std::vector<int>* data) {
    double sum = 0;
    for (int i = 0; i < data->size(); ++i) { //Corregida la condición del bucle para eliminar el desbordamiento
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

    std::vector<double> averages; //Este amiguito se crea en el stack, por eso no se libera. Cuando se hacen push_backs estos sí usan el heap, pero al llamarse al destructor de averages todo esto se libera, de ahí que no cuente como memory leak
    /*
     * Esta sección del código es el programa principal. Aquí estarán las fugas de memoria o se llamarán a la función con error de desbordamiento,
     * es fundamental definir la instrumentación a partir de aquí para poder tener la observación global del programa.
     * Es la porción de interés. Las llamadas a las funciones se harán siempre a partir de aquí.
     */
    CALLGRIND_START_INSTRUMENTATION;
    for (int i = 0; i < NUM_SENSORS; ++i) {
        std::vector<int>* sensor_data = generate_sensor_data();
        double avg = calculate_average(sensor_data);
        delete sensor_data; // Ahora la memoria apuntada por sensor_data se elimina y no se acumula entre iteraciones luego de corregir el assert
        averages.push_back(avg);

        usleep(1000);  // 1ms
    }


    std::cout << "Processed data from " << NUM_SENSORS << " sensors.\n";
    return 0;
}
