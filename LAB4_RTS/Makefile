CROSS_COMPILE =
CC = ${CROSS_COMPILE}gcc
AR = ar
CFLAGS = -O1 -Wall -Werror -Wpedantic -I${INCLUDE_DIR}
LDFLAGS = -lpthread -lrt -lm -L${BUILD_DIR} -lcar_control
INCLUDE_DIR = ./include
SOURCE_DIR = ./src
BUILD_DIR = ./build
LIBABS_BREAKS_OBJ = ${BUILD_DIR}/libabs_breaks.o
LIBSENSE_VELOCITY_OBJ = ${BUILD_DIR}/libsense_velocity.o
LIBREPEATING_TIMERS_CLOCKS_OBJ = ${BUILD_DIR}/librepeating_timers_clocks.o
LIBFUEL_INJECTION_OBJ = ${BUILD_DIR}/libfuel_injection.o
LIBGPS_OBJ = ${BUILD_DIR}/libgps.o
APP_OBJ = ${BUILD_DIR}/app.o
MAIN_OBJ = ${BUILD_DIR}/main.o
LIBCAR_CONTROL =  ${BUILD_DIR}/libcar_control.a
OUTPUT = ${BUILD_DIR}/practica_4

all: ${MAIN_OBJ} ${APP_OBJ} ${LIBCAR_CONTROL}
	mkdir -p ${BUILD_DIR}
	${CC} ${CFLAGS} $^ ${LDFLAGS} -o ${OUTPUT} 

${MAIN_OBJ}: ${SOURCE_DIR}/main.c
	mkdir -p ${BUILD_DIR}
	${CC} -c ${CFLAGS} $< -o $@

${APP_OBJ}: ${SOURCE_DIR}/app.c
	mkdir -p ${BUILD_DIR}
	${CC} -c ${CFLAGS} $< -o $@

${LIBSENSE_VELOCITY_OBJ}: ${SOURCE_DIR}/libsense_vel.c
	mkdir -p ${BUILD_DIR}
	${CC} -c ${CFLAGS}  $< -o $@

${LIBABS_BREAKS_OBJ}: ${SOURCE_DIR}/libABS_breaks.c
	mkdir -p ${BUILD_DIR}
	${CC} -c ${CFLAGS}  $< -o $@

${LIBREPEATING_TIMERS_CLOCKS_OBJ}: ${SOURCE_DIR}/librepeating_timers_clocks.c
	mkdir -p ${BUILD_DIR}
	${CC} -c ${CFLAGS} $< -o $@

${LIBFUEL_INJECTION_OBJ}: ${SOURCE_DIR}/libfuel_injection.c
	mkdir -p ${BUILD_DIR}
	${CC} -c ${CFLAGS} $< -o $@

${LIBGPS_OBJ}: ${SOURCE_DIR}/libgps.c
	mkdir -p ${BUILD_DIR}
	${CC} -c ${CFLAGS} $<  -o $@

${LIBCAR_CONTROL}: ${LIBSENSE_VELOCITY_OBJ} \
					${LIBABS_BREAKS_OBJ} \
					${LIBREPEATING_TIMERS_CLOCKS_OBJ} \
					${LIBFUEL_INJECTION_OBJ} \
					${LIBGPS_OBJ}
	${AR} rcs $@ $^

clean:
	rm -rf ./build
