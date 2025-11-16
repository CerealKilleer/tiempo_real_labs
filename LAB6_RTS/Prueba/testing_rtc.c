#include <stdio.h> //Entradas y salidas estándar, aquí se usa para printf
#include <fcntl.h> //file control options para usar open y el macro O_RDONLY
#include <unistd.h> //Para close, me causa gracia que open esté en fcntl.h y close en unistd.h, ¿por qué no las tienen juntas?
#include <sys/ioctl.h> //Aquí están los archivos de cabecera para la función ioctl
#include <linux/rtc.h> //Entrega la interfaz para los drivers de los relojes en tiempo real, mira que está debajo de /linux, es propio del kernel :)

int main() {
    //<! El punto de montaje del RTC es en el directorio /dev/ (devices) y, como todo en Linux es un archivo, pues se accede a el como un archivo de nombre rtc0
    const char *dev = "/dev/rtc0";

    /*<! Los archivos de texto o binarios se abren con la llamada al sistema Open, en este caso, el RTC se abre en modo de solo lectura (porque lo que se va a hacer es leer).
     * Cuando se abre un archivo con Open se crea un File Descriptor (fd), esto es simplemente un número entero que identifica al archivo abierto. Si todo sale bien el valor es no
     * negativo, aquí se trata de abrir el archivo que representa el rtc y se valida si se pudo abrir correctamente */*
    int fd = open(dev, O_RDONLY);
    if (fd < 0) {
        perror("Error abriendo el RTC");
        return 1;                           //<! Si algo falla se retorna al SO un valor de error. Cualquier número distinto a 0 que se retorne desde el main, es un error para el sistema operativo.
    }

    /*<! Esta estructura permite almacenar los campos que retorna una medición del rtc: segundos, minutos, horas, dia del mes, mes, año. El dia de la semana, del año y la bandera que
     *indica si se debe adelantar o no una hora deacuerdo al tiempo de verano o invierno, no se utilizan*/*
    struct rtc_time rtc;

    /*<! Algunos archivos abiertos como el del RTC pueden recibir comandos para realizar ciertas acciones por medio de parámetros especiales, ioctl permite realizar eso. En este caso hacer una lectura de la marca de
     tiempo y almacenarla en la estructura anterior. Si retorna un valor negativo esto indica que ocurrió un error.*/*
    if (ioctl(fd, RTC_RD_TIME, &rtc) < 0) {
        perror("Error leyendo el RTC (ioctl)");
        close(fd); //Claramente hay que cerrar el archivo abierto, porque consume memoria. Es una buena práctica aunque luego del return igual se libera.
        return 2;
    }

    //Aquí se formatea la salida así: yyyy-mm-yy hh:mm:ss con un salto de linea al final pa que quede más bonito
    printf("(%s) Hora RTC DS3231: %04d-%02d-%02d %02d:%02d:%02d\n",
           dev,
           rtc.tm_year + 1900, //Tengo dudas sobre esto porque el registro de año en el ds3231 va de 00-99, realmente debería ser 2000
           rtc.tm_mon  + 1, //Igual aquí, el registro del mes va de 1-12
           rtc.tm_mday,
           rtc.tm_hour, rtc.tm_min, rtc.tm_sec);

    close(fd);
    return 0; //Todo salió bien :)
}
