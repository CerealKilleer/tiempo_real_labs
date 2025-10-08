#ifndef __LIBSENSE_VEL_H__
#define __LIBSENSE_VEL_H__
    #include <stdbool.h>
    #define WHEELS_NUM 4

    struct speed_data {
        float wheel_speed[WHEELS_NUM];
        float break_pressure[WHEELS_NUM];
        float avg_vehicle_speed;
        bool stoped;
        bool abs_ready;
        bool speed_sensor_ready;
    };
    extern struct speed_data data;
    float simulate_speed(float break_pressure);
    void sense_speed(void);

#endif