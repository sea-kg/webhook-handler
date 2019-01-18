cmake_minimum_required(VERSION 3.0)

project(mecd)

add_definitions(-DMECD_VERSION="0.2.0")
add_definitions(-DMECD_APP_NAME="mecd")

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH ${mecd_SOURCE_DIR})
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set (MECD_LIBRARIES "")
set (MECD_INCLUDE_DIRS "")
set (MECD_SOURCES "")

# Sources

# include header dirs
list (APPEND MECD_INCLUDE_DIRS "src")
list (APPEND MECD_INCLUDE_DIRS "src/scripts")
list (APPEND MECD_INCLUDE_DIRS "src/config")
list (APPEND MECD_INCLUDE_DIRS "src/utils")
list (APPEND MECD_INCLUDE_DIRS "src/interfaces")
list (APPEND MECD_INCLUDE_DIRS "src/web-server")
list (APPEND MECD_INCLUDE_DIRS "src/web-server/core")
list (APPEND MECD_INCLUDE_DIRS "src/3rd_party/nlohmann")
list (APPEND MECD_INCLUDE_DIRS "src/3rd_party/HowardHinnant/date")

# nlohmann::json
list (APPEND MECD_SOURCES "src/3rd_party/nlohmann/json.hpp")

# HowardHinnant/date
list (APPEND MECD_SOURCES "src/3rd_party/HowardHinnant/date/date.h")

# utils
list (APPEND MECD_SOURCES "src/utils/colormod.h")
list (APPEND MECD_SOURCES "src/utils/fs.h")
list (APPEND MECD_SOURCES "src/utils/fs.cpp")
list (APPEND MECD_SOURCES "src/utils/ts.h")
list (APPEND MECD_SOURCES "src/utils/ts.cpp")
list (APPEND MECD_SOURCES "src/utils/str.h")
list (APPEND MECD_SOURCES "src/utils/str.cpp")
list (APPEND MECD_SOURCES "src/utils/help_parse_args.h")
list (APPEND MECD_SOURCES "src/utils/help_parse_args.cpp")
list (APPEND MECD_SOURCES "src/utils/logger.h")
list (APPEND MECD_SOURCES "src/utils/logger.cpp")
list (APPEND MECD_SOURCES "src/utils/conf_file_parser.h")
list (APPEND MECD_SOURCES "src/utils/conf_file_parser.cpp")

# scripts
list (APPEND MECD_SOURCES "src/scripts/dorunscript.h")
list (APPEND MECD_SOURCES "src/scripts/dorunscript.cpp")
list (APPEND MECD_SOURCES "src/scripts/scripts_thread.h")
list (APPEND MECD_SOURCES "src/scripts/scripts_thread.cpp")
list (APPEND MECD_SOURCES "src/scripts/deque_webhooks.h")
list (APPEND MECD_SOURCES "src/scripts/deque_webhooks.cpp")

# config
list (APPEND MECD_SOURCES "src/config/config.h")
list (APPEND MECD_SOURCES "src/config/config.cpp")
list (APPEND MECD_SOURCES "src/config/webhook.h")
list (APPEND MECD_SOURCES "src/config/webhook.cpp")


# web-server
list (APPEND MECD_SOURCES "src/web-server/core/light_http_deque_requests.h")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_deque_requests.cpp")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_handler_base.h")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_handler_base.cpp")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_handlers.h")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_handlers.cpp")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_request.h")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_request.cpp")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_response.h")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_response.cpp")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_server.h")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_server.cpp")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_thread_worker.h")
list (APPEND MECD_SOURCES "src/web-server/core/light_http_thread_worker.cpp")
list (APPEND MECD_SOURCES "src/web-server/http_handler_webhooks.h")
list (APPEND MECD_SOURCES "src/web-server/http_handler_webhooks.cpp")

# main
list (APPEND MECD_SOURCES "src/main.cpp")

# CURL
FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  list (APPEND MECD_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
  list (APPEND MECD_LIBRARIES ${CURL_LIBRARIES})
ELSE(CURL_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the CURL library and development files.")
ENDIF(CURL_FOUND)

include_directories(${MECD_INCLUDE_DIRS})

add_executable (mecd ${MECD_SOURCES})

target_link_libraries(mecd -lpthread ${MECD_LIBRARIES} )

install(
    TARGETS
        mecd
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
)
