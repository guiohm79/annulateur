cmake_minimum_required(VERSION 3.20)
project(asio_backend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration spécifique ASIO
set(ASIO_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../asiosdk_2.3.3_2019-06-14/common")
set(ASIO_LIBRARIES "")

# Configuration Node.js
execute_process(COMMAND node -p "require('node-addon-api').include"
              OUTPUT_VARIABLE NODE_ADDON_API_DIR
              OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories(
    ${ASIO_INCLUDE_DIRS}
    ${NODE_ADDON_API_DIR}
)

add_library(asio_backend SHARED
    asio_processor.cpp
)

target_link_libraries(asio_backend
    ${ASIO_LIBRARIES}
    winmm.lib
    ole32.lib
)
