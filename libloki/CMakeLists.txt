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

include_directories(mbedtls)

IF(BUILD_TESTING)
# TODO: add unit test definitions?
ENDIF(BUILD_TESTING)

include_directories(. ../src ../http ../crypto ../app ../json)

set(loki_SRCS
crypto_provider_mbedtls.c
pow.c
lock.c
)

add_library(loki OBJECT ${loki_SRCS})

# Add -fPIC flag
if(BUILD_SHARED_LIBS)
    set_property(TARGET mbedtls PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
