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
	add_definitions(-DMBEDTLS_SELF_TEST)
ENDIF(BUILD_TESTING)

set(mbedtls_SRCS
aes.c
aesni.c
arc4.c
aria.c
asn1parse.c
asn1write.c
base64.c
bignum.c
blowfish.c
camellia.c
ccm.c
certs.c
chacha20.c
chachapoly.c
cipher_wrap.c
cipher.c
cmac.c
ctr_drbg.c
debug.c
des.c
dhm.c
ecdh.c
ecdsa.c
ecjpake.c
ecp_curves.c
ecp.c
entropy_poll.c
entropy.c
error.c
gcm.c
havege.c
hkdf.c
hmac_drbg.c
md_wrap.c
md.c
md2.c
md4.c
md5.c
memory_buffer_alloc.c
net_sockets.c
nist_kw.c
oid.c
padlock.c
pem.c
pk_wrap.c
pk.c
pkcs11.c
pkcs12.c
pkcs5.c
pkparse.c
pkwrite.c
platform_util.c
platform.c
poly1305.c
ripemd160.c
rsa_internal.c
rsa.c
scrub_data.c
sha1.c
sha256.c
sha512.c
ssl_cache.c
ssl_ciphersuites.c
ssl_cli.c
ssl_cookie.c
ssl_srv.c
ssl_ticket.c
ssl_tls.c
threading.c
timing.c
version_features.c
version.c
x509_create.c
x509_crl.c
x509_crt.c
x509_csr.c
x509.c
x509write_crt.c
x509write_csr.c
xtea.c
)

add_library(mbedtls OBJECT ${mbedtls_SRCS})

# Add -fPIC flag
if(BUILD_SHARED_LIBS)
    set_property(TARGET mbedtls PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
