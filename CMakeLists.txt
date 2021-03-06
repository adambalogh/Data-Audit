cmake_minimum_required(VERSION 3.2)

project(Data-Audit)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET(CMAKE_CXX_FLAGS 
  "${CMAKE_CXX_FLAGS} -Wall -O3 -D _TURN_OFF_PLATFORM_STRING")

set(SERVER_SIDE true)
set(CLIENT_SIDE false)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DCOMPRESS_FILES")

find_package( ZLIB REQUIRED )

# Build client when we are on Mac
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CLIENT_SIDE true)
  set(SERVER_SIDE false)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 

enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/../)
include_directories(/usr/local/include)

link_directories(/usr/local/lib)

set(THIRD_PARTY_LIBRARIES cryptopp crypto pthread) 

set(LIBRARIES serverlib clientlib local_disklib azurelib cporlib)

set(TEST_LIBRARIES gtest gtest_main test_util ${LIBRARIES} ${THIRD_PARTY_LIBRARIES})

# Data Audit libraries

if(${CLIENT_SIDE})

add_library(clientlib
  util.cc
  client/prf.cc
  client/upload/block_tag_serializer.cc
  client/upload/block_tagger.cc
  client/upload/client.cc
  client/upload/file.cc
  client/upload/storage.cc
  client/verify/client.cc
  client/verify/file_tag_source.cc
)

add_library(azurelib
  providers/azure/proof_source.cc
  providers/azure/file_storage.cc
  providers/azure/file_list_source.cc
  providers/azure/file_tag_source.cc
)

add_library(dropboxlib
  providers/dropbox/dropbox_client.cc
  providers/dropbox/fetcher.cc
  providers/dropbox/file_list_source.cc
  providers/dropbox/file_tag_source.cc
  providers/dropbox/file_storage.cc
  providers/dropbox/token_source.cc
  providers/dropbox/token_source_instance.cc
)

add_library(file_browserlib files/file_browser.cc)

endif(${CLIENT_SIDE})

add_library(serverlib
  util.cc
  server/prover.cc
)

add_library(local_disklib
  providers/local_disk/fetcher.cc
  providers/local_disk/file_storage.cc
  providers/local_disk/file_tag_source.cc
  providers/local_disk/file_list_source.cc
)
target_link_libraries(local_disklib lz4)

add_library(cporlib proto/cpor.pb.cc)
target_link_libraries(cporlib protobuf)

add_library(test_util test_util.cc)

# Server binary, only build if we are on Unix
if (${SERVER_SIDE})

add_executable(server
  server/main.cc
  server/proof_handler.cc
  server/storage_handler.cc
  server/file_tag_handler.cc
  server/file_list_handler.cc
  server/batch_proof_handler.cc
) 

target_link_libraries(server serverlib cporlib local_disklib ${THIRD_PARTY_LIBRARIES} proxygenlib
  proxygenhttpserver folly wangle glog gflags)

endif (${SERVER_SIDE})

# Benchmark and Simple test

if(${CLIENT_SIDE})

add_executable(install_main install.cc)
target_link_libraries(install_main cryptopp)

add_executable(test_main test/main.cc)
target_link_libraries(test_main ${THIRD_PARTY_LIBRARIES} ${LIBRARIES})

add_executable(benchmark_main benchmark/main.cc)
target_link_libraries(benchmark_main benchmark ${THIRD_PARTY_LIBRARIES}
    boost_system-mt boost_thread-mt boost_chrono-mt boost_date_time-mt
    cpprest.dylib ssl ${LIBRARIES} )


endif(${CLIENT_SIDE})

# Tests

macro (create_test target target_file)
  add_executable(${target} ${target_file})
  target_link_libraries(${target} ${TEST_LIBRARIES})
  add_test (${target} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target})
endmacro (create_test)

if(${CLIENT_SIDE})

create_test(util_test util_test.cc)
create_test(block_tag_map_test server/block_tag_map_test.cc)
create_test(prf_test client/prf_test.cc)
create_test(progress_bar_test client/upload/progress_bar_test.cc)
create_test(block_tag_serializer_test client/upload/block_tag_serializer_test.cc)
create_test(block_tagger_test client/upload/block_tagger_test.cc)
create_test(file_test client/upload/file_test.cc)
create_test(verify_file_test client/verify/verify_file_test.cc)
create_test(prover_test server/prover_test.cc)
create_test(local_file_storage providers/local_disk/file_storage_test.cc)

endif(${CLIENT_SIDE})
