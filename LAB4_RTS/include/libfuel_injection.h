#ifndef __LIBFUEL_INJECTION_H__
#define __LIBFUEL_INJECTION_H__
        #include <stdint.h>
        #define FUEL_INYECTED 1000 //Cantidad de combustible que se inyecta cada vez
        #define MAX_FUEL 1000000 //Tanque maximo
        struct injector_data {
                uint32_t fuel; //Cantidad total de combustible
                uint32_t fuel_inyected; //Cantidad total inyectada
        };
        void injection(void);
#endif