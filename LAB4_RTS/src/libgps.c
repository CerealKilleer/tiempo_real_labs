#include <stdio.h>
#include <math.h>
#include "libgps.h"
#include "synchronization_communication.h"
#define KM_H_TO_M_S(x) (((x) * 1000) / (60 * 60)) //Para convertir km/h a m/s
#define DEGRESS_TO_RADS(x) ((x) * (M_PI/180.0)) //Grados a radianes
#define METERS_PER_LATITUD_DEGREE 111320 //Este es un valor aproximado de dividir la circunferencia de la tierra entre 360 grados
#define METERS_PER_LONGITUDE_DEGREE(lat_deg) (111320.0 * cos((lat_deg) * M_PI / 180.0)) //Convierte la longitud a metros

/**
 * @brief Calcula la nueva posicion a partir de la ultima velocidad promedio calculada,
 * el intervalo de tiempo y la ultima posicion. Debe tenerse en cuenta
 * que dicha medicion no está teniendo en cuenta variaciones en la aceleración entre intervalos.
 * En una aplicación real, esta función haría una consulta al gps en lugar de calcular la nueva posición
 * a partir de la velocidad.
 * @param pos: informacion de la ultima posicion
 * @param avg_vel: velocidad promedio del vehiculo desde la ultima medicion
 * @param dt: intervalo de tiempo entre mediciones
 */
static void calculate_new_position(struct position *pos, float avg_vel, float dt)
{
        double d = KM_H_TO_M_S(avg_vel) * dt; //Desplazamiento simulado
        double rad = DEGRESS_TO_RADS(pos->heading_deg);
        double lat = (d * cos(rad)) / METERS_PER_LATITUD_DEGREE; //Se extrae la componente de latitud (norte) y se convierte metros
        double lon = (d * sin(rad)) / METERS_PER_LONGITUDE_DEGREE(pos->lat); //Cuanto se mueve hacia el este respecto de la latitud
        /*Note que el desplazamiento en longitud siempre es cero 
        porque nos estamos moviendo la norte, no al noreste o al noroeste, por ejemplo.*/

        pos->lat += lat;
        pos->lon += lon;
}
/**
 * @brief Esta funcion simula la posicion actual de un objeto
 * que se mueve a velocidad fija durante un intervalo de tiempo a partir de su ultima posicion
 * @param dt: intervalo de tiempo
 */

void show_position(float dt)
{
        static struct position pos = {
                .heading_deg = 0, //Para este caso el auto se mueve hacia el norte
                .lat = 6.335180965636349, //Latitud: Parque de Copacabana
                .lon = -75.55843405851274, //Longitud: Parque de Copacabana
        };
        
        /*La idea es obtener la nueva posicion, desplazandose hacia el norte
        Con la ultima velocidad promedio medida del auto*/

        //Hay que intentar tomar el mutex

        pthread_mutex_lock(&data_mutex);
        /* Pero no basta con tomarlo, hay que saber si hay una medicion valida
        Y en caso de que no, esperar hasta que la haya */

        while (!data.speed_sensor_ready) {
                pthread_cond_wait(&speed_sensor_cond, &data_mutex);
        }
        //Ahora, hay una medida valida de la velocidad. Se calcula la nueva posicion
        //Se obtiene una copia de la velocidad promedio actual y se libera el mutex para que alguien mas lo use
        float avg_vel = data.avg_vehicle_speed; //Se hace una copia del dato de interes
        pthread_mutex_unlock(&data_mutex); //Se libera rapidamente el mutex

        calculate_new_position(&pos, avg_vel, dt);

        PRINT_DATA_ARGS(stdout, "[show_position] Posicion Lat = %.5f, Lon = %.5f\n", pos.lat, pos.lon);
}