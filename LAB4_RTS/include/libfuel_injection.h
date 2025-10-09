#ifndef __LIBFUEL_INJECTION_H__
#define __LIBFUEL_INJECTION_H__
        #include <stdint.h>
        struct injector_data {
                uint32_t fuel;
                uint32_t fuel_inyected;
        };
        void injection(void);
#endif