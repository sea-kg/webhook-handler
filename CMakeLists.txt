cmake_minimum_required(VERSION 3.0)

project(webhook-handler)

include(${CMAKE_CURRENT_SOURCE_DIR}/src.wsjcpp/CMakeLists.txt)

set(CMAKE_CXX_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH ${webhook-handler_SOURCE_DIR})

#### BEGIN_WSJCPP_APPEND
#### END_WSJCPP_APPEND

# Sources

# include header dirs
list (APPEND WSJCPP_INCLUDE_DIRS "src")
list (APPEND WSJCPP_INCLUDE_DIRS "src/scripts")
list (APPEND WSJCPP_INCLUDE_DIRS "src/utils")
list (APPEND WSJCPP_INCLUDE_DIRS "src/interfaces")
list (APPEND WSJCPP_INCLUDE_DIRS "src/filter-rules")
list (APPEND WSJCPP_INCLUDE_DIRS "src/web-server")

# filters 
list (APPEND WSJCPP_SOURCES "src/filter-rules/github_json_filter_rules.h")
list (APPEND WSJCPP_SOURCES "src/filter-rules/github_json_filter_rules.h")

# utils
list (APPEND WSJCPP_SOURCES "src/utils/help_parse_args.h")
list (APPEND WSJCPP_SOURCES "src/utils/help_parse_args.cpp")

# scripts
list (APPEND WSJCPP_SOURCES "src/scripts/deque_webhooks.h")
list (APPEND WSJCPP_SOURCES "src/scripts/deque_webhooks.cpp")
list (APPEND WSJCPP_SOURCES "src/scripts/do_run_command.h")
list (APPEND WSJCPP_SOURCES "src/scripts/do_run_command.cpp")
list (APPEND WSJCPP_SOURCES "src/scripts/scripts_thread.h")
list (APPEND WSJCPP_SOURCES "src/scripts/scripts_thread.cpp")

# web-server
list (APPEND WSJCPP_SOURCES "src/web-server/http_handler_webhooks.h")
list (APPEND WSJCPP_SOURCES "src/web-server/http_handler_webhooks.cpp")

# main
list (APPEND WSJCPP_SOURCES "src/webhook_handler_config.h")
list (APPEND WSJCPP_SOURCES "src/webhook_handler_config.cpp")
list (APPEND WSJCPP_SOURCES "src/main.cpp")

include_directories(${WSJCPP_INCLUDE_DIRS})

add_executable (webhook-handler ${WSJCPP_SOURCES})

target_link_libraries(webhook-handler -lpthread ${WSJCPP_LIBRARIES} )

install(
    TARGETS
        webhook-handler
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
)
