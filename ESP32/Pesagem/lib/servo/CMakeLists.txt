set(srcs "src/iot_servo.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include")