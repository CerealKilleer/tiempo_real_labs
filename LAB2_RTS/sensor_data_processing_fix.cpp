#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>  
#include <valgrind/callgrind.h>

#define NUM_SENSORS 1000
#define READINGS_PER_SENSOR 100

int read_sensor_value() {
    return rand() % 1024;
}

int* generate_sensor_data() {
    int *readings = new int[READINGS_PER_SENSOR]; //Dejemos de usar arreglos de la STL porque no son necesarios y agrega overhead
    for (int i = 0; i < READINGS_PER_SENSOR; ++i) {
        readings[read_sensor_value()];
    }
    return readings;
}

double calculate_average(int *data) {
    double sum = 0;
    for (int i = 0; i < READINGS_PER_SENSOR; ++i) { //Corregida la condición del bucle para eliminar el desbordamiento
        sum += data[i]; //Es incluso más legible que (*data)[]
    }
    return sum / READINGS_PER_SENSOR;
}

int main() {
    /*
     * Se desactivará la instrumentación para la inicialización del programa
     * Esta parte no es particularmente interesante.
     */
    CALLGRIND_STOP_INSTRUMENTATION;
    srand(time(0));

    int *averages = new int[NUM_SENSORS]; //¿Usar la STL para un arreglo que no se redimensiona? No, por favor. Igual podría haberlo creado en el stack porque tiene longitud fija.
    /*
     * Esta sección del código es el programa principal. Aquí estarán las fugas de memoria o se llamarán a la función con error de desbordamiento,
     * es fundamental definir la instrumentación a partir de aquí para poder tener la observación global del programa.
     * Es la porción de interés. Las llamadas a las funciones se harán siempre a partir de aquí.
     */
    CALLGRIND_START_INSTRUMENTATION;
    for (int i = 0; i < NUM_SENSORS; ++i) {
        int *sensor_data = generate_sensor_data();
        double avg = calculate_average(sensor_data);
        delete sensor_data; // Ahora la memoria apuntada por sensor_data se elimina y no se acumula entre iteraciones luego de corregir el assert
        averages[i] = avg; //Raw array superiority

        usleep(1000);  // 1ms
    }

    delete averages;

    std::cout << "Processed data from " << NUM_SENSORS << " sensors.\n";
    return 0;
}
