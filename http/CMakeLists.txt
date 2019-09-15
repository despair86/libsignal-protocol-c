IF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable -Wno-unused-function -Wno-shadow")
ENDIF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")

IF(CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-compare")
    IF(GCC_WARN_SIGN_CONVERSION)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-conversion")
    ENDIF(GCC_WARN_SIGN_CONVERSION)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(CMAKE_C_COMPILER_ID MATCHES "Clang")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shorten-64-to-32")
ENDIF(CMAKE_C_COMPILER_ID MATCHES "Clang")

include_directories(. ../src ../crypto)

set(http_SRCS
http.c
chunk.c
header.c
client.c
)

add_library(http OBJECT ${http_SRCS})

# Add -fPIC flag
if(BUILD_SHARED_LIBS)
    set_property(TARGET http PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
