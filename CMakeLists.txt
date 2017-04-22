cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(RealmOfAesirBackend CXX)
#set(CMAKE_CXX_STANDARD 17) #not supported yet :(
#set(CMAKE CXX STANDARD REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# only support linux for now
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -Wno-long-long -Wfloat-equal -pedantic -std=c++17")

include_directories("${PROJECT_SOURCE_DIR}")

file(GLOB_RECURSE PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB EASYLOGGING_SOURCE ${CMAKE_SOURCE_DIR}/external/common/external/easyloggingpp/src/easylogging++.cc)
add_executable(RealmOfAesirBackend ${EASYLOGGING_SOURCE} ${PROJECT_SOURCES} )

include_directories("${CMAKE_SOURCE_DIR}/external/common/src")
include_directories("${CMAKE_SOURCE_DIR}/external/common/external/librdkafka/src-cpp")
include_directories("${CMAKE_SOURCE_DIR}/external/common/external/librdkafka/src")
include_directories("${CMAKE_SOURCE_DIR}/external/common/external/easyloggingpp/src")
include_directories("${CMAKE_SOURCE_DIR}/external/common/external/cereal/include")
include_directories("${CMAKE_SOURCE_DIR}/external/common/external/di/include")
include_directories("${CMAKE_SOURCE_DIR}/external/libpqxx/include")
include_directories("${CMAKE_SOURCE_DIR}/external/libsodium/src/libsodium/include")
include_directories("${CMAKE_SOURCE_DIR}/external/uWebSockets/src")
include_directories("${CMAKE_SOURCE_DIR}/external/json/src")

find_library(LIBRDKAFKA_LIBRARY rdkafka HINTS ${CMAKE_SOURCE_DIR}/external/common/external/librdkafka/src)
find_library(LIBRDKAFKAPP_LIBRARY rdkafka++ HINTS ${CMAKE_SOURCE_DIR}/external/common/external/librdkafka/src-cpp)
find_library(UWEBSOCKET_LIBRARY uWS HINTS ${CMAKE_SOURCE_DIR}/external/uWebSockets)
find_library(PQXX_LIBRARY pqxx HINTS ${CMAKE_SOURCE_DIR}/external/libpqxx/src/.libs)
find_library(SODIUM_LIBRARY sodium HINTS ${CMAKE_SOURCE_DIR}/external/libsodium/src/libsodium)
find_library(LIBROA_COMMON_LIBRARY RealmOfAesirCommon HINTS ${CMAKE_SOURCE_DIR}/external/common/bin)

target_link_libraries(RealmOfAesirBackend PUBLIC ${LIBROA_COMMON_LIBRARY})
target_link_libraries(RealmOfAesirBackend PUBLIC ${LIBRDKAFKAPP_LIBRARY})
target_link_libraries(RealmOfAesirBackend PUBLIC ${LIBRDKAFKA_LIBRARY})
target_link_libraries(RealmOfAesirBackend PUBLIC ${UWEBSOCKET_LIBRARY})
target_link_libraries(RealmOfAesirBackend PUBLIC ${PQXX_LIBRARY})
target_link_libraries(RealmOfAesirBackend PUBLIC ${SODIUM_LIBRARY})

find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(RealmOfAesirBackend PUBLIC ${PostgreSQL_LIBRARIES})

find_package( ZLIB REQUIRED )
include_directories( ${ZLIB_INCLUDE_DIRS} )
target_link_libraries(RealmOfAesirBackend PUBLIC ${ZLIB_LIBRARIES} )

find_package (Threads)
target_link_libraries (RealmOfAesirBackend PUBLIC ${CMAKE_THREAD_LIBS_INIT})

find_package(OpenSSL REQUIRED)
target_link_libraries (RealmOfAesirBackend PUBLIC ${OPENSSL_LIBRARIES})