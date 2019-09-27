cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")
project(signal-protocol-c)

if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()

SET(SIGNAL_PROTOCOL_C_VERSION_MAJOR 2)
SET(SIGNAL_PROTOCOL_C_VERSION_MINOR 3)
SET(SIGNAL_PROTOCOL_C_VERSION_PATCH 2)
SET(SIGNAL_PROTOCOL_C_VERSION ${SIGNAL_PROTOCOL_C_VERSION_MAJOR}.${SIGNAL_PROTOCOL_C_VERSION_MINOR}.${SIGNAL_PROTOCOL_C_VERSION_PATCH})

SET(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
SET(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE STRING "The directory the binaries are installed in")
SET(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "The directory the libraries are installed in")
SET(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "The directory the headers are installed in")
SET(INSTALL_PKGCONFIG_DIR "${LIB_INSTALL_DIR}/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

if(WIN32)
OPTION(USE_SEPARATE_CURSES "Do not use a pre-built CDK+curses." OFF)
endif()

# libm is fully integrated into libc on windows
if (WIN32)
SET(M_LIB "")
endif()

INCLUDE(CheckSymbolExists)
INCLUDE(CheckCCompilerFlag)
INCLUDE(TestBigEndian)

if(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  set(SOLARIS ON)
  set(CMAKE_C_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lsocket -lnsl")
  add_definitions(-D_POSIX_PTHREAD_SEMANTICS)
endif()

CHECK_SYMBOL_EXISTS(memset_s "string.h" HAVE_MEMSET_S)

IF(WIN32)
  CHECK_SYMBOL_EXISTS(SecureZeroMemory "windows.h;winbase.h" HAVE_SECUREZEROMEMORY)
ENDIF()

IF(BUILD_TESTING)
  IF (MSVC)
    MESSAGE(WARNING "Check-based testing not available with MSVC, unit tests make extensive use of C99. Enabling only a limited set of unit tests.")
  ENDIF()
  enable_testing()
ENDIF(BUILD_TESTING)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  if(MSVC_VERSION LESS 1900)
    add_definitions(-Dsnprintf=_snprintf)
  endif()
  include_directories(include/msvc)
endif()

# On Windows targets, we use these to find curses, check, and cdk
# On all others, we only need CDK
include_directories(include)
include_directories(include/cdk)
link_directories(lib)

IF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0 -Wall -Wmissing-field-initializers -Wno-missing-braces -Wparentheses -Wno-pointer-sign -Wno-implicit-int")
ENDIF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")

IF(CMAKE_COMPILER_IS_GNUCC)
	CHECK_C_COMPILER_FLAG("-Wsign-conversion" GCC_WARN_SIGN_CONVERSION)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-compare")
	IF(GCC_WARN_SIGN_CONVERSION)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-conversion")
	ENDIF(GCC_WARN_SIGN_CONVERSION)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(CMAKE_C_COMPILER_ID MATCHES "Clang")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch -Wunused-variable -Wunused-value -Wshadow -Wint-conversion -Wpointer-sign -Wprotocol -Wshorten-64-to-32")
ENDIF(CMAKE_C_COMPILER_ID MATCHES "Clang")

IF(HAVE_MEMSET_S)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_MEMSET_S=1")
ENDIF(HAVE_MEMSET_S)

TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
IF(WORDS_BIGENDIAN)
	ADD_DEFINITIONS(-DWORDS_BIGENDIAN)
ENDIF(WORDS_BIGENDIAN)

IF(COVERAGE)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
	SET(LINK_FLAGS "${LINK_FLAGS} -fprofile-arcs -ftest-coverage")

	add_custom_command(OUTPUT run_coverage
			COMMAND ctest
			COMMAND lcov -q --capture --directory src --output-file coverage.info.total
			COMMAND lcov -q --remove coverage.info.total 'vpool.*' 'ut*.h' '*.pb-c.*' 'protobuf-c/*' 'curve25519/*' --output-file coverage.info
			COMMAND genhtml -q coverage.info --output-directory coverage
			COMMENT Collecting and creating coverage information
	)
	add_custom_target( coverage DEPENDS run_coverage )
ENDIF(COVERAGE)

add_subdirectory(src)
add_subdirectory(crypto)
add_subdirectory(json)
add_subdirectory(http)
add_subdirectory(app)

IF(BUILD_TESTING)
	add_subdirectory(tests)
ENDIF(BUILD_TESTING)
