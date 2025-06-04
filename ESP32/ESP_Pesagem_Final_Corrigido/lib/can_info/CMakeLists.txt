set(srcs "src/HX711.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include")