#ifndef __APP_H__
#define __APP_H__
        #include "utilities.h"
        #define MAX_THREADS 4 //Numero maximo de hilos
        //Macros que definen los offset y los periodos de las tareas
        #define OFFSET_SPEED_SENSOR_MS 1000
        #define PERIOD_SPEED_SENSOR_MS 20
        #define OFFSET_ABS_CONTROL_MS 5000
        #define PERIOD_ABS_CONTROL_MS 40
        #define OFFSET_FUEL_INJECTION_MS 1000
        #define PERIOD_FUEL_INJECTION_US 12500
        #define OFFSET_POSITION_SENSOR_MS 1000
        #define PERIOD_POSITION_SENSOR_MS 200
        
        #define TIMESPEC_TO_MS(timespec) SECS_TO_MS(timespec.tv_sec) + 
        #define TIMESPEC_DIFF_NS(start, stop) ((stop.tv_nsec) + SEC_TO_NS(stop.tv_sec)) - ((start.tv_nsec) + SEC_TO_NS(start.tv_sec))
        #define TIMESPEC_DIFF_MS(start, stop) NS_TO_MS((float)(TIMESPEC_DIFF_NS(start, stop)))
        int init_app(void);
#endif