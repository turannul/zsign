cmake_minimum_required(VERSION 3.10)

project(zsign)
set(CMAKE_CXX_STANDARD 11)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
        COMMAND brew --prefix openssl
        RESULT_VARIABLE BREW_OPENSSL
        OUTPUT_VARIABLE BREW_OPENSSL_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_OPENSSL EQUAL 0 AND EXISTS "${BREW_OPENSSL_PREFIX}")
        message(STATUS "Found OpenSSL keg installed by Homebrew at ${BREW_OPENSSL_PREFIX}")
        set(OPENSSL_ROOT_DIR "${BREW_OPENSSL_PREFIX}")
        set(OPENSSL_INCLUDE_DIR "${BREW_OPENSSL_PREFIX}/include")
        set(OPENSSL_LIBRARIES "${BREW_OPENSSL_PREFIX}/lib/libssl.dylib;${BREW_OPENSSL_PREFIX}/lib/libcrypto.dylib")
    else()
        message("OpenSSL not found in Homebrew prefix")
    endif()
else()
    find_package(OpenSSL REQUIRED)
endif()

message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")
include_directories(${OPENSSL_INCLUDE_DIR})
list(APPEND LIB_LIST ${OPENSSL_LIBRARIES})

find_package(ZLIB REQUIRED)
message("zlib include dir: ${ZLIB_INCLUDE_DIR}")
message("zlib libraries: ${ZLIB_LIBRARIES}")
include_directories(${ZLIB_INCLUDE_DIR})
list(APPEND LIB_LIST ${ZLIB_LIBRARIES})

file(GLOB SRC src/zsign/*.cpp)

add_executable(zsign ${SRC})

target_link_libraries(zsign ${LIB_LIST})

add_test(NAME zsign_Returns_0_1
    COMMAND zsign -v)
add_test(NAME zsign_Returns_0_2
    COMMAND zsign -k test/test.p12 -m test/test.mobileprovision -p favour test/test.ipa -o successful_Test.ipa)