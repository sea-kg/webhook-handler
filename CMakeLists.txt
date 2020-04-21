cmake_minimum_required(VERSION 3.0)

project(webhook-handler)

include(${CMAKE_CURRENT_SOURCE_DIR}/src.wsjcpp/CMakeLists.txt)

set(CMAKE_CXX_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH ${webhook-handler_SOURCE_DIR})
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#### BEGIN_WSJCPP_APPEND
#### END_WSJCPP_APPEND

# Sources

# include header dirs
list (APPEND WSJCPP_INCLUDE_DIRS "src")
list (APPEND WSJCPP_INCLUDE_DIRS "src/scripts")
list (APPEND WSJCPP_INCLUDE_DIRS "src/utils")
list (APPEND WSJCPP_INCLUDE_DIRS "src/interfaces")
list (APPEND WSJCPP_INCLUDE_DIRS "src/web-server")
list (APPEND WSJCPP_INCLUDE_DIRS "src/web-server/core")
list (APPEND WSJCPP_INCLUDE_DIRS "src/3rd_party/HowardHinnant/date")

# HowardHinnant/date
list (APPEND WSJCPP_SOURCES "src/3rd_party/HowardHinnant/date/date.h")

# utils
list (APPEND WSJCPP_SOURCES "src/utils/help_parse_args.h")
list (APPEND WSJCPP_SOURCES "src/utils/help_parse_args.cpp")
list (APPEND WSJCPP_SOURCES "src/utils/conf_file_parser.h")
list (APPEND WSJCPP_SOURCES "src/utils/conf_file_parser.cpp")

# scripts
list (APPEND WSJCPP_SOURCES "src/scripts/dorunscript.h")
list (APPEND WSJCPP_SOURCES "src/scripts/dorunscript.cpp")
list (APPEND WSJCPP_SOURCES "src/scripts/scripts_thread.h")
list (APPEND WSJCPP_SOURCES "src/scripts/scripts_thread.cpp")
list (APPEND WSJCPP_SOURCES "src/scripts/deque_webhooks.h")
list (APPEND WSJCPP_SOURCES "src/scripts/deque_webhooks.cpp")

# web-server
list (APPEND WSJCPP_SOURCES "src/web-server/http_handler_webhooks.h")
list (APPEND WSJCPP_SOURCES "src/web-server/http_handler_webhooks.cpp")

# main
list (APPEND WSJCPP_SOURCES "src/webhook_handler_config.h")
list (APPEND WSJCPP_SOURCES "src/webhook_handler_config.cpp")
list (APPEND WSJCPP_SOURCES "src/main.cpp")

# CURL
FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  list (APPEND WSJCPP_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
  list (APPEND WSJCPP_LIBRARIES ${CURL_LIBRARIES})
ELSE(CURL_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the CURL library and development files.")
ENDIF(CURL_FOUND)

include_directories(${WSJCPP_INCLUDE_DIRS})

add_executable (webhook-handler ${WSJCPP_SOURCES})

target_link_libraries(webhook-handler -lpthread ${WSJCPP_LIBRARIES} )

install(
    TARGETS
        webhook-handler
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
)
