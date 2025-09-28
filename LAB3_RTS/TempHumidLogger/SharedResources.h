#ifndef SHARED_RESOURCES_H
#define SHARED_RESOURCES_H

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string> // Libreria string para utilizar la funcion centralizada de imprimir

struct SharedQueue {
    std::queue<int> buffer;
    const size_t max_size = 10;
};

/*
    La palabra reservada extern indica que
    las variables indicadas aqui pueden ser encontradas 
    por cualquier otro archivo que incluya este .h
*/
extern SharedQueue temp_queue;
extern SharedQueue humidity_queue;
extern std::mutex temp_mutex;
extern std::mutex hum_mutex;
extern std::mutex std_mutex;
extern std::condition_variable temp_cv;
extern std::condition_variable hum_cv;

/**
 * Esta funcion proporciona una interfaz unica para usar la salida estandar
 * es thread safe
 */
void print_data(const std::string &mensaje) {
    //Se centraliza completamente el uso del recurso compartido del buffer de salida
    std::scoped_lock<std::mutex> lock(std_mutex);
    std::cout << mensaje << std::endl;
}

#endif // SHARED_RESOURCES_H
