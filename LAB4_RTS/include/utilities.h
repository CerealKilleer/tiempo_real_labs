#ifndef __UTILITIES_H__
#define __UTILITIES_H__
        //Macros para la conversion de tiempos
        #define NS_PER_MS (1000000)
        #define NS_PER_SEC (1*1000*1000000)
        #define SEC_TO_NS(x) ((x) * NS_PER_SEC)
        #define NS_TO_MS(x) ((x) / NS_PER_MS)
        #define US_TO_NS(x) ((x) * 1000)
        #define SECS_PER_NS(x) ((x) / NS_PER_SEC)
        #define REMAIN_NS_FROM_SECS(x) ((x) % NS_PER_SEC)
        #define US_PER_SEC (1000000)
        #define US_TO_SECS(x) ((x) / US_PER_SEC)
        #define NS_PER_US (1000)
        #define REMAIN_NS_FROM_US(x) (((x) % US_PER_SEC) * NS_PER_US)
        #define MS_PER_SEC (1000)
        #define SECS_TO_MS(x) ((x) * MS_PER_SEC)
        #define SECS_PER_MS(x) ((x) / MS_PER_SEC)
        #define MS_TO_US(x) ((x) * MS_PER_SEC)
        #define MS_TO_S(x) ((x) / MS_PER_SEC)
#endif