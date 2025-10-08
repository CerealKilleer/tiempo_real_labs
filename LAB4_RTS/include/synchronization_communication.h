#ifndef __SYNCHRONIZATION_COMMUNICATION_H__
#define __SYNCHRONIZATION_COMMUNICATION_H__
        #include <pthread.h>
        #include <stdio.h>

        extern pthread_mutex_t data_mutex;
        extern pthread_mutex_t stdout_mutex;
        extern pthread_cond_t abs_cond;
        extern pthread_cond_t speed_sensor_cond;

        #define PRINT_DATA(stream, fmt) \
        do { \
                pthread_mutex_lock(&stdout_mutex); \
                fprintf((stream), (fmt)); \
                pthread_mutex_unlock(&stdout_mutex); \
        } while (0)

        #define PRINT_DATA_ARGS(stream, fmt, ...) \
        do { \
                pthread_mutex_lock(&stdout_mutex); \
                fprintf((stream), (fmt), __VA_ARGS__); \
                pthread_mutex_unlock(&stdout_mutex); \
        } while (0)

#endif