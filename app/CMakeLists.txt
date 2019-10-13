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

set(LIBS ${LIBS}
	${M_LIB}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS}
	signal-protocol-c
	cdk
)

if(WIN32)
set(LIBS ${LIBS}
	ws2_32
)
endif()

if (NOT WIN32 OR USE_SEPARATE_CURSES)
set(LIBS ${LIBS} curses)
endif()

include_directories(. ../src ../http ../crypto ../json)

set(loki_pager_app_SRCS
main.c
crypto_provider_mbedtls.c
loki.c
app.c
mnemonic.c
strings.c
sqlite3.c
)

add_executable(pager ${loki_pager_app_SRCS})
target_link_libraries(pager ${LIBS})

# built only for user to debug corrupt database file
if(BUILD_SQL_DEBUG_SHELL)
add_executable(sqlite_shell sqlite3.c shell.c)
endif()

# Add -fPIC flag
if(BUILD_SHARED_LIBS)
    set_property(TARGET pager PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

add_custom_command(TARGET pager POST_BUILD
  COMMAND curl https://curl.haxx.se/ca/cacert.pem > rootcerts.pem
  COMMENT "Downloading Netscape Navigator root certificate bundle"
)

if (MSVC AND NOT USE_SEPARATE_CURSES)
add_custom_command(TARGET pager POST_BUILD
  COMMAND EDITBIN -SUBSYSTEM:WINDOWS $<TARGET_FILE:pager>
  COMMENT "setting PE header flag"
)
endif()
