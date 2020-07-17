cmake_minimum_required(VERSION 3.5)
project(rma)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -pthread")


file(GLOB SRC
        common/*.cpp
        data_structures/*.cpp
        data_structures/rma/*.cpp
        data_structures/rma/baseline/*.cpp
        data_structures/rma/batch_processing/*.cpp
        data_structures/rma/one_by_one/*.cpp
        data_structures/rma/common/*.cpp
        data_structures/abtree/sequential/*.cpp
        data_structures/abtree/parallel/*.cpp
        data_structures/bwtree/*.cpp
        data_structures/pcsr/*.cpp
        experiments/*.cpp
        distributions/*.cpp
        third-party/zipf/*.cpp
        third-party/sqlite3/sqlite3.c
        third-party/catch/*.cpp
        third-party/openbwtree/*.cpp
        third-party/art-olc/*.cpp
        third-party/gperftools-2.7/*.cpp
        distributions/*.cpp
        )

include_directories(.)
include_directories(common)
include_directories(data_structures)
include_directories(data_structures/pcsr)
include_directories(data_structures/rma)
include_directories(data_structures/rma/common)
include_directories(data_structures/rma/baseline)
include_directories(data_structures/rma/batch_processing)
include_directories(data_structures/rma/one_by_one)
include_directories(data_structures/abtree/sequential)
include_directories(data_structures/abtree/parallel)
include_directories(data_structures/bwtree)
include_directories(experiments)
include_directories(distributions)
include_directories(third-party/sqlite3)
include_directories(third-party/zipf)
include_directories(third-party/catch)
include_directories(third-party/clara)
include_directories(third-party/art-olc)
include_directories(third-party/openbwtree)

link_libraries(event event_core dl atomic event_pthreads)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

add_executable(rma main.cpp ${SRC})
