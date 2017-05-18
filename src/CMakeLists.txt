cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(RealmOfAesirBackend CXX)

include_directories("${PROJECT_SOURCE_DIR}")

file(GLOB_RECURSE PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB EASYLOGGING_SOURCE ${CMAKE_SOURCE_DIR}/external/common_backend/external/common/external/easyloggingpp/src/easylogging++.cc)
add_executable(RealmOfAesirBackend ${EASYLOGGING_SOURCE} ${PROJECT_SOURCES} )

include_directories("${CMAKE_SOURCE_DIR}/external/common_backend/src")
include_directories("${CMAKE_SOURCE_DIR}/external/common_backend/external/common/src")
include_directories("${CMAKE_SOURCE_DIR}/external/common_backend/external/librdkafka/src-cpp")
include_directories("${CMAKE_SOURCE_DIR}/external/common_backend/external/librdkafka/src")
include_directories("${CMAKE_SOURCE_DIR}/external/common_backend/external/common/external/easyloggingpp/src")
include_directories("${CMAKE_SOURCE_DIR}/external/common_backend/external/common/external/cereal/include")
include_directories("${CMAKE_SOURCE_DIR}/external/common_backend/external/common/external/di/include")
include_directories("${CMAKE_SOURCE_DIR}/external/common_backend/external/libpqxx/include")
include_directories("${CMAKE_SOURCE_DIR}/external/libsodium/src/libsodium/include")
include_directories("${CMAKE_SOURCE_DIR}/external/common_backend/external/common/external/json/src")

find_library(LIBRDKAFKA_LIBRARY rdkafka HINTS ${CMAKE_SOURCE_DIR}/external/common_backend/external/librdkafka/src)
find_library(LIBRDKAFKAPP_LIBRARY rdkafka++ HINTS ${CMAKE_SOURCE_DIR}/external/common_backend/external/librdkafka/src-cpp)
find_library(PQXX_LIBRARY pqxx HINTS ${CMAKE_SOURCE_DIR}/external/common_backend/external/libpqxx/src/.libs)
find_library(SODIUM_LIBRARY sodium HINTS ${CMAKE_SOURCE_DIR}/external/libsodium/src/libsodium/.libs)
find_library(LIBROA_COMMON_LIBRARY RealmOfAesirCommon HINTS ${CMAKE_SOURCE_DIR}/external/common_backend/external/common/bin)
find_library(LIBROA_COMMON_BACKEND_LIBRARY RealmOfAesirCommonBackend HINTS ${CMAKE_SOURCE_DIR}/external/common_backend/bin)

if(NOT LIBRDKAFKA_LIBRARY)
    message(FATAL_ERROR "librdkafka not found")
endif()

if(NOT LIBRDKAFKAPP_LIBRARY)
    message(FATAL_ERROR "librdkafkapp not found")
endif()

if(NOT PQXX_LIBRARY)
    message(FATAL_ERROR "libpqxx not found")
endif()

if(NOT SODIUM_LIBRARY)
    message(FATAL_ERROR "libsodium not found")
endif()

if(NOT LIBROA_COMMON_LIBRARY)
    message(FATAL_ERROR "libroa not found")
endif()

if(NOT LIBROA_COMMON_BACKEND_LIBRARY)
    message(FATAL_ERROR "libroa_backend not found")
endif()

target_link_libraries(RealmOfAesirBackend PUBLIC ${LIBROA_COMMON_LIBRARY})
target_link_libraries(RealmOfAesirBackend PUBLIC ${LIBROA_COMMON_BACKEND_LIBRARY})
target_link_libraries(RealmOfAesirBackend PUBLIC ${LIBRDKAFKAPP_LIBRARY})
target_link_libraries(RealmOfAesirBackend PUBLIC ${LIBRDKAFKA_LIBRARY})
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