cmake_minimum_required(VERSION 3.15)
project(RequestTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# External includes
include_directories(
    ${CMAKE_SOURCE_DIR}/external/json/include
    ${CMAKE_SOURCE_DIR}/external/openssl/include
)

# OpenSSL library path depending on build type
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(OPENSSL_LIB_DIR "${CMAKE_SOURCE_DIR}/external/openssl/lib/MDd")
else()
    set(OPENSSL_LIB_DIR "${CMAKE_SOURCE_DIR}/external/openssl/lib/MD")
endif()

link_directories(${OPENSSL_LIB_DIR})

# Source files
set(SOURCES
    src/main.cpp
    src/logger/logger.cpp
    src/server/server.cpp
    src/utils/certgen.cpp
    src/utils/utils.cpp
)

# Headers (optional for IDEs)
set(HEADERS
    src/logger/logger.h
    src/server/server.h
    src/utils/certgen.h
    src/utils/utils.h
)

add_executable(RequestTracker ${SOURCES} ${HEADERS})

# Link OpenSSL and Threads
target_link_libraries(RequestTracker
    libssl
    libcrypto
    Threads::Threads
)

add_custom_command(TARGET RequestTracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/external/openssl/bin/libcrypto-3-x64.dll
        ${CMAKE_SOURCE_DIR}/external/openssl/bin/libssl-3-x64.dll
        $<TARGET_FILE_DIR:RequestTracker>
)

add_custom_command(TARGET RequestTracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/static
        $<TARGET_FILE_DIR:RequestTracker>/static
)
