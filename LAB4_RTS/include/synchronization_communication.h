#ifndef __SYNCHRONIZATION_COMMUNICATION_H__
#define __SYNCHRONIZATION_COMMUNICATION_H__
        #include <pthread.h>
        #include <stdio.h>
        #include <stdbool.h>
        #define WHEELS_NUM 4 //Numero de ruedas en el carro
        
        struct speed_data { //estructura para almacenar los datos de velocidad del auto y su estado
                float wheel_speed[WHEELS_NUM]; //Una posicion para la velocidad de cada rueda
                float break_pressure[WHEELS_NUM]; //Una posicion para la presión de los frenos en cada rueda
                float avg_vehicle_speed; //Velocidad promedio del auto
                bool stoped; //Dice si el auto esta detenido o no
                bool abs_ready; //Dice si la tarea abs actualizó la presión de los frenos o no
                bool speed_sensor_ready; //Dice si la tarea del sensor de velocidad tiene una medida completa válida o no
        };

        extern pthread_mutex_t data_mutex; //Para serializar el acceso al speed_data
        extern pthread_mutex_t stdout_mutex; //Para serializar el acceso al buffer de la salida estándar
        extern pthread_cond_t abs_cond; //Para notificar a la tarea del sensor de velocidad cuando el control abs ha modificado la presión de frenado
        extern pthread_cond_t speed_sensor_cond; //Para notificar al abs, la inyeccion, y el sensor de posicion cuando hay una medida de velocidad anterior
        extern struct speed_data data; //Para almacenar la velocidad actual del auto
        
        //Estos dos macros permiten escribir de manera segura en la salida estándar usando el mutex
        #define PRINT_DATA(stream, fmt) \
        do { \
                pthread_mutex_lock(&stdout_mutex); \
                fprintf((stream), (fmt)); \
                pthread_mutex_unlock(&stdout_mutex); \
        } while (0)
        //Este es un macro variadico pues recibe cualquier numero de parametros adicionales
        //Es util para escribir en la salida con formatos mas complejos
        #define PRINT_DATA_ARGS(stream, fmt, ...) \
        do { \
                pthread_mutex_lock(&stdout_mutex); \
                fprintf((stream), (fmt), __VA_ARGS__); \
                pthread_mutex_unlock(&stdout_mutex); \
        } while (0)

#endif