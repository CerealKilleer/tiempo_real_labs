#ifndef __LIBGPS_H__
#define __LIBGPS_H__
        #include <stdint.h>
        struct position {
                double lat; //Latitud
                double lon; //Longitud
                uint8_t heading_deg; //Angulo de desplazamiento
        };

        
        void show_position(float dt);
#endif