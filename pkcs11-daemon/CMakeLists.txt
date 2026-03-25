cmake_minimum_required(VERSION 3.10)
project(pkcs11_daemon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(SQLite3 REQUIRED)
pkg_check_modules(LIBUDEV REQUIRED libudev)
pkg_check_modules(P11KIT REQUIRED p11-kit-1)

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${LIBUDEV_INCLUDE_DIRS}
    ${P11KIT_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    .
)

add_executable(pkcs11d
    daemon.cpp
    crypto.cpp
    vault.cpp
    sd_import.cpp
)

add_library(pkcs11lib SHARED
    pkcs11.cpp
)

target_link_libraries(pkcs11d
    ${OPENSSL_LIBRARIES}
    ${LIBUDEV_LIBRARIES}
    SQLite::SQLite3
)

target_link_libraries(pkcs11lib
    ${OPENSSL_LIBRARIES}
    ${P11KIT_LIBRARIES}
    SQLite::SQLite3
)

target_compile_options(pkcs11d PRIVATE ${LIBUDEV_CFLAGS_OTHER})
target_include_directories(pkcs11d PRIVATE ${LIBUDEV_INCLUDE_DIRS})

target_compile_options(pkcs11lib PRIVATE ${P11KIT_CFLAGS_OTHER})
target_include_directories(pkcs11lib PRIVATE
    ${P11KIT_INCLUDE_DIRS}
    .
)
