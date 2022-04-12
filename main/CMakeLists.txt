
file(GLOB_RECURSE SOURCES views/src/*.c ../assets/images/converted/*.c)


idf_component_register(SRCS ${SOURCES} main.c
                        INCLUDE_DIRS . views/inc
                        )
